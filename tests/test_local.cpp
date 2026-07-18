// Minimal hand-rolled test harness (no gtest dependency) covering the
// local-only logic: phone normalization, SMS analysis, validators.

#include <sendafrica/sendafrica.hpp>

#include <iostream>
#include <string>

using namespace sendafrica;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                                    \
    do {                                                                                \
        g_checks++;                                                                     \
        if (!(cond)) {                                                                  \
            g_failures++;                                                               \
            std::cerr << "FAIL: " << #cond << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        }                                                                                \
    } while (0)

#define CHECK_THROWS(expr, ExcType)                                                     \
    do {                                                                                \
        g_checks++;                                                                     \
        bool threw = false;                                                             \
        try {                                                                           \
            expr;                                                                       \
        } catch (const ExcType&) {                                                      \
            threw = true;                                                               \
        } catch (...) {                                                                 \
        }                                                                               \
        if (!threw) {                                                                   \
            g_failures++;                                                               \
            std::cerr << "FAIL: expected " #ExcType " from " #expr "\n";                \
        }                                                                                \
    } while (0)

void test_normalize_phone() {
    CHECK(util::normalize_phone("0712345678") == "+255712345678");
    CHECK(util::normalize_phone("+255712345678") == "+255712345678");
    CHECK(util::normalize_phone("255712345678") == "+255712345678");
    CHECK(util::normalize_phone("712345678") == "+255712345678");
    CHECK(util::normalize_phone("+255 712 345 678") == "+255712345678");
    CHECK_THROWS(util::normalize_phone(""), InvalidPhoneError);
    CHECK_THROWS(util::normalize_phone("abc"), InvalidPhoneError);
}

void test_is_valid_tz_mobile() {
    CHECK(util::is_valid_tz_mobile("0712345678") == true);
    CHECK(util::is_valid_tz_mobile("+14155552671") == false);
}

void test_analyze_sms() {
    auto r1 = util::analyze_sms("Hello world");
    CHECK(r1.encoding == "GSM-7");
    CHECK(r1.parts == 1);
    CHECK(r1.credits == 1);

    auto r2 = util::analyze_sms(std::string(200, 'a'));
    CHECK(r2.encoding == "GSM-7");
    CHECK(r2.parts == 2);

    auto r3 = util::analyze_sms("");
    CHECK(r3.parts == 0);
    CHECK(r3.credits == 0);
}

void test_validators() {
    CHECK_THROWS(util::validate_sender_id("ThisSenderIdIsWayTooLong"), ValidationError);
    CHECK(util::validate_sender_id("MyBrand") == "MyBrand");
    CHECK_THROWS(util::validate_positive_amount(0), ValidationError);
    CHECK_THROWS(util::validate_positive_amount(-5), ValidationError);
}

void test_client_construction_and_local_calls() {
    // No network calls here — just verifies the client wires up correctly
    // and local-only resource methods (analyze) work end to end.
    ClientOptions opts;
    opts.debug = false;
    Client client(std::string("sk_test_123"), opts);
    CHECK(client.environment() == "production");

    auto analysis = client.sms().analyze("Habari \xF0\x9F\x98\x8A");  // "Habari 😊"
    CHECK(analysis.encoding == "UCS-2");
    CHECK(analysis.parts == 1);
}

void test_missing_api_key_throws() {
    unsetenv("SENDAFRICA_API_KEY");
    CHECK_THROWS(Client client(std::nullopt), AuthenticationError);
}

int main() {
    test_normalize_phone();
    test_is_valid_tz_mobile();
    test_analyze_sms();
    test_validators();
    test_client_construction_and_local_calls();
    test_missing_api_key_throws();

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
