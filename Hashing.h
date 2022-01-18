#pragma once

#include <functional>
#include <iostream>

namespace bayan {

using BlockHash = uint64_t;
using BlockHashFunction = std::function<BlockHash(const std::vector<char> &)>;

enum class HashType { CRC16, CRC32, CRC64 };

[[nodiscard]] BlockHashFunction GetHashFunction(HashType type);

std::istream &operator>>(std::istream &in, HashType &type);
std::ostream &operator<<(std::ostream &out, HashType type);

} // namespace bayan