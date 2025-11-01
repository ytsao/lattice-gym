#ifndef LOG_HPP
#define LOG_HPP

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace Logger {

enum Level { INFO, WARN, ERROR, DEBUG, EMPTY };

inline bool ENABLE_COLOR = true;
inline bool ENABLE_LOG = true;

inline std::string timestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto t = system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%H:%M:%S");
  return oss.str();
}

inline std::string color(Level level) {
  if (!ENABLE_COLOR)
    return "";
  switch (level) {
  case Level::INFO:
    return "\033[32m"; // green
  case Level::WARN:
    return "\033[33m"; // yellow
  case Level::ERROR:
    return "\033[31m"; // red
  case Level::DEBUG:
    return "\033[36m"; // cyan
  case Level::EMPTY:
    return "\033[0m"; // origin
  }
  return "";
}

inline void log(Level level, const std::string &msg) {
  if (!ENABLE_LOG)
    return;
  std::string tag;
  switch (level) {
  case Level::INFO:
    tag = "[INFO] ";
    break;
  case Level::WARN:
    tag = "[WARN] ";
    break;
  case Level::ERROR:
    tag = "[ERROR] ";
    break;
  case Level::DEBUG:
    tag = "[DEBUG] ";
    break;
  case Level::EMPTY:
    tag = "";
    break;
  }

  if (level == Level::ERROR) {
    std::cerr << color(level) << "[" << timestamp() << "]" << tag << "\033[0m"
              << msg << std::endl;
  } else if (level == Level::EMPTY) {
    std::cout << color(level) << tag << "\033[0m" << msg << std::endl;
  } else {
    std::cout << color(level) << "[" << timestamp() << "]" << tag << "\033[0m"
              << msg << std::endl;
  }
}

}; // namespace Logger

#endif // LOG_HPP
