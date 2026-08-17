#ifndef WBWLIB_STR_MANACHER_HPP
#define WBWLIB_STR_MANACHER_HPP

/**
 * @file manacher.hpp
 * @brief Manacher 回文算法。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(n)。
 *
 * 返回：
 *   d1[i] —— 以 i 为中心（0 基）的奇回文半径（含中心），回文长 = 2*d1[i]-1；
 *   d2[i] —— 以 i 与 i+1 之间为中心的偶回文半径，回文长 = 2*d2[i]。
 *
 * @par 示例
 * @code{.cpp}
 *   auto [d1, d2] = wbwlib::str::manacher(s);
 * @endcode
 */

#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/string/palindromic-pam.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief Manacher 算法，线性求全部回文半径。
 * @param s 输入字符串
 * @return pair(d1, d2)：d1[i] 为以 i（0 基）为中心的奇回文半径（含中心，回文长 \f$2\cdot d1[i]-1\f$）；d2[i] 为以 i 与 i+1 之间为中心的偶回文半径（回文长 \f$2\cdot d2[i]\f$）
 */
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

/**
 * @brief 本质不同回文子串个数（经回文自动机统计；manacher 仅能数含重复的总数）。
 * @param s 输入字符串
 * @return 本质不同回文子串数量
 */
inline i64 count_distinct_palindromes(const std::string& s) {
  PAM<> pam;
  pam.build(s);
  return pam.distinct();
}

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_MANACHER_HPP
