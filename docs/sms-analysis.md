# SMS Analysis

The SDK includes a local-only SMS text analyzer that estimates encoding,
segment count, and credit cost — without making a network call.

## Usage

```cpp
#include <sendafrica/sendafrica.hpp>

// Via client (same function)
auto result = client.sms().analyze("Hello world");

// Via utility (no client needed)
auto result = sendafrica::util::analyze_sms("Hello world");
```

## SMSAnalysis struct

```cpp
struct SMSAnalysis {
    std::string encoding;  // "GSM-7" or "UCS-2"
    size_t characters = 0; // character count
    int parts = 0;         // number of SMS segments
    int credits = 0;       // estimated credit cost (1 per segment)
};
```

## GSM-7 vs UCS-2

| Encoding | Characters | Single segment | Concatenated |
|---|---|---|---|
| GSM-7 | Basic Latin + symbols | 160 chars | 153 chars/segment |
| UCS-2 | Unicode (emoji, etc.) | 70 chars | 67 chars/segment |

### GSM-7 character set

GSM-7 covers:
- A-Z, a-z, 0-9
- Common symbols: `@£$¥èéùìòÇØøÅåΔ_ΦΓΛΩΠΨΣΘΞÆæßÉ`
- Space, `! "#¤%&'()*+,-./:;<=>?¡ÄÖÑÜ¿`
- Some Greek letters (Δ, Φ, Γ, Λ, Ω, Π, Ψ, Σ, Θ, Ξ)

Anything outside this set forces UCS-2 encoding.

### When UCS-2 kicks in

- Emoji (any Unicode emoji)
- Non-Latin scripts (Arabic, Chinese, etc.)
- Some accented characters outside GSM-7

## Examples

```cpp
// Single GSM-7 segment
auto a1 = sendafrica::util::analyze_sms("Hello world");
// encoding="GSM-7", characters=11, parts=1, credits=1

// UCS-2 due to emoji
auto a2 = sendafrica::util::analyze_sms("Habari 😊");
// encoding="UCS-2", characters=8, parts=1, credits=1

// GSM-7 concatenated (2 parts)
auto a3 = sendafrica::util::analyze_sms(std::string(200, 'a'));
// encoding="GSM-7", characters=200, parts=2, credits=2

// Empty string
auto a4 = sendafrica::util::analyze_sms("");
// encoding="GSM-7", characters=0, parts=0, credits=0
```

## Segment calculation

```
if length <= single_limit:
    parts = 1
else:
    parts = ceil(length / concat_limit)
```

- GSM-7: `single_limit=160`, `concat_limit=153`
- UCS-2: `single_limit=70`, `concat_limit=67`

Credit cost equals the number of segments (1 credit per segment).

## Use cases

1. **Pre-send cost estimation** — show the user how many credits before sending
2. **Message length validation** — warn if the message will be split
3. **Character set validation** — check if the message uses emoji/Unicode
4. **Batch planning** — estimate total credits for bulk sends
