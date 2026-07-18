#include "sendafrica/resources.hpp"
#include "sendafrica/exceptions.hpp"

#include <chrono>
#include <thread>

namespace sendafrica {

SMSResult SMSResource::send(const std::string& to, const std::string& message,
                             const std::optional<std::string>& sender) {
    util::require_nonempty(message, "message");
    std::string normalized_to = util::normalize_phone(to);

    nlohmann::json payload = {{"to", normalized_to}, {"message", message}};
    if (sender) {
        std::string s = util::validate_sender_id(*sender);
        if (!s.empty()) payload["from"] = s;
    }

    auto data = transport_.request("POST", "/sms", payload);
    return SMSResult::from_json(data);
}

BulkSMSResult SMSResource::send_many(const std::vector<BulkItem>& messages,
                                      const std::optional<std::string>& sender, double rate_limit_per_sec) {
    BulkSMSResult result;
    double delay = rate_limit_per_sec > 0 ? 1.0 / rate_limit_per_sec : 0.0;

    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& item = messages[i];
        try {
            auto sent = send(item.to, item.message, item.sender ? item.sender : sender);
            result.results.push_back(sent);
        } catch (const Error& e) {
            result.failed.push_back({static_cast<int>(i), item.to, e.message()});
        }

        if (delay > 0 && i + 1 < messages.size()) {
            std::this_thread::sleep_for(std::chrono::duration<double>(delay));
        }
    }

    return result;
}

util::SMSAnalysis SMSResource::analyze(const std::string& message) const {
    return util::analyze_sms(message);
}

}  // namespace sendafrica
