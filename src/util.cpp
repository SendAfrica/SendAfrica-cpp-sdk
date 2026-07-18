#include "sendafrica/util.hpp"
#include "sendafrica/exceptions.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <unordered_set>

namespace sendafrica::util {

namespace {

// GSM 03.38 basic character set (simplified, covers the common case).
const std::string kGsm7Basic =
    "@\xC2\xA3$\xC2\xA5\xC3\xA8\xC3\xA9\xC3\xB9\xC3\xAC\xC3\xB2\xC3\x87\n\xC3\x98\xC3\xB8\r"
    "\xC3\x85\xC3\xA5\xCE\x94_\xCE\xA6\xCE\x93\xCE\x9B\xCE\xA9\xCE\xA0\xCE\xA8\xCE\xA3\xCE\x98\xCE\x9E"
    " !\"#\xC2\xA4%&'()*+,-./0123456789:;<=>?"
    "\xC2\xA1"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ\xC3\x84\xC3\x96\xC3\x91\xC3\x9C\xC2\xA7"
    "\xC2\xBF"
    "abcdefghijklmnopqrstuvwxyz\xC3\xA4\xC3\xB6\xC3\xB1\xC3\xBC\xC3\xA0";

bool is_gsm7(const std::string& text) {
    // Byte-level check against the (UTF-8 encoded) GSM-7 basic set. Good
    // enough for the common ASCII/Latin-1 case this SDK targets; anything
    // with true multi-byte UTF-8 outside that set correctly falls to UCS-2.
    for (unsigned char c : text) {
        if (kGsm7Basic.find(static_cast<char>(c)) == std::string::npos) {
            // Multi-byte sequences won't match single-byte find reliably for
            // all cases, so conservatively treat anything non-ASCII as UCS-2.
            if (c > 127) return false;
        }
    }
    return true;
}

constexpr int kGsm7Single = 160;
constexpr int kGsm7Concat = 153;
constexpr int kUcs2Single = 70;
constexpr int kUcs2Concat = 67;
constexpr size_t kMaxSenderIdLength = 11;

std::string strip_non_digits_plus(const std::string& raw) {
    std::string out;
    for (char c : raw) {
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '+') out.push_back(c);
    }
    return out;
}

}  // namespace

std::string normalize_phone(const std::string& raw, const std::string& default_country_code) {
    if (raw.empty()) {
        throw InvalidPhoneError("Phone number must be a non-empty string");
    }

    std::string digits = strip_non_digits_plus(raw);
    std::string candidate;

    static const std::regex tz_mobile_prefix("^(6|7)\\d{8}$");

    if (!digits.empty() && digits[0] == '+') {
        candidate = digits.substr(1);
    } else if (digits.rfind("00", 0) == 0) {
        candidate = digits.substr(2);
    } else if (!digits.empty() && digits[0] == '0') {
        candidate = default_country_code + digits.substr(1);
    } else if (digits.rfind(default_country_code, 0) == 0) {
        candidate = digits;
    } else if (std::regex_match(digits, tz_mobile_prefix)) {
        candidate = default_country_code + digits;
    } else {
        candidate = digits;
    }

    bool all_digits = !candidate.empty() &&
        std::all_of(candidate.begin(), candidate.end(), [](unsigned char c) { return std::isdigit(c); });

    if (!all_digits || candidate.size() < 9 || candidate.size() > 15) {
        throw InvalidPhoneError("'" + raw + "' is not a valid phone number");
    }

    return "+" + candidate;
}

bool is_valid_tz_mobile(const std::string& raw) {
    std::string normalized;
    try {
        normalized = normalize_phone(raw);
    } catch (const InvalidPhoneError&) {
        return false;
    }
    if (normalized.rfind("+255", 0) != 0) return false;
    std::string local = normalized.substr(4);
    static const std::regex tz_mobile_prefix("^(6|7)\\d{8}$");
    return std::regex_match(local, tz_mobile_prefix);
}

SMSAnalysis analyze_sms(const std::string& text) {
    SMSAnalysis result;
    result.characters = text.size();

    if (text.empty()) {
        result.encoding = "GSM-7";
        result.parts = 0;
        result.credits = 0;
        return result;
    }

    int single_limit, concat_limit;
    if (is_gsm7(text)) {
        result.encoding = "GSM-7";
        single_limit = kGsm7Single;
        concat_limit = kGsm7Concat;
    } else {
        result.encoding = "UCS-2";
        single_limit = kUcs2Single;
        concat_limit = kUcs2Concat;
    }

    size_t length = result.characters;
    int parts;
    if (static_cast<int>(length) <= single_limit) {
        parts = 1;
    } else {
        parts = static_cast<int>((length + concat_limit - 1) / concat_limit);
    }

    result.parts = parts;
    result.credits = parts;
    return result;
}

std::string validate_sender_id(const std::string& sender) {
    std::string trimmed = sender;
    // trim whitespace
    trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(),
                  [](unsigned char c) { return !std::isspace(c); }));
    trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(),
                  [](unsigned char c) { return !std::isspace(c); }).base(), trimmed.end());

    if (trimmed.empty()) return "";
    if (trimmed.size() > kMaxSenderIdLength) {
        throw ValidationError("sender id '" + trimmed + "' exceeds " +
                               std::to_string(kMaxSenderIdLength) + " characters");
    }
    return trimmed;
}

void validate_positive_amount(double amount, const std::string& field_name) {
    if (amount <= 0) {
        throw ValidationError("'" + field_name + "' must be a positive number");
    }
}

void require_nonempty(const std::string& value, const std::string& field_name) {
    if (value.empty()) {
        throw ValidationError("'" + field_name + "' is required");
    }
}

}  // namespace sendafrica::util
