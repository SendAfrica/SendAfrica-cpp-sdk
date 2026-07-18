# SendAfrica C++ SDK

C++17 client for the SendAfrica SMS Infrastructure-as-a-Service API. Mirrors
the [Python SDK](../python)'s resource design (`sms`, `credits`, `payments`,
`webhooks`) with the same phone normalization and GSM-7/UCS-2 SMS analysis
logic, built on `libcurl` + `nlohmann/json` + `OpenSSL`.

## Quickstart

```cpp
#include <sendafrica/sendafrica.hpp>

int main() {
    sendafrica::Client client("sk_live_xxxxx");
    auto result = client.sms().send("0712345678", "Welcome to SendAfrica");
    // result.message_id, result.status, result.credits_used
}
```

The API key can also come from the `SENDAFRICA_API_KEY` environment
variable — pass `std::nullopt` as the constructor's first argument.

```cpp
sendafrica::ClientOptions opts;
opts.timeout_seconds = 30;
opts.max_retries = 3;
opts.debug = true;
sendafrica::Client client(std::nullopt, opts);
```

## Resources

```cpp
client.sms().send(to, message, sender);
client.sms().send_many(items, sender, rate_limit_per_sec);
client.sms().analyze(message);          // local-only, no network call

client.credits().balance();
client.credits().history(page, per_page);

client.payments().create(amount, provider, phone);
client.payments().rate();               // fetch tiered pricing schedule

client.webhooks().parse(payload, signature, secret);  // HMAC-SHA256 verified
```

`payments.get()` and `payments.list()` don't exist: the API only exposes
`POST /v1/vouchers` (create) and `GET /v1/vouchers/rate` (pricing) to API
key callers — order lookup/listing is an admin-console (JWT) feature.

All errors derive from `sendafrica::Error` (`AuthenticationError`,
`ValidationError`, `InvalidPhoneError`, `InsufficientCreditsError`,
`RateLimitError`, `NotFoundError`, `ServerError`, `ConnectionError`,
`WebhookSignatureError`).

## Building it yourself

Dependencies: a C++17 compiler, CMake >= 3.16, `libcurl`, `OpenSSL`.
`nlohmann/json` is auto-fetched via CMake `FetchContent` if not already
installed on the system.

```bash
# Debian/Ubuntu
apt-get install cmake libcurl4-openssl-dev libssl-dev nlohmann-json3-dev

mkdir build && cd build
cmake -DSENDAFRICA_BUILD_TESTS=ON -DSENDAFRICA_BUILD_EXAMPLES=ON ..
make -j
./tests/sendafrica_tests    # all checks passed
```

## Installation for downstream projects

### Option A — FetchContent (recommended)

```cmake
include(FetchContent)
FetchContent_Declare(sendafrica
    GIT_REPOSITORY https://github.com/SendAfrica/SendAfrica-cpp-sdk.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(sendafrica)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

### Option B — cmake --install

```bash
cd build && cmake --install . --prefix /usr/local
```

Then in the consumer project:

```cmake
find_package(sendafrica REQUIRED)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

## Project layout

```
sendafrica-cpp/
├── CMakeLists.txt
├── cmake/sendafricaConfig.cmake.in
├── include/sendafrica/
│   ├── sendafrica.hpp           # umbrella header
│   ├── client.hpp
│   ├── http_transport.hpp
│   ├── exceptions.hpp
│   ├── models.hpp
│   ├── resources.hpp
│   ├── auth.hpp
│   └── util.hpp
├── src/
├── tests/test_local.cpp
└── examples/basic_send.cpp
```

## Roadmap (mirrors the Python SDK)

- **Phase 1 (done):** client, auth, SMS send/send_many/analyze, credits,
  payments/vouchers, error tree, webhook verification
- **Phase 2:** async variant (libcurl multi-interface or a coroutine
  wrapper), CLI, bulk-send polish
- **Phase 3:** campaigns, contacts, templates, scheduling
