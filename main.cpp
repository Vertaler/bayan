#include <iostream>

#include <boost/program_options.hpp>

#include "FileCollector.h"
#include "FileDuplicatesSearher.h"

namespace options = boost::program_options;

int main(int argc, char **argv) try {
  options::options_description desc("Tool for searching file duplicates");

  bayan::FileQuery fileQuery{};
  bayan::FileDuplicatesSearherOptions duplicateSearherOptions{};
  bayan::HashType hashType{};

  // clang-format off
  desc.add_options()
    ("help,h", "Show help")
    ("dirs", options::value<std::vector<boost::filesystem::path>>(&fileQuery.dirs)->multitoken()->required())
    ("exclude_dirs", options::value<std::vector<boost::filesystem::path>>()->multitoken())
    ("min_file_size", options::value<uintmax_t>(&fileQuery.minFileSize)->default_value(1))
    ("filename_pattern", options::value<std::string>())
    ("recursion_depth", options::value<std::string>())
    ("hash", options::value<bayan::HashType>(&hashType)->default_value(bayan::HashType::CRC32))
    ("block_size", options::value<size_t>(&duplicateSearherOptions.blockSize)->default_value(4096));
  // clang-format on

  bayan::FileCollector collector;
  options::variables_map optionsMap;
  options::parsed_options parsed =
      options::command_line_parser(argc, argv).options(desc).run();
  options::store(parsed, optionsMap);

  if (optionsMap.count("help")) {
    std::cout << desc;
    return EXIT_SUCCESS;
  }

  if (optionsMap.count("exclude_dirs") != 0) {
    auto exludeDirs =
        optionsMap["exclude_dirs"].as<std::vector<boost::filesystem::path>>();

    fileQuery.excludeDirs.insert(std::make_move_iterator(exludeDirs.begin()),
                                 std::make_move_iterator(exludeDirs.end()));
  }

  if (optionsMap.count("recursion_depth") != 0) {
    fileQuery.recursionDepth = optionsMap["recursion_depth"].as<int>();
  }

  if (optionsMap.count("filename_pattern") != 0) {
    fileQuery.fileNamePattern.emplace(
        optionsMap["filename_pattern"].as<std::string>(), boost::regex::icase);
  }

  options::notify(optionsMap);
  duplicateSearherOptions.hashFunction = bayan::GetHashFunction(hashType);

  const auto filePaths = collector.CollectFiles(fileQuery);

  bayan::FileDuplicatesSearher duplicatesSearcher{duplicateSearherOptions};
  const auto duplicatesList = duplicatesSearcher.FindDuplicates(filePaths);

  for (const auto &duplicatesGroup : duplicatesList) {
    for (const auto &file : duplicatesGroup) {
      std::cout << file << std::endl;
    }
    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
} catch (const std::exception &e) {
  std::cerr << "Uncaught exception: " << e.what();
  return EXIT_FAILURE;
}