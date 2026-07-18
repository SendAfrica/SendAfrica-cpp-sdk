#pragma once
// sendafrica::Client — the main entry point of the SDK.
//
//   sendafrica::Client client("sk_live_xxxxx");
//   auto result = client.sms().send("0712345678", "Welcome to SendAfrica");

#include "sendafrica/http_transport.hpp"
#include "sendafrica/resources.hpp"

#include <memory>
#include <optional>
#include <string>

namespace sendafrica {

struct ClientOptions {
    std::string base_url = kDefaultBaseUrl;
    double timeout_seconds = kDefaultTimeoutSeconds;
    int max_retries = kDefaultMaxRetries;
    std::string environment = "production";
    bool debug = false;
    std::optional<std::string> webhook_secret;
};

class Client {
public:
    // api_key may be std::nullopt to read from SENDAFRICA_API_KEY env var.
    explicit Client(std::optional<std::string> api_key = std::nullopt, ClientOptions options = {});

    SMSResource& sms() { return sms_; }
    CreditsResource& credits() { return credits_; }
    PaymentsResource& payments() { return payments_; }
    WebhooksResource& webhooks() { return webhooks_; }

    const std::string& environment() const { return environment_; }

private:
    std::string environment_;
    HttpTransport transport_;
    SMSResource sms_;
    CreditsResource credits_;
    PaymentsResource payments_;
    WebhooksResource webhooks_;
};

}  // namespace sendafrica
