#ifndef WBWLIB_STR_MINIMAL_STRING_HPP
#define WBWLIB_STR_MINIMAL_STRING_HPP

/**
 * @file minimal-string.hpp
 * @brief 最小表示法：求循环同构串中字典序最小的起始位置。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O(n)。
 *
 * 用法：
 *   int p = wbwlib::str::minimal_rotation(s);        // 返回 0 基起始下标
 *   std::string t = s.substr(p) + s.substr(0, p);
 */

#include <string>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

inline int minimal_rotation(const std::string& s) {
  int n = (int)s.size();
  if (n == 0) return -1;
  int i = 0, j = 1, k = 0;
  while (i < n && j < n && k < n) {
    char a = s[(i + k) % n], b = s[(j + k) % n];
    if (a == b) { ++k; continue; }
    if (a > b) { i += k + 1; }
    else { j += k + 1; }
    if (i == j) ++j;
    k = 0;
  }
  return i < j ? i : j;
}

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_MINIMAL_STRING_HPP