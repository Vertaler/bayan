#include <iostream>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/program_options.hpp>

#include "FileCollector.h"
#include "FileDuplicatesSearher.h"

namespace options = boost::program_options;

int main(int argc, char **argv) try {
  options::options_description desc("Tool for searching file duplicates");

  bayan::FileQuery fileQuery{};
  bayan::FileDuplicatesSearherOptions duplicateSearherOptions{};
  bayan::HashType hashType{};

  std::string logFile;
  using LogSeverity = boost::log::trivial::severity_level;
  LogSeverity logSeverityLevel{};

  // clang-format off
  desc.add_options()
    ("help,h", "Show help")
    ("dirs", options::value<std::vector<boost::filesystem::path>>(&fileQuery.dirs)->multitoken()->required(), "Directories for scan")
    ("exclude_dirs", options::value<std::vector<boost::filesystem::path>>()->multitoken(), "Directories to exclude from scan")
    ("min_file_size", options::value<uintmax_t>(&fileQuery.minFileSize)->default_value(1), "Minimal size of file to check for duplicates (in bytes)")
    ("filename_pattern", options::value<std::string>(), "Name pattern for files to check for duplicates (case insensitive)")
    ("recursion_depth", options::value<int>(), "Maximal depth of nested directories to scan (0 means current directly only). By default unlimited")
    ("hash", options::value<bayan::HashType>(&hashType)->default_value(bayan::HashType::CRC32), "Hashing algoritm used for file block comparison. Allowed values: crc16,crc32 and crc64")
    ("block_size", options::value<size_t>(&duplicateSearherOptions.blockSize)->default_value(4096), "Size of file block read from disk at once (in bytes)")
    ("log_file", options::value(&logFile)->default_value("bayan.log"), "File to log some auxiliary information during utility execution")
    ("log_severity", options::value(&logSeverityLevel)->default_value(LogSeverity::info), "Boost log severity level");
  // clang-format on

  bayan::FileCollector collector;
  options::variables_map optionsMap;
  options::parsed_options parsed =
      options::command_line_parser(argc, argv).options(desc).run();
  options::store(parsed, optionsMap);

  if (optionsMap.count("help") != 0) {
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

  const auto logFormat = "[%TimeStamp%][%Severity%]: %Message%";
  options::notify(optionsMap);
  boost::log::add_file_log(boost::log::keywords::file_name = logFile,
                           boost::log::keywords::auto_flush = true,
                           boost::log::keywords::format = logFormat);
  boost::log::add_common_attributes();
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      logSeverityLevel);

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