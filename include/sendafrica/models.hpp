#pragma once
// Response models — plain structs, built from the parsed JSON response.

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace sendafrica {

struct SMSResult {
    std::string message_id;
    std::string status;
    int credits_used = 0;
    std::optional<std::string> cost;
    std::optional<std::string> to;

    static SMSResult from_json(const nlohmann::json& j) {
        SMSResult r;
        r.message_id = j.value("message_id", "");
        r.status = j.value("status", "");
        r.credits_used = j.value("credits_used", 0);
        if (j.contains("cost") && !j["cost"].is_null()) r.cost = j["cost"].get<std::string>();
        if (j.contains("to") && !j["to"].is_null()) r.to = j["to"].get<std::string>();
        return r;
    }
};

struct BulkSMSFailure {
    int index = 0;
    std::string to;
    std::string error;
};

struct BulkSMSResult {
    std::vector<SMSResult> results;
    std::vector<BulkSMSFailure> failed;

    size_t sent_count() const { return results.size(); }
    size_t failed_count() const { return failed.size(); }
};

struct CreditBalance {
    std::string account_id;
    long long balance = 0;

    static CreditBalance from_json(const nlohmann::json& j) {
        CreditBalance b;
        b.account_id = j.value("account_id", "");
        b.balance = j.value("balance", 0LL);
        return b;
    }
};

struct CreditTransaction {
    std::string id;
    std::string type;
    long long amount = 0;
    long long balance_after = 0;
    std::optional<std::string> description;
    std::optional<std::string> created_at;

    static CreditTransaction from_json(const nlohmann::json& j) {
        CreditTransaction t;
        t.id = j.value("id", "");
        t.type = j.value("type", "");
        t.amount = j.value("amount", 0LL);
        t.balance_after = j.value("balance_after", 0LL);
        if (j.contains("description") && !j["description"].is_null())
            t.description = j["description"].get<std::string>();
        if (j.contains("created_at") && !j["created_at"].is_null())
            t.created_at = j["created_at"].get<std::string>();
        return t;
    }
};

struct Payment {
    std::string id;
    std::string status;
    std::optional<int> amount;
    std::optional<int> credit_amount;
    std::string currency = "TZS";
    std::optional<std::string> provider;
    std::optional<std::string> source;
    std::optional<std::string> created_at;

    static Payment from_json(const nlohmann::json& j) {
        Payment p;
        p.id = j.value("id", "");
        p.status = j.value("status", "");
        if (j.contains("amount") && !j["amount"].is_null()) p.amount = j["amount"].get<int>();
        if (j.contains("credit_amount") && !j["credit_amount"].is_null())
            p.credit_amount = j["credit_amount"].get<int>();
        p.currency = j.value("currency", "TZS");
        if (j.contains("provider") && !j["provider"].is_null())
            p.provider = j["provider"].get<std::string>();
        if (j.contains("source") && !j["source"].is_null())
            p.source = j["source"].get<std::string>();
        if (j.contains("created_at") && !j["created_at"].is_null())
            p.created_at = j["created_at"].get<std::string>();
        return p;
    }
};

struct RateTier {
    int max_amount_tzs = 0;
    int rate_tzs_per_credit = 0;

    static RateTier from_json(const nlohmann::json& j) {
        RateTier t;
        t.max_amount_tzs = j.value("max_amount_tzs", 0);
        t.rate_tzs_per_credit = j.value("rate_tzs_per_credit", 0);
        return t;
    }
};

struct VoucherRate {
    int min_amount_tzs = 0;
    std::vector<RateTier> tiers;

    static VoucherRate from_json(const nlohmann::json& j) {
        VoucherRate v;
        v.min_amount_tzs = j.value("min_amount_tzs", 0);
        if (j.contains("tiers") && j["tiers"].is_array()) {
            for (const auto& t : j["tiers"]) {
                v.tiers.push_back(RateTier::from_json(t));
            }
        }
        return v;
    }
};

struct MessageStatus {
    std::string message_id;
    std::string status;

    static MessageStatus from_json(const nlohmann::json& j) {
        MessageStatus s;
        s.message_id = j.value("message_id", "");
        s.status = j.value("status", "");
        return s;
    }
};

struct WebhookEvent {
    std::string type;
    std::optional<std::string> message_id;
    nlohmann::json data;

    static WebhookEvent from_json(const nlohmann::json& j) {
        WebhookEvent e;
        e.type = j.value("type", "");
        if (j.contains("message_id") && !j["message_id"].is_null())
            e.message_id = j["message_id"].get<std::string>();
        e.data = j;
        return e;
    }
};

}  // namespace sendafrica
