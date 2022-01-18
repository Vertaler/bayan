#pragma once

#include <list>
#include <map>
#include <vector>

#include "FileInfo.h"
#include "Hashing.h"

namespace bayan {

struct FileDuplicatesSearherOptions {
  BlockHashFunction hashFunction;
  size_t blockSize{4096};
};

class FileDuplicatesSearher {
public:
  explicit FileDuplicatesSearher(FileDuplicatesSearherOptions options)
      : m_options(std::move(options)) {}

  std::vector<std::vector<boost::filesystem::path>>
  FindDuplicates(const std::vector<FileInfo> &files) {
    /**
     * @brief Helper struct for map key
     *
     */
    struct Key {
      Key(uintmax_t size) : fileSize(size) {}

      uintmax_t fileSize{};
      std::vector<BlockHash> blockHashes;

      [[nodiscard]] bool operator<(const Key &rhs) const noexcept {
        return fileSize < rhs.fileSize && blockHashes < rhs.blockHashes;
      }
    };

    std::map<Key, std::list<boost::filesystem::path>> workMap;

    const auto addItemToWorkMap = [&workMap](const Key &key,
                                             boost::filesystem::path path) {
      if (const auto it = workMap.find(key); it != workMap.end()) {
        it->second.push_back(std::move(path));
      } else {
        workMap.emplace(key, std::list{std::move(path)});
      }
    };

    for (const auto &file : files) {
      addItemToWorkMap(file.size, file.path);
    }

    const auto &blockSize = m_options.blockSize;
    std::vector<char> readBuffer(blockSize, 0);
    for (auto it = workMap.begin(); it != workMap.end(); ++it) {
      const auto blockTotalSize = it->first.blockHashes.size() * blockSize;
      const auto &fileSize = it->first.fileSize;
      auto &pathList = it->second;

      if ((pathList.size() <= 1) || (blockTotalSize >= fileSize)) {
        continue;
      }

      if ((fileSize - blockTotalSize) < blockSize) {
        std::fill(readBuffer.begin(), readBuffer.end(), '\0');
      }
      for (auto listIt = pathList.begin(); listIt != pathList.end();
           listIt = pathList.erase(listIt)) {
        auto path = std::move(*listIt);
        boost::filesystem::ifstream in(path);
        in.seekg(blockTotalSize);
        in.read(readBuffer.data(), readBuffer.size());
        const auto hash = m_options.hashFunction(readBuffer);
        auto newKey = it->first;
        newKey.blockHashes.push_back(hash);
        addItemToWorkMap(newKey, std::move(path));
      }
    }

    std::vector<std::vector<boost::filesystem::path>> result;
    for (const auto &[key, paths] : workMap) {
      if (paths.size() < 2) {
        continue;
      }

      result.emplace_back(std::make_move_iterator(paths.begin()),
                          std::make_move_iterator(paths.end()));
    }

    return result;
  }

private:
  FileDuplicatesSearherOptions m_options;
};

} // namespace bayan