#ifndef WBWLIB_STR_MANACHER_HPP
#define WBWLIB_STR_MANACHER_HPP

/**
 * @file manacher.hpp
 * @brief Manacher 回文算法。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O(n)。
 *
 * 返回：
 *   d1[i] —— 以 i 为中心（0 基）的奇回文半径（含中心），回文长 = 2*d1[i]-1；
 *   d2[i] —— 以 i 与 i+1 之间为中心的偶回文半径，回文长 = 2*d2[i]。
 *
 * 用法：
 *   auto [d1, d2] = wbwlib::str::manacher(s);
 */

#include <string>
#include <vector>
#include <utility>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

/// 返回 (d1, d2)
inline std::pair<std::vector<int>, std::vector<int>> manacher(const std::string& s) {
  int n = (int)s.size();
  std::vector<int> d1(n), d2(n);
  for (int i = 0, l = 0, r = -1; i < n; ++i) {
    int k = (i > r) ? 1 : std::min(d1[l + r - i], r - i + 1);
    while (0 <= i - k && i + k < n && s[i - k] == s[i + k]) ++k;
    d1[i] = k--;
    if (i + k > r) { l = i - k; r = i + k; }
  }
  for (int i = 0, l = 0, r = -1; i < n; ++i) {
    int k = (i > r) ? 0 : std::min(d2[l + r - i + 1], r - i + 1);
    while (0 <= i - k - 1 && i + k < n && s[i - k - 1] == s[i + k]) ++k;
    d2[i] = k--;
    if (i + k > r) { l = i - k - 1; r = i + k; }
  }
  return {d1, d2};
}

/// 本质不同回文子串个数（另一种做法见回文自动机）
inline i64 count_distinct_palindromes(const std::string& s) {
  std::pair<std::vector<int>, std::vector<int>> d = manacher(s);
  const std::vector<int>& d1 = d.first;
  const std::vector<int>& d2 = d.second;
  i64 cnt = 0;
  for (int x : d1) cnt += x;
  for (int x : d2) cnt += x;
  return cnt;
}

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_MANACHER_HPP