#include "Hashing.h"

#include <map>

#include <boost/bimap.hpp>

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4701)
// C4701: Potentially uninitialized local variable 'result' used
#include <boost/crc.hpp>
#pragma warning(pop)
#else
#include <boost/crc.hpp>
#endif

namespace bayan {

namespace {

template <typename L, typename R>
boost::bimap<L, R>
MakeBimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

const auto HASH_TYPE_LOOKUP =
    MakeBimap<std::string, HashType>({{"crc16", HashType::CRC16},
                                      {"crc32", HashType::CRC32},
                                      {"crc64", HashType::CRC64}});

template <typename Hasher>
[[nodiscard]] inline BlockHashFunction GetHashFunction() {
  return [](const std::vector<char> &data) -> BlockHash {
    Hasher hasher;
    hasher.process_bytes(data.data(), data.size());
    return hasher.checksum();
  };
}

} // namespace

BlockHashFunction GetHashFunction(HashType type) {
  constexpr auto CRC64_SIZE = 64;
  constexpr auto CRC64_POLYNOME = 0x04C11DB7;
  using CRC64 =
      boost::crc_optimal<CRC64_SIZE, CRC64_POLYNOME, 0, 0, false, false>;
  switch (type) {
  case HashType::CRC16:
    return GetHashFunction<boost::crc_16_type>();
  case HashType::CRC32:
    return GetHashFunction<boost::crc_32_type>();
  case HashType::CRC64:
    return GetHashFunction<CRC64>();
  default:
    throw std::invalid_argument("Unknown hash type");
  }
}

std::istream &operator>>(std::istream &in, HashType &type) {
  std::string tmp;
  in >> tmp;

  if (auto it = HASH_TYPE_LOOKUP.left.find(tmp);
      it != HASH_TYPE_LOOKUP.left.end()) {
    type = it->second;
  } else {
    throw std::invalid_argument("Invalid hash type: " + tmp);
  }

  return in;
}

std::ostream &operator<<(std::ostream &out, HashType type) {
  if (auto it = HASH_TYPE_LOOKUP.right.find(type);
      it != HASH_TYPE_LOOKUP.right.end()) {
    out << it->second;
  } else {
    throw std::invalid_argument("Unexpected hash type value: " +
                                std::to_string(static_cast<int>(type)));
  }
  return out;
}

} // namespace bayan