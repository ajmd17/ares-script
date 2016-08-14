#ifndef TO_STRING_H
#define TO_STRING_H

#include <string>
#include <sstream>

namespace util {
  template <typename T> 
  static std::string to_string(const T &t) {
    std::ostringstream ss;
    ss << t;
    return ss.str();
  }
}

#endif