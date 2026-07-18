# SendAfrica C++ SDK

[![Version](https://img.shields.io/badge/version-1.0.1-blue.svg)](https://github.com/SendAfrica/SendAfrica-cpp-sdk)
[![C++ Standard](https://img.shields.io/badge/C++-17-green.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)](LICENSE)

Official C++17 client for the [SendAfrica](https://sendafrica.online) SMS Infrastructure-as-a-Service API.

Designed to feel like [Stripe's C++ libraries](https://github.com/stripe/stripe-cpp): simple for a first integration, enough control for production use. Mirrors the [Python SDK](https://github.com/SendAfrica/SendAfrica-python-sdk)'s resource design with the same phone normalization and GSM-7/UCS-2 SMS analysis logic.

## Quickstart

```cpp
#include <sendafrica/sendafrica.hpp>
#include <iostream>

int main() {
    sendafrica::Client client("sk_live_xxxxx");

    auto result = client.sms().send("0712345678", "Welcome to SendAfrica");
    std::cout << result.message_id << "\n";       // "SA-xxxx-xxxx-xxxx"
    std::cout << result.status << "\n";           // "Success"
    std::cout << result.credits_used << "\n";     // 1
}
```

## Resources

| Resource | Methods | Docs |
|---|---|---|
| `client.sms()` | `send`, `send_many`, `analyze` | [API Reference](docs/api-reference.md#smsresource) |
| `client.credits()` | `balance`, `history` | [API Reference](docs/api-reference.md#creditsresource) |
| `client.payments()` | `create`, `rate` | [API Reference](docs/api-reference.md#paymentsresource) |
| `client.webhooks()` | `parse` | [Webhooks](docs/webhooks.md) |

## Installation

### 1. CMake FetchContent (recommended)

```cmake
include(FetchContent)
FetchContent_Declare(sendafrica
    GIT_REPOSITORY https://github.com/SendAfrica/SendAfrica-cpp-sdk.git
    GIT_TAG v1.0.1
)
FetchContent_MakeAvailable(sendafrica)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

### 2. vcpkg

```bash
vcpkg install sendafrica
```

```cmake
find_package(sendafrica CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

### 3. Conan

```bash
conan install sendafrica/1.0.1@ --build=missing
```

```cmake
find_package(sendafrica REQUIRED)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

### 4. Build from source

```bash
git clone https://github.com/SendAfrica/SendAfrica-cpp-sdk.git
cd SendAfrica-cpp-sdk
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --install .
```

## Dependencies

| Dependency | Version | Notes |
|---|---|---|
| C++ compiler | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |
| CMake | >= 3.16 | |
| libcurl | >= 7.75 | HTTP transport |
| OpenSSL | >= 3.0 | HMAC-SHA256 for webhooks |
| nlohmann/json | >= 3.10 | JSON parsing (auto-fetched if missing) |

### Debian/Ubuntu

```bash
sudo apt-get install cmake g++ libcurl4-openssl-dev libssl-dev
```

## Configuration

```cpp
sendafrica::ClientOptions opts;
opts.timeout_seconds = 30;      // default 10
opts.max_retries = 5;           // default 3
opts.debug = true;              // log to stderr
opts.webhook_secret = "whsec_...";

sendafrica::Client client("sk_live_xxxxx", opts);
```

| Option | Type | Default | Description |
|---|---|---|---|
| `base_url` | `string` | `https://api.sendafrica.online/v1` | API endpoint |
| `timeout_seconds` | `double` | `10.0` | Request timeout |
| `max_retries` | `int` | `3` | Retries on 429/5xx |
| `environment` | `string` | `"production"` | Environment label |
| `debug` | `bool` | `false` | Log requests |
| `webhook_secret` | `optional<string>` | `nullopt` | Webhook HMAC secret |

## Error Handling

All errors derive from `sendafrica::Error`:

```cpp
try {
    client.sms().send("0712345678", "Hello");
} catch (const sendafrica::AuthenticationError& e) {
    std::cerr << "Bad API key: " << e.message() << "\n";
} catch (const sendafrica::RateLimitError& e) {
    std::cerr << "Rate limited, retry after " << *e.retry_after() << "s\n";
} catch (const sendafrica::Error& e) {
    std::cerr << e.message() << " (HTTP " << *e.status_code() << ")\n";
}
```

| Exception | HTTP Status | Meaning |
|---|---|---|
| `AuthenticationError` | 401 | Invalid or missing API key |
| `ValidationError` | 400, 422 | Bad request payload |
| `InvalidPhoneError` | — | Phone number failed validation |
| `InsufficientCreditsError` | 402 | Not enough SMS credits |
| `RateLimitError` | 429 | Too many requests |
| `NotFoundError` | 404 | Resource not found |
| `ServerError` | 5xx | API server error |
| `ConnectionError` | — | Network/timeout failure |
| `WebhookSignatureError` | — | Webhook signature mismatch |

See [Error Handling](docs/errors.md) for full details.

## Documentation

| Document | Description |
|---|---|
| [Getting Started](docs/getting-started.md) | First SMS in 5 minutes |
| [API Reference](docs/api-reference.md) | All types, methods, and parameters |
| [Error Handling](docs/errors.md) | Exception hierarchy and retry logic |
| [Phone Normalization](docs/phone-normalization.md) | E.164 number formats |
| [SMS Analysis](docs/sms-analysis.md) | GSM-7/UCS-2 encoding detection |
| [Webhooks](docs/webhooks.md) | Incoming event verification |

## Test App

A ready-to-run test app is available at [SendAfrica/SendAfrica-cpp-test](https://github.com/SendAfrica/SendAfrica-cpp-test).

```bash
git clone https://github.com/SendAfrica/SendAfrica-cpp-test.git
cd SendAfrica-cpp-test
cp .env.example .env    # add your API key
cmake -B build -S .
cmake --build build
./build/sendafrica_test
```

## Project Layout

```
sendafrica-cpp/
├── CMakeLists.txt                  # build + install/export
├── vcpkg.json                      # vcpkg manifest
├── conanfile.py                    # Conan recipe
├── cmake/sendafricaConfig.cmake.in # find_package config
├── include/sendafrica/
│   ├── sendafrica.hpp              # umbrella header
│   ├── client.hpp                  # Client, ClientOptions
│   ├── http_transport.hpp          # HTTP layer (libcurl)
│   ├── exceptions.hpp              # error hierarchy
│   ├── models.hpp                  # response structs
│   ├── resources.hpp               # SMS, Credits, Payments, Webhooks
│   ├── auth.hpp                    # API key resolution
│   └── util.hpp                    # phone normalization, SMS analysis
├── src/                            # implementation files
├── docs/                           # documentation
├── tests/test_local.cpp            # local unit tests
└── examples/basic_send.cpp         # usage example
```

## SDK Ecosystem

| Language | Package Manager | Repository |
|---|---|---|
| Python | PyPI (`pip install sendafrica`) | [SendAfrica-python-sdk](https://github.com/SendAfrica/SendAfrica-python-sdk) |
| C++ | GitHub + vcpkg + Conan | [SendAfrica-cpp-sdk](https://github.com/SendAfrica/SendAfrica-cpp-sdk) |
| TypeScript | npm | [SendAfrica-ts-sdk](https://github.com/SendAfrica/SendAfrica-ts-sdk) |
| Go | Go Modules | [SendAfrica-go-sdk](https://github.com/SendAfrica/SendAfrica-go-sdk) |

## Roadmap

- **Phase 1 (done):** client, auth, SMS send/send_many/analyze, credits, payments/vouchers, error tree, webhook verification
- **Phase 2:** async variant (libcurl multi-interface or coroutine wrapper), CLI, bulk-send polish
- **Phase 3:** campaigns, contacts, templates, scheduling

## License

MIT
