#include "sendafrica/resources.hpp"

namespace sendafrica {

Payment PaymentsResource::create(int amount, const std::string& provider,
                                  const std::optional<std::string>& phone) {
    util::require_nonempty(provider, "provider");
    util::validate_positive_amount(static_cast<double>(amount));

    nlohmann::json payload = {{"provider", provider}, {"amount", amount}};
    if (phone) {
        payload["phone"] = util::normalize_phone(*phone);
    }

    auto data = transport_.request("POST", "/vouchers", payload);
    return Payment::from_json(data);
}

VoucherRate PaymentsResource::rate() {
    auto data = transport_.request("GET", "/vouchers/rate");
    return VoucherRate::from_json(data);
}

}  // namespace sendafrica
