#include "sendafrica/auth.hpp"
#include "sendafrica/exceptions.hpp"

#include <cstdlib>

namespace sendafrica::util {

std::string resolve_api_key(const std::optional<std::string>& api_key) {
    if (api_key && !api_key->empty()) return *api_key;

    const char* env = std::getenv("SENDAFRICA_API_KEY");
    if (env && *env) return std::string(env);

    throw AuthenticationError(
        "No API key provided. Pass api_key or set the SENDAFRICA_API_KEY environment variable.");
}

}  // namespace sendafrica::util
