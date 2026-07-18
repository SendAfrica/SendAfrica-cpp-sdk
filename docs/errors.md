# Error Handling

The SendAfrica C++ SDK uses a structured exception hierarchy. All errors
derive from `sendafrica::Error`, so you can catch everything with one handler
or pick specific subtypes for fine-grained control.

## Exception hierarchy

```
std::runtime_error
└── sendafrica::Error
    ├── AuthenticationError      (HTTP 401)
    ├── ValidationError         (HTTP 400, 422)
    │   └── InvalidPhoneError   (phone normalization failure)
    ├── InsufficientCreditsError (HTTP 402)
    ├── RateLimitError          (HTTP 429)
    ├── NotFoundError           (HTTP 404)
    ├── ServerError             (HTTP 5xx)
    ├── ConnectionError         (network/timeout)
    └── WebhookSignatureError   (HMAC mismatch)
```

## Catching errors

```cpp
#include <sendafrica/sendafrica.hpp>

try {
    auto result = client.sms().send("0712345678", "Hello");
} catch (const sendafrica::AuthenticationError& e) {
    // Bad or missing API key
    std::cerr << "Auth failed: " << e.message() << "\n";
} catch (const sendafrica::RateLimitError& e) {
    // Too many requests — retry after wait
    if (e.retry_after()) {
        std::cerr << "Retry after " << *e.retry_after() << "s\n";
    }
} catch (const sendafrica::Error& e) {
    // Any other SDK error
    std::cerr << e.message() << "\n";
    if (e.status_code()) {
        std::cerr << "HTTP " << *e.status_code() << "\n";
    }
}
```

## Error properties

Every `sendafrica::Error` provides:

| Method | Type | Description |
|---|---|---|
| `message()` | `const string&` | Human-readable error message from the API |
| `status_code()` | `optional<int>` | HTTP status code (if available) |
| `request_id()` | `optional<string>` | Unique request ID from `X-Request-Id` header |
| `response_body()` | `optional<string>` | Raw JSON response body |

`RateLimitError` also provides:

| Method | Type | Description |
|---|---|---|
| `retry_after()` | `optional<double>` | Seconds to wait before retrying (from `Retry-After` header) |

## Automatic retries

The SDK automatically retries on:

| Status | Behavior |
|---|---|
| 429 (rate limit) | Retries after `Retry-After` header or exponential backoff |
| 500, 502, 503, 504 | Retries with exponential backoff |
| Connection errors | Retries with exponential backoff |

Backoff formula: `min(0.5 * 2^(attempt-1), 8.0)` seconds.

Maximum retries: configurable via `ClientOptions::max_retries` (default 3).

## HTTP status to exception mapping

| HTTP Status | Exception |
|---|---|
| 400 | `ValidationError` |
| 401 | `AuthenticationError` |
| 402 | `InsufficientCreditsError` |
| 404 | `NotFoundError` |
| 422 | `ValidationError` |
| 429 | `RateLimitError` |
| 5xx | `ServerError` |
| Other | `Error` (base class) |

## Common error scenarios

### Invalid phone number

```cpp
try {
    client.sms().send("not-a-phone", "Hello");
} catch (const sendafrica::InvalidPhoneError& e) {
    std::cerr << "Bad phone: " << e.message() << "\n";
}
```

### No API key

```cpp
// unsetenv("SENDAFRICA_API_KEY");
try {
    sendafrica::Client client;  // no key provided
} catch (const sendafrica::AuthenticationError& e) {
    std::cerr << "No API key: " << e.message() << "\n";
}
```

### Insufficient credits

```cpp
try {
    client.sms().send("0712345678", "Hello");
} catch (const sendafrica::InsufficientCreditsError& e) {
    std::cerr << "Buy more credits: " << e.message() << "\n";
}
```

### Rate limiting with retry

```cpp
try {
    client.sms().send("0712345678", "Hello");
} catch (const sendafrica::RateLimitError& e) {
    if (e.retry_after()) {
        std::this_thread::sleep_for(
            std::chrono::duration<double>(*e.retry_after())
        );
        // retry...
    }
}
```

Note: The SDK already handles retries internally. You only need this
if you want custom retry logic.
