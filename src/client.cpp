#include "sendafrica/client.hpp"
#include "sendafrica/auth.hpp"

namespace sendafrica {

Client::Client(std::optional<std::string> api_key, ClientOptions options)
    : environment_(options.environment),
      transport_(util::resolve_api_key(api_key), options.base_url, options.timeout_seconds,
                 options.max_retries, options.debug),
      sms_(transport_),
      credits_(transport_),
      payments_(transport_),
      webhooks_(transport_, options.webhook_secret) {}

}  // namespace sendafrica
