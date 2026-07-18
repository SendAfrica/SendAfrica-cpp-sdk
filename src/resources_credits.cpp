#include "sendafrica/resources.hpp"

namespace sendafrica {

CreditBalance CreditsResource::balance() {
    auto data = transport_.request("GET", "/credits/balance");
    return CreditBalance::from_json(data);
}

std::vector<CreditTransaction> CreditsResource::history(int page, int per_page) {
    std::map<std::string, std::string> params = {
        {"page", std::to_string(page)},
        {"per_page", std::to_string(per_page)}};

    auto data = transport_.request("GET", "/credits/history", std::nullopt, params);
    const auto& items = data.contains("items") ? data["items"] : data;

    std::vector<CreditTransaction> result;
    for (const auto& item : items) result.push_back(CreditTransaction::from_json(item));
    return result;
}

}  // namespace sendafrica
