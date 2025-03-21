#include "gecko.hpp"

#include <string>
#include <vector>
#include <ranges>

#include "base64.hpp"
#include "nss3.hpp"
#include "asn1.hpp"

DecodedPayload Gecko::DecodePayload(const std::string& payload) {
    const auto decoded = base64_decode(payload);
    const auto [iv, encrypted] = DecodeDER(decoded);
    return { encrypted, iv };
}
