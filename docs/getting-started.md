# Getting Started

This guide walks you through installing the SendAfrica C++ SDK and sending
your first SMS in under 5 minutes.

## Prerequisites

- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake >= 3.16
- libcurl development headers (`libcurl4-openssl-dev` on Debian/Ubuntu)
- OpenSSL development headers (`libssl-dev` on Debian/Ubuntu)
- A SendAfrica API key ([get one here](https://dashboard.sendafrica.online))

## Install dependencies (Debian/Ubuntu)

```bash
sudo apt-get install cmake g++ libcurl4-openssl-dev libssl-dev
```

## Create your project

```bash
mkdir my-sms-app && cd my-sms-app
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_sms_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(sendafrica
    GIT_REPOSITORY https://github.com/SendAfrica/SendAfrica-cpp-sdk.git
    GIT_TAG v1.0.1
)
FetchContent_MakeAvailable(sendafrica)

add_executable(my_sms_app main.cpp)
target_link_libraries(my_sms_app PRIVATE sendafrica::sendafrica)
```

### main.cpp

```cpp
#include <sendafrica/sendafrica.hpp>

#include <iostream>

int main() {
    // Option 1: pass API key directly
    sendafrica::Client client("SA-xxxxx");

    // Option 2: read from SENDAFRICA_API_KEY environment variable
    // sendafrica::Client client;

    // Send an SMS
    try {
        auto result = client.sms().send("0712345678", "Hello from C++ SDK!");
        std::cout << "Sent! message_id=" << result.message_id
                  << "  status=" << result.status
                  << "  credits=" << result.credits_used << "\n";
    } catch (const sendafrica::Error& e) {
        std::cerr << "Error: " << e.message() << "\n";
        return 1;
    }

    return 0;
}
```

## Build and run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./my_sms_app
```

## Using environment variables

For production, set your API key via environment variable instead of
hardcoding it:

```bash
export SENDAFRICA_API_KEY="SA-xxxxx"
./my_sms_app
```

Then in code:

```cpp
sendafrica::Client client;  // reads SENDAFRICA_API_KEY from env
```

## Next steps

- [API Reference](api-reference.md) — full method signatures and types
- [Error Handling](errors.md) — exception hierarchy and retry logic
- [Phone Normalization](phone-normalization.md) — E.164 number formats
- [SMS Analysis](sms-analysis.md) — encoding detection and credit estimation
- [Webhooks](webhooks.md) — incoming event verification
