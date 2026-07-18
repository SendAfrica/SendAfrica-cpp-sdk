#include "sendafrica/resources.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <iomanip>
#include <sstream>

namespace sendafrica {

namespace {

std::string hmac_sha256_hex(const std::string& key, const std::string& data) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(), digest, &digest_len);

    std::ostringstream oss;
    for (unsigned int i = 0; i < digest_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return oss.str();
}

bool constant_time_equal(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

}  // namespace

void WebhooksResource::verify_signature(const std::string& payload, const std::string& signature,
                                         const std::string& secret) {
    std::string expected = hmac_sha256_hex(secret, payload);
    if (!constant_time_equal(expected, signature)) {
        throw WebhookSignatureError("Webhook signature verification failed");
    }
}

WebhookEvent WebhooksResource::parse(const std::string& payload, const std::optional<std::string>& signature,
                                      const std::optional<std::string>& secret) {
    std::optional<std::string> effective_secret = secret ? secret : webhook_secret_;
    if (signature && effective_secret) {
        verify_signature(payload, *signature, *effective_secret);
    }

    auto data = nlohmann::json::parse(payload);
    return WebhookEvent::from_json(data);
}

}  // namespace sendafrica
