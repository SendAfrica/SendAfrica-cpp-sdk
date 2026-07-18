#pragma once

#include "sendafrica/exceptions.hpp"
#include "sendafrica/http_transport.hpp"
#include "sendafrica/models.hpp"
#include "sendafrica/util.hpp"

#include <optional>
#include <string>
#include <vector>

namespace sendafrica {

class SMSResource {
public:
    explicit SMSResource(HttpTransport& transport) : transport_(transport) {}

    SMSResult send(const std::string& to, const std::string& message,
                    const std::optional<std::string>& sender = std::nullopt);

    // Sends each message in order, pacing at rate_limit_per_sec. Failures
    // are captured per-message rather than aborting the whole batch.
    struct BulkItem {
        std::string to;
        std::string message;
        std::optional<std::string> sender;
    };
    BulkSMSResult send_many(const std::vector<BulkItem>& messages,
                             const std::optional<std::string>& sender = std::nullopt,
                             double rate_limit_per_sec = 10.0);

    // Local-only, no network call.
    util::SMSAnalysis analyze(const std::string& message) const;

private:
    HttpTransport& transport_;
};

class CreditsResource {
public:
    explicit CreditsResource(HttpTransport& transport) : transport_(transport) {}

    CreditBalance balance();
    std::vector<CreditTransaction> history(int page = 1, int per_page = 25);

private:
    HttpTransport& transport_;
};

class PaymentsResource {
public:
    explicit PaymentsResource(HttpTransport& transport) : transport_(transport) {}

    Payment create(int amount, const std::string& provider = "manual",
                    const std::optional<std::string>& phone = std::nullopt);
    VoucherRate rate();

private:
    HttpTransport& transport_;
};

class WebhooksResource {
public:
    explicit WebhooksResource(HttpTransport& transport, std::optional<std::string> webhook_secret = std::nullopt)
        : transport_(transport), webhook_secret_(std::move(webhook_secret)) {}

    // Verifies (if a signature + secret are available) then parses the
    // payload. Throws WebhookSignatureError on mismatch.
    WebhookEvent parse(const std::string& payload, const std::optional<std::string>& signature = std::nullopt,
                        const std::optional<std::string>& secret = std::nullopt);

private:
    HttpTransport& transport_;
    std::optional<std::string> webhook_secret_;

    void verify_signature(const std::string& payload, const std::string& signature, const std::string& secret);
};

}  // namespace sendafrica
