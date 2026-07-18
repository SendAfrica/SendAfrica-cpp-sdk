cmake_minimum_required(VERSION 3.16)
project(sendafrica VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(SENDAFRICA_BUILD_TESTS "Build sendafrica SDK tests" OFF)
option(SENDAFRICA_BUILD_EXAMPLES "Build sendafrica SDK examples" OFF)
option(SENDAFRICA_BUILD_SHARED "Build shared library instead of static" OFF)

include(GNUInstallDirs)
include(FetchContent)

# --- Dependencies -----------------------------------------------------------

find_package(CURL REQUIRED)
if(TARGET CURL::libcurl)
    get_target_property(_curl_loc CURL::libcurl IMPORTED_LOCATION)
    if(NOT _curl_loc)
        set_target_properties(CURL::libcurl PROPERTIES
            IMPORTED_LOCATION "${CURL_LIBRARIES}"
        )
    endif()
endif()
find_package(OpenSSL REQUIRED)

# nlohmann/json: prefer an already-installed system package (apt/vcpkg/conan),
# fall back to fetching the header-only release if none is found. This keeps
# a plain `cmake --build .` working on a fresh machine with no extra setup.
find_package(nlohmann_json 3.10 QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# --- Library -----------------------------------------------------------------

set(SENDAFRICA_SOURCES
    src/auth.cpp
    src/client.cpp
    src/http_transport.cpp
    src/resources_sms.cpp
    src/resources_credits.cpp
    src/resources_payments.cpp
    src/resources_webhooks.cpp
    src/util.cpp
)

if(SENDAFRICA_BUILD_SHARED)
    add_library(sendafrica SHARED ${SENDAFRICA_SOURCES})
else()
    add_library(sendafrica STATIC ${SENDAFRICA_SOURCES})
endif()
add_library(sendafrica::sendafrica ALIAS sendafrica)

target_include_directories(sendafrica
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(sendafrica
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        CURL::libcurl
        OpenSSL::Crypto
)

target_compile_features(sendafrica PUBLIC cxx_std_17)

if(SENDAFRICA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SENDAFRICA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Install / export (what makes `find_package(sendafrica)` work for the client) --

install(TARGETS sendafrica
    EXPORT sendafricaTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT sendafricaTargets
    FILE sendafricaTargets.cmake
    NAMESPACE sendafrica::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sendafrica
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sendafricaConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sendafricaConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sendafrica
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sendafricaConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sendafricaConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/sendafricaConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sendafrica
)
