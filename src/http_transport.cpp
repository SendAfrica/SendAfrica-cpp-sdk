#include "sendafrica/http_transport.hpp"
#include "sendafrica/exceptions.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

namespace sendafrica {

namespace {

constexpr std::array<long, 5> kRetryableStatus = {429, 500, 502, 503, 504};

bool is_retryable(long status) {
    return std::find(kRetryableStatus.begin(), kRetryableStatus.end(), status) != kRetryableStatus.end();
}

std::string generate_request_id() {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(rng);
    return oss.str();
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* out = static_cast<std::string*>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* headers = static_cast<std::map<std::string, std::string>*>(userdata);
    std::string line(buffer, size * nitems);
    auto colon = line.find(':');
    if (colon != std::string::npos) {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        // trim
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            auto last = s.find_last_not_of(" \t\r\n");
            if (last != std::string::npos) s.erase(last + 1);
        };
        trim(key);
        trim(value);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        (*headers)[key] = value;
    }
    return size * nitems;
}

double backoff_seconds(int attempt) {
    return std::min(0.5 * std::pow(2, attempt - 1), 8.0);
}

}  // namespace

HttpTransport::HttpTransport(std::string api_key, std::string base_url, double timeout_seconds,
                              int max_retries, bool debug)
    : api_key_(std::move(api_key)),
      base_url_(std::move(base_url)),
      timeout_seconds_(timeout_seconds),
      max_retries_(max_retries),
      debug_(debug) {
    if (!base_url_.empty() && base_url_.back() == '/') base_url_.pop_back();
}

void HttpTransport::log(const std::string& msg) const {
    if (debug_) std::cerr << "[sendafrica] " << msg << std::endl;
}

HttpTransport::RawResponse HttpTransport::perform_once(const std::string& method, const std::string& url,
                                                          const std::optional<std::string>& body,
                                                          const std::string& request_id) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw ConnectionError("Failed to initialize libcurl");
    }

    RawResponse resp;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: sendafrica-cpp/1.0");
    headers = curl_slist_append(headers, ("X-Request-Id: " + request_id).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_seconds_ * 1000));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp.headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

    if (body) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body->c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body->size()));
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string err = curl_easy_strerror(res);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ConnectionError("Connection to SendAfrica API failed: " + err);
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    resp.status_code = status;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return resp;
}

std::string extract_error_message(const nlohmann::json& payload, long status_code) {
    // Match the Python SDK's _error_message: extract from {success, error: {code, message, details}} envelope.
    if (payload.contains("error") && payload["error"].is_object() && payload["error"].contains("message")) {
        return payload["error"]["message"].get<std::string>();
    }
    if (payload.contains("message") && payload["message"].is_string()) {
        return payload["message"].get<std::string>();
    }
    return "HTTP " + std::to_string(status_code);
}

nlohmann::json HttpTransport::handle_response(const RawResponse& resp) {
    nlohmann::json payload = nlohmann::json::object();
    if (!resp.body.empty()) {
        try {
            payload = nlohmann::json::parse(resp.body);
        } catch (const nlohmann::json::parse_error&) {
            payload = {{"message", resp.body}};
        }
    }

    if (resp.status_code >= 200 && resp.status_code < 300) {
        // Unwrap the {success, data, error, meta, request_id, timestamp} envelope,
        // matching the Python SDK's _handle_response.
        if (payload.is_object() && payload.contains("data")) {
            return payload["data"];
        }
        return payload;
    }

    std::string message = extract_error_message(payload, resp.status_code);
    std::optional<std::string> request_id;
    auto it = resp.headers.find("x-request-id");
    if (it != resp.headers.end()) request_id = it->second;
    std::string body_str = payload.dump();

    switch (resp.status_code) {
        case 400:
        case 422:
            throw ValidationError(message, resp.status_code, request_id, body_str);
        case 401:
            throw AuthenticationError(message, resp.status_code, request_id, body_str);
        case 402:
            throw InsufficientCreditsError(message, resp.status_code, request_id, body_str);
        case 404:
            throw NotFoundError(message, resp.status_code, request_id, body_str);
        case 429: {
            std::optional<double> retry_after;
            auto ra = resp.headers.find("retry-after");
            if (ra != resp.headers.end()) retry_after = std::stod(ra->second);
            throw RateLimitError(message, resp.status_code, request_id, body_str, retry_after);
        }
        default:
            if (resp.status_code >= 500) {
                throw ServerError(message, resp.status_code, request_id, body_str);
            }
            throw Error(message, resp.status_code, request_id, body_str);
    }
}

nlohmann::json HttpTransport::request(const std::string& method, const std::string& path,
                                       const std::optional<nlohmann::json>& json_body,
                                       const std::map<std::string, std::string>& query_params) {
    std::string url = base_url_ + path;
    if (!query_params.empty()) {
        std::ostringstream qs;
        bool first = true;
        for (const auto& [k, v] : query_params) {
            qs << (first ? '?' : '&') << k << '=' << v;
            first = false;
        }
        url += qs.str();
    }

    std::optional<std::string> body_str;
    if (json_body) body_str = json_body->dump();

    int attempt = 0;
    std::string last_error;

    while (attempt <= max_retries_) {
        attempt++;
        std::string request_id = generate_request_id();
        log(method + " " + path + " attempt=" + std::to_string(attempt) + " request_id=" + request_id);

        RawResponse resp;
        try {
            resp = perform_once(method, url, body_str, request_id);
        } catch (const ConnectionError& e) {
            last_error = e.message();
            if (attempt > max_retries_) throw;
            std::this_thread::sleep_for(std::chrono::duration<double>(backoff_seconds(attempt)));
            continue;
        }

        log("status=" + std::to_string(resp.status_code));

        if (is_retryable(resp.status_code) && attempt <= max_retries_) {
            double wait = backoff_seconds(attempt);
            if (resp.status_code == 429) {
                auto ra = resp.headers.find("retry-after");
                if (ra != resp.headers.end()) wait = std::stod(ra->second);
            }
            std::this_thread::sleep_for(std::chrono::duration<double>(wait));
            continue;
        }

        return handle_response(resp);
    }

    throw ConnectionError("Request failed after " + std::to_string(max_retries_) + " retries: " + last_error);
}

}  // namespace sendafrica
