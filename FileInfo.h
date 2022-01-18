#pragma once

#include <boost/filesystem.hpp>

namespace bayan {

struct FileInfo {
  FileInfo(boost::filesystem::path p, uintmax_t s)
      : path(std::move(p)), size(s) {}
  FileInfo() = default;

  boost::filesystem::path path;
  uintmax_t size{};
};

} // namespace bayan