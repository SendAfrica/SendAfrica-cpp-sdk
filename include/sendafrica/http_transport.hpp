#pragma once
// Low-level HTTP transport: auth headers, retries/backoff, error mapping.

#include <nlohmann/json.hpp>
#include <map>
#include <optional>
#include <string>

namespace sendafrica {

constexpr const char* kDefaultBaseUrl = "https://api.sendafrica.online/v1";
constexpr double kDefaultTimeoutSeconds = 10.0;
constexpr int kDefaultMaxRetries = 3;

class HttpTransport {
public:
    HttpTransport(std::string api_key,
                  std::string base_url = kDefaultBaseUrl,
                  double timeout_seconds = kDefaultTimeoutSeconds,
                  int max_retries = kDefaultMaxRetries,
                  bool debug = false);

    // Performs an HTTP request, retrying on 429/5xx/connection errors up to
    // max_retries with exponential backoff (respects Retry-After on 429).
    // Returns the parsed JSON body on success; throws a sendafrica::Error
    // subclass on failure.
    nlohmann::json request(const std::string& method,
                            const std::string& path,
                            const std::optional<nlohmann::json>& json_body = std::nullopt,
                            const std::map<std::string, std::string>& query_params = {});

private:
    std::string api_key_;
    std::string base_url_;
    double timeout_seconds_;
    int max_retries_;
    bool debug_;

    struct RawResponse {
        long status_code = 0;
        std::string body;
        std::map<std::string, std::string> headers;
    };

    RawResponse perform_once(const std::string& method,
                              const std::string& url,
                              const std::optional<std::string>& body,
                              const std::string& request_id);

    nlohmann::json handle_response(const RawResponse& resp);
    void log(const std::string& msg) const;
};

}  // namespace sendafrica
