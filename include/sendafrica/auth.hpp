#pragma once
#include <optional>
#include <string>

namespace sendafrica::util {

// Resolves the API key: explicit arg > SENDAFRICA_API_KEY env var.
// Throws AuthenticationError if neither is set.
std::string resolve_api_key(const std::optional<std::string>& api_key);

}  // namespace sendafrica::util
