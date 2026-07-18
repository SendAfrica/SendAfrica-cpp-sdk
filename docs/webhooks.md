# Webhook Verification

The SendAfrica SDK includes HMAC-SHA256 webhook signature verification to
ensure incoming payloads are authentic.

## How it works

1. SendAfrica signs each webhook payload with a shared secret using HMAC-SHA256
2. The signature is sent in the `X-SendAfrica-Signature` header
3. Your server receives the webhook and passes it to the SDK
4. The SDK verifies the signature before parsing the payload

## Setup

### Configure the webhook secret

Pass the secret when creating the client:

```cpp
sendafrica::ClientOptions opts;
opts.webhook_secret = "your_webhook_secret_here";
sendafrica::Client client(std::nullopt, opts);
```

Or pass it per-request to `parse()`.

### Parse a webhook

```cpp
#include <sendafrica/sendafrica.hpp>

// In your HTTP handler:
std::string raw_body = /* read from request */;
std::string signature = /* from X-SendAfrica-Signature header */;

try {
    auto event = client.webhooks().parse(raw_body, signature);

    if (event.type == "sms.delivered") {
        std::cout << "Message " << *event.message_id << " delivered\n";
    } else if (event.type == "sms.failed") {
        std::cout << "Message " << *event.message_id << " failed\n";
    }
} catch (const sendafrica::WebhookSignatureError& e) {
    // Signature mismatch — reject the request
    std::cerr << "Invalid webhook signature\n";
    // Return HTTP 400 to the caller
}
```

### Without client-level secret

You can verify per-request instead of configuring the client:

```cpp
auto event = client.webhooks().parse(
    raw_body,
    signature,
    "per_request_secret"
);
```

## WebhookEvent struct

```cpp
struct WebhookEvent {
    std::string type;                       // e.g. "sms.delivered"
    std::optional<std::string> message_id;  // related message ID
    nlohmann::json data;                    // full event payload
};
```

## Security notes

- Always verify signatures before processing webhooks
- Return HTTP 400 if verification fails (don't leak information)
- Store the webhook secret in an environment variable, not in source code
- The SDK uses constant-time comparison to prevent timing attacks

## Example: bare-bones HTTP server

```cpp
#include <sendafrica/sendafrica.hpp>
#include <iostream>
#include <string>

// Integrate with your HTTP library (cpp-httplib, Crow, etc.)
void handle_webhook(const std::string& body, const std::string& signature) {
    static sendafrica::Client client;  // reads webhook_secret from env

    try {
        auto event = client.webhooks().parse(body, signature);
        std::cout << "Event: " << event.type << "\n";

        // Process event...
    } catch (const sendafrica::WebhookSignatureError&) {
        std::cerr << "Rejected: bad signature\n";
        // Return HTTP 400
    }
}
```

## Current status

> **Note:** As of this writing, the SendAfrica API does not yet forward
> signed events to customer endpoints. This resource is provided ahead
> of that backend feature shipping. The signature verification uses
> HMAC-SHA256 over the raw body, matching the common pattern used by
> Stripe and most SMS aggregators.
