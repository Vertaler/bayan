#include <set>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
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
    if (excludeDirs.count(it->path()) != 0) {
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

    BOOST_LOG_TRIVIAL(info) << "Start file collection";
    for (const auto &dir : query.dirs) {
      BOOST_LOG_TRIVIAL(info) << "Process directory: " << dir;
      try {
        ProcessDirectory(dir, query, result);
      } catch (std::exception &e) {
        BOOST_LOG_TRIVIAL(error)
            << "Error occuring during processing directory " << dir << ": "
            << e.what();
      }
    }

    BOOST_LOG_TRIVIAL(info)
        << "File collection finished. Count of files to process: "
        << result.size();
    return result;
  }

private:
  void ProcessDirectory(const fs::path &dir, const FileQuery &query,
                        std::vector<FileInfo> &result) const {
    fs::recursive_directory_iterator end;
    boost::system::error_code ec{};
    for (auto it = fs::recursive_directory_iterator(dir); it != end;
         it.increment(ec)) {

      if (ec) {
        BOOST_LOG_TRIVIAL(warning)
            << "Error occured on recursive_directory_iterator increment: "
            << ec.message();
        it.no_push();
        ec.clear();
        continue;
      }
      try {
        BOOST_LOG_TRIVIAL(debug) << "Processing filesystem item " << it->path()
                                 << ". Recursion depth: " << it.depth();
        const auto fileType = it->status().type();
        if (fileType == fs::regular_file &&
            query.FileShouldBeAdded(it->path())) {
          const auto size = fs::file_size(it->path());
          result.emplace_back(it->path(), size);
        }

        if (fileType == fs::directory_file &&
            !query.DirectoryShouldBeProcessed(it)) {
          it.no_push();
        }
      } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(warning)
            << "Error occurred during processing filesystem item " << *it
            << ": " << e.what();
      }
    }
  }
};

} // namespace bayan