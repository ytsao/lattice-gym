#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "log.hpp"

namespace Utility {

struct TaskEntry {
  std::string vnnlib_path;
  std::string onnx_path;
  std::string propagation_method;
};

inline std::vector<TaskEntry> read_csv(const std::string &filename) {
  std::vector<TaskEntry> entries;
  std::ifstream file(filename);

  if (!file.is_open()) {
    Logger::log(Logger::Level::ERROR, "Cannot open file: " + filename);
    return entries;
  }

  std::string line;

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string onnx, vnnlib, method;

    std::getline(ss, vnnlib, ',');
    std::getline(ss, onnx, ',');
    std::getline(ss, method, ',');

    entries.push_back({vnnlib, onnx, method});
  }

  return entries;
}

inline void print_tasks(const std::vector<TaskEntry> &tasks) {
  for (const auto &t : tasks) {
    Logger::log(Logger::Level::INFO,
                "\nVNNLIB: " + t.vnnlib_path + "\nONNX: " + t.onnx_path +
                    "\nPropagation method: " + t.propagation_method);
  }
}

} // namespace Utility

#endif
