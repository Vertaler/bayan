#include <set>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>

#include "FileInfo.h"

namespace bayan {

namespace fs = boost::filesystem;

struct FileQuery {
  boost::optional<boost::regex> fileNamePattern;
  std::vector<fs::path> dirs;
  std::set<fs::path> excludeDirs;
  boost::optional<int> recursionDepth;
  uintmax_t minFileSize{1};

  [[nodiscard]] bool FileShouldBeAdded(const fs::path &path) const {
    if (fs::file_size(path) < minFileSize) {
      return false;
    }

    if (fileNamePattern &&
        !boost::regex_match(path.filename().string(), *fileNamePattern)) {
      return false;
    }

    return true;
  }

  [[nodiscard]] bool
  DirectoryShouldBeProcessed(const fs::recursive_directory_iterator &it) const {
    assert(it->status().type() == fs::file_type::directory_file);
    if (excludeDirs.count(it->path()) == 0) {
      return false;
    }

    if (recursionDepth && (it.depth() > *recursionDepth)) {
      return false;
    }

    return true;
  }
};

class FileCollector {
public:
  [[nodiscard]] std::vector<FileInfo>
  CollectFiles(const FileQuery &query) const {
    std::vector<FileInfo> result;

    for (const auto &dir : query.dirs) {
      std::cout << "Process directory: " << dir << std::endl;
      ProcessDirectory(dir, query, result);
    }

    std::cout << "File collection finished" << std::endl;
    return result;
  }

private:
  void ProcessDirectory(const fs::path &dir, const FileQuery &query,
                        std::vector<FileInfo> &result) const {
    fs::recursive_directory_iterator end;
    for (auto it = fs::recursive_directory_iterator(dir); it != end; ++it) {

      const auto fileType = it->status().type();
      if (fileType == fs::regular_file && query.FileShouldBeAdded(it->path())) {
        const auto size = fs::file_size(it->path());
        result.emplace_back(it->path(), size);
      }

      if (fileType == fs::directory_file &&
          !query.DirectoryShouldBeProcessed(it)) {
        it.no_push();
      }
    }
  }
};

} // namespace bayan