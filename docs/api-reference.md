# API Reference

Complete reference for all public types and methods in the SendAfrica C++ SDK.

## Namespace

Everything lives under `sendafrica::`. Use the umbrella header for convenience:

```cpp
#include <sendafrica/sendafrica.hpp>
```

Or include individual headers for faster compilation:

```cpp
#include <sendafrica/client.hpp>
#include <sendafrica/models.hpp>
#include <sendafrica/exceptions.hpp>
```

---

## Client

**Header:** `sendafrica/client.hpp`

```cpp
namespace sendafrica {

struct ClientOptions {
    std::string base_url          = "https://api.sendafrica.online/v1";
    double timeout_seconds        = 10.0;
    int max_retries               = 3;
    std::string environment       = "production";
    bool debug                    = false;
    std::optional<std::string> webhook_secret;
};

class Client {
public:
    explicit Client(
        std::optional<std::string> api_key = std::nullopt,
        ClientOptions options = {}
    );

    SMSResource&      sms();
    CreditsResource&  credits();
    PaymentsResource& payments();
    WebhooksResource& webhooks();

    const std::string& environment() const;
};

}  // namespace sendafrica
```

### Constructor

| Parameter | Type | Default | Description |
|---|---|---|---|
| `api_key` | `optional<string>` | `nullopt` | API key. If `nullopt`, reads from `SENDAFRICA_API_KEY` env var. |
| `options` | `ClientOptions` | `{}` | Configuration (timeout, retries, debug, etc.) |

### ClientOptions

| Field | Type | Default | Description |
|---|---|---|---|
| `base_url` | `string` | `https://api.sendafrica.online/v1` | API base URL |
| `timeout_seconds` | `double` | `10.0` | HTTP request timeout |
| `max_retries` | `int` | `3` | Retries on 429/5xx/connection errors |
| `environment` | `string` | `"production"` | Label for the environment |
| `debug` | `bool` | `false` | Log requests to stderr |
| `webhook_secret` | `optional<string>` | `nullopt` | HMAC-SHA256 secret for webhook verification |

---

## SMSResource

**Header:** `sendafrica/resources.hpp`

Access via `client.sms()`.

### send()

Send a single SMS.

```cpp
SMSResult send(
    const std::string& to,
    const std::string& message,
    const std::optional<std::string>& sender = std::nullopt
);
```

| Parameter | Description |
|---|---|
| `to` | Phone number (any format — normalized to E.164 locally) |
| `message` | SMS body (max 160 GSM-7 or 70 UCS-2 for single part) |
| `sender` | Optional alphanumeric sender ID (max 11 chars) |

**Returns:** `SMSResult` with `message_id`, `status`, `credits_used`, `cost`, `to`.

**Throws:** `InvalidPhoneError`, `ValidationError`, `AuthenticationError`, `RateLimitError`, etc.

```cpp
auto result = client.sms().send("0712345678", "Hello!");
std::cout << result.message_id << "\n";   // "SA-xxxx-xxxx-xxxx"
std::cout << result.status << "\n";       // "Success"
std::cout << result.credits_used << "\n"; // 1
std::cout << *result.cost << "\n";        // "TZS 22.0000"
```

### send_many()

Send multiple SMS with rate limiting. Failures are captured per-message
rather than aborting the batch.

```cpp
BulkSMSResult send_many(
    const std::vector<BulkItem>& messages,
    const std::optional<std::string>& sender = std::nullopt,
    double rate_limit_per_sec = 10.0
);
```

**BulkItem struct:**

```cpp
struct BulkItem {
    std::string to;
    std::string message;
    std::optional<std::string> sender;
};
```

**Returns:** `BulkSMSResult` with `results`, `failed`, `sent_count()`, `failed_count()`.

```cpp
std::vector<SMSResource::BulkItem> items = {
    {"+255711111111", "Hello John"},
    {"+255722222222", "Hello Mary"},
};
auto bulk = client.sms().send_many(items, "MyBrand");
std::cout << bulk.sent_count() << " sent, " << bulk.failed_count() << " failed\n";
```

### analyze()

