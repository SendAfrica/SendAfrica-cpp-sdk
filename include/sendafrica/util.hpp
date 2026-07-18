#pragma once
// Phone normalization, SMS encoding analysis, and small field validators.

#include <string>

namespace sendafrica::util {

// Normalizes to E.164 (e.g. "+255712345678"). Accepts 0712345678,
// 712345678, 255712345678, +255712345678 (with or without spaces).
// Throws InvalidPhoneError if it cannot be confidently normalized.
std::string normalize_phone(const std::string& raw, const std::string& default_country_code = "255");

bool is_valid_tz_mobile(const std::string& raw);

struct SMSAnalysis {
    std::string encoding;  // "GSM-7" or "UCS-2"
    size_t characters = 0;
    int parts = 0;
    int credits = 0;
};

// Local-only, no network call. Mirrors standard SMS aggregator billing:
// GSM-7 160/153 chars per segment (single/concatenated), UCS-2 70/67.
SMSAnalysis analyze_sms(const std::string& text);

// Throws ValidationError if sender exceeds the 11-char GSM alphanumeric
// sender ID cap. Returns the trimmed sender, or empty if input was empty.
std::string validate_sender_id(const std::string& sender);

// Throws ValidationError if amount <= 0.
void validate_positive_amount(double amount, const std::string& field_name = "amount");

// Throws ValidationError if value is empty.
void require_nonempty(const std::string& value, const std::string& field_name);

}  // namespace sendafrica::util
