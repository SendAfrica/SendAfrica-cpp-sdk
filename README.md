# SendAfrica C++ SDK

C++17 client for the SendAfrica SMS Infrastructure-as-a-Service API. Mirrors
the [Python SDK](../python)'s resource design (`sms`, `credits`, `payments`,
`webhooks`) with the same phone normalization and GSM-7/UCS-2 SMS analysis
logic, built on `libcurl` + `nlohmann/json` + `OpenSSL`.

## Quickstart

```cpp
#include <sendafrica/client.hpp>

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

## Installation

### 1. CMake FetchContent (recommended)

No package manager needed. CMake pulls the source and builds it alongside
your project.

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
# Add as a git submodule (or use vcpkg's builtin registry)
vcpkg install sendafrica
```

Then in your CMakeLists.txt:

```cmake
find_package(sendafrica CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

Or add to your `vcpkg.json`:

```json
{
    "dependencies": [
        {
            "name": "sendafrica",
            "version>=": "1.0.1"
        }
    ]
}
```

### 3. Conan

```bash
conan install sendafrica/1.0.0@ --build=missing
```

Or add to your `conanfile.txt`:

```ini
[requires]
sendafrica/1.0.1

[generators]
CMakeDeps
CMakeToolchain
```

Then:

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

Then in any project:

```cmake
find_package(sendafrica REQUIRED)
target_link_libraries(your_app PRIVATE sendafrica::sendafrica)
```

## Dependencies

| Dependency | Version | Notes |
|---|---|---|
| C++ compiler | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |
| CMake | >= 3.16 | |
| libcurl | >= 7.75 | HTTP transport |
| OpenSSL | >= 3.0 | HMAC-SHA256 for webhooks |
| nlohmann/json | >= 3.10 | JSON parsing (auto-fetched if missing) |

## Project layout

```
sendafrica-cpp/
├── CMakeLists.txt
├── vcpkg.json
├── conanfile.py
├── cmake/sendafricaConfig.cmake.in
├── include/sendafrica/
│   ├── client.hpp
│   ├── sendafrica.hpp           # umbrella header
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