Local-only SMS analysis. No network call — estimates encoding, parts,
and credit cost before sending.

```cpp
util::SMSAnalysis analyze(const std::string& message) const;
```

**Returns:** `SMSAnalysis` with `encoding`, `characters`, `parts`, `credits`.

```cpp
auto a = client.sms().analyze("Hello world");
// a.encoding = "GSM-7", a.parts = 1, a.credits = 1
```

See [SMS Analysis](sms-analysis.md) for encoding details.

---

## CreditsResource

**Header:** `sendafrica/resources.hpp`

Access via `client.credits()`.

### balance()

```cpp
CreditBalance balance();
```

**Returns:** `CreditBalance` with `account_id` and `balance`.

```cpp
auto b = client.credits().balance();
std::cout << b.account_id << ": " << b.balance << " credits\n";
```

### history()

List credit transactions with page-based pagination.

```cpp
std::vector<CreditTransaction> history(int page = 1, int per_page = 25);
```

| Parameter | Description |
|---|---|
| `page` | Page number (1-indexed) |
| `per_page` | Results per page (max 200) |

**Returns:** Vector of `CreditTransaction`.

```cpp
auto txns = client.credits().history(1, 10);
for (const auto& t : txns) {
    std::cout << t.id << "  " << t.type << "  " << t.amount << "\n";
}
```

---

## PaymentsResource

**Header:** `sendafrica/resources.hpp`

Access via `client.payments()`.

Credit top-ups are pay-as-you-go: you pick any TZS amount and the API
converts it to credits at the current tiered rate.

### create()

```cpp
Payment create(
    int amount,
    const std::string& provider = "manual",
    const std::optional<std::string>& phone = std::nullopt
);
```

| Parameter | Description |
|---|---|
| `amount` | Amount in TZS (positive integer) |
| `provider` | Payment provider (`"manual"`, `"snippe"`, etc.) |
| `phone` | Required for mobile-money providers, optional for `"manual"` |

**Returns:** `Payment` with `id`, `status`, `amount`, `credit_amount`, `currency`, `provider`.

```cpp
auto p = client.payments().create(50000, "snippe", "+255712345678");
std::cout << p.id << "  " << p.status << "\n";
```

### rate()

Fetch the minimum top-up amount and tiered TZS-per-credit pricing schedule.

```cpp
VoucherRate rate();
```

**Returns:** `VoucherRate` with `min_amount_tzs` and `tiers`.

```cpp
auto vr = client.payments().rate();
std::cout << "Minimum: TZS " << vr.min_amount_tzs << "\n";
for (const auto& t : vr.tiers) {
    std::cout << "  up to TZS " << t.max_amount_tzs
              << " = " << t.rate_tzs_per_credit << " TZS/credit\n";
}
```

**Note:** `payments.get()` and `payments.list()` don't exist — the API only
exposes `POST /v1/vouchers` (create) and `GET /v1/vouchers/rate` (pricing)
to API key callers. Order listing is an admin-console (JWT) feature.

---

## WebhooksResource

**Header:** `sendafrica/resources.hpp`

Access via `client.webhooks()`.

### parse()

Parse and optionally verify an incoming webhook payload using HMAC-SHA256.

```cpp
WebhookEvent parse(
    const std::string& payload,
    const std::optional<std::string>& signature = std::nullopt,
    const std::optional<std::string>& secret = std::nullopt
);
```

| Parameter | Description |
|---|---|
| `payload` | Raw JSON body from the webhook request |
| `signature` | Value of `X-SendAfrica-Signature` header |
| `secret` | HMAC secret (or use the one configured on the client) |

**Returns:** `WebhookEvent` with `type`, `message_id`, `data`.

**Throws:** `WebhookSignatureError` if signature verification fails.

```cpp
auto event = client.webhooks().parse(
    raw_body,
    signature_header,
    "your_webhook_secret"
);

if (event.type == "sms.delivered") {
    std::cout << "Delivered: " << *event.message_id << "\n";
}
```

See [Webhooks](webhooks.md) for full setup guide.

---

## Response Models

