# Phone Normalization

The SendAfrica SDK normalizes phone numbers to E.164 format before sending
them to the API. This happens locally (no network call), so bad numbers
fail fast.

## E.164 format

E.164 is the international phone number standard: `+` followed by country
code and local number, with no spaces or dashes.

```
+255712345678
```

## Supported input formats

The SDK accepts all of these and normalizes them to `+255712345678`:

| Input | Normalized |
|---|---|
| `0712345678` | `+255712345678` |
| `712345678` | `+255712345678` |
| `255712345678` | `+255712345678` |
| `+255712345678` | `+255712345678` |
| `+255 712 345 678` | `+255712345678` |
| `0712 345 678` | `+255712345678` |
| `00255712345678` | `+255712345678` |

## How it works

```cpp
#include <sendafrica/util.hpp>

std::string normalized = sendafrica::util::normalize_phone("0712345678");
// "+255712345678"
```

### Normalization rules

1. **Strip non-digits** (except leading `+`)
2. **Apply prefix rules:**
   - Starts with `+` → use digits after `+`
   - Starts with `00` → use digits after `00`
   - Starts with `0` → prepend default country code (`255`) + digits after `0`
   - Matches TZ mobile pattern (6x/7x + 8 digits) → prepend `255`
   - Otherwise → use as-is
3. **Validate:** must be 9-15 digits, all numeric

### Default country code

The default country code is `255` (Tanzania). You can override it:

```cpp
std::string normalized = sendafrica::util::normalize_phone(
    "0712345678",
    "254"  // Kenya
);
```

## Tanzanian mobile validation

```cpp
bool valid = sendafrica::util::is_valid_tz_mobile("0712345678");  // true
bool valid = sendafrica::util::is_valid_tz_mobile("+14155552671"); // false
```

Returns `true` only if the number normalizes to a Tanzanian mobile:
- Country code `+255`
- Local part starts with `6` or `7`
- Local part is exactly 9 digits

### TZ mobile prefixes

| Prefix | Carrier |
|---|---|
| 65x-69x | Vodacom, Tigo/Mixx, Airtel, Halotel, TTCL |
| 71x-79x | Vodacom, Tigo/Mixx, Airtel, Halotel, TTCL |

## Error handling

Invalid numbers throw `InvalidPhoneError` (a subclass of `ValidationError`):

```cpp
try {
    sendafrica::util::normalize_phone("");
} catch (const sendafrica::InvalidPhoneError& e) {
    std::cerr << e.message() << "\n";
    // "Phone number must be a non-empty string"
}

try {
    sendafrica::util::normalize_phone("abc");
} catch (const sendafrica::InvalidPhoneError& e) {
    std::cerr << e.message() << "\n";
    // "'abc' is not a valid phone number"
}
```
