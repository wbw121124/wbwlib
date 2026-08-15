#ifndef WBWLIB_STR_Z_HPP
#define WBWLIB_STR_Z_HPP

/**
 * @file z.hpp
 * @brief Z 函数（扩展 KMP 基础）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O(n)。
 *
 * z[i] = 最长公共前缀 s[0..] 与 s[i..]。
 * 用法：z_function(s)；字符串匹配可用 z[分界位置] 判断。
 */

#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

inline std::vector<int> z_function(const std::string& s) {
  int n = (int)s.size();
  std::vector<int> z(n, 0);
  int l = 0, r = 0;
  for (int i = 1; i < n; ++i) {
    if (i <= r) z[i] = std::min(r - i + 1, z[i - l]);
    while (i + z[i] < n && s[z[i]] == s[i + z[i]]) ++z[i];
    if (i + z[i] - 1 > r) { l = i; r = i + z[i] - 1; }
  }
  return z;
}

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_Z_HPP