**Header:** `sendafrica/models.hpp`

### SMSResult

| Field | Type | Description |
|---|---|---|
| `message_id` | `string` | Unique message ID (e.g. `"SA-xxxx-xxxx-xxxx"`) |
| `status` | `string` | `"Success"` or error status |
| `credits_used` | `int` | Number of credits consumed |
| `cost` | `optional<string>` | Human-readable cost (e.g. `"TZS 22.0000"`) |
| `to` | `optional<string>` | Recipient phone number |

### BulkSMSResult

| Field | Type | Description |
|---|---|---|
| `results` | `vector<SMSResult>` | Successfully sent messages |
| `failed` | `vector<BulkSMSFailure>` | Per-message failures |
| `sent_count()` | method | `results.size()` |
| `failed_count()` | method | `failed.size()` |

### BulkSMSFailure

| Field | Type | Description |
|---|---|---|
| `index` | `int` | Index in the original batch |
| `to` | `string` | Recipient phone |
| `error` | `string` | Error message |

### CreditBalance

| Field | Type | Description |
|---|---|---|
| `account_id` | `string` | Account identifier |
| `balance` | `long long` | Available credits |

### CreditTransaction

| Field | Type | Description |
|---|---|---|
| `id` | `string` | Transaction ID |
| `type` | `string` | `"debit"`, `"credit"`, etc. |
| `amount` | `long long` | Transaction amount |
| `balance_after` | `long long` | Balance after transaction |
| `description` | `optional<string>` | Human-readable description |
| `created_at` | `optional<string>` | ISO 8601 timestamp |

### Payment

| Field | Type | Description |
|---|---|---|
| `id` | `string` | Payment ID |
| `status` | `string` | `"pending"`, `"completed"`, etc. |
| `amount` | `optional<int>` | Amount in TZS |
| `credit_amount` | `optional<int>` | Credits purchased |
| `currency` | `string` | `"TZS"` |
| `provider` | `optional<string>` | Payment provider |
| `source` | `optional<string>` | Payment source |
| `created_at` | `optional<string>` | ISO 8601 timestamp |

### RateTier

| Field | Type | Description |
|---|---|---|
| `max_amount_tzs` | `int` | Upper bound of this tier (0 = unbounded) |
| `rate_tzs_per_credit` | `int` | TZS per credit in this tier |

### VoucherRate

| Field | Type | Description |
|---|---|---|
| `min_amount_tzs` | `int` | Minimum top-up amount |
| `tiers` | `vector<RateTier>` | Tiered pricing brackets |

### WebhookEvent

| Field | Type | Description |
|---|---|---|
| `type` | `string` | Event type (e.g. `"sms.delivered"`) |
| `message_id` | `optional<string>` | Related message ID |
| `data` | `json` | Full event payload |

---

## Utility Functions

**Header:** `sendafrica/util.hpp`

### normalize_phone()

```cpp
std::string normalize_phone(
    const std::string& raw,
    const std::string& default_country_code = "255"
);
```

Normalizes a phone number to E.164 format. See [Phone Normalization](phone-normalization.md).

### is_valid_tz_mobile()

```cpp
bool is_valid_tz_mobile(const std::string& raw);
```

Returns `true` if the number normalizes to a Tanzanian mobile number.

### analyze_sms()

```cpp
SMSAnalysis analyze_sms(const std::string& text);
```

Same as `client.sms().analyze()` — available without a client instance.

### validate_sender_id()

```cpp
std::string validate_sender_id(const std::string& sender);
```

Returns trimmed sender or empty string. Throws `ValidationError` if > 11 chars.

### validate_positive_amount()

```cpp
void validate_positive_amount(double amount, const std::string& field_name = "amount");
```

Throws `ValidationError` if `amount <= 0`.

### require_nonempty()

```cpp
void require_nonempty(const std::string& value, const std::string& field_name);
```

Throws `ValidationError` if `value` is empty.

---

## Version

```cpp
namespace sendafrica {
inline constexpr const char* kVersion = "1.0.1";
}
```

Use `sendafrica::kVersion` to check the SDK version at compile time.
