#ifndef WBWLIB_STR_KMP_HPP
#define WBWLIB_STR_KMP_HPP

/**
 * @file kmp.hpp
 * @brief KMP 单模式匹配。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(n+m)。
 *
 * @par 示例
 * @code{.cpp}
 *   std::string t = "aba", s = "ababa";
 *   auto pi = wbwlib::str::prefix_function(t);     // 失配数组
 *   auto pos = wbwlib::str::kmp_search(t, s);      // 返回所有匹配结束位置(1 基)
 * @endcode
 */

#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief 前缀函数：\f$\pi[i]\f$ = 子串 [0..i] 的最长相等真前后缀长度，满足 \f$\pi[i] = \max\{k : s[0..k-1] = s[i-k+1..i]\}\f$。
 * @param s 输入字符串
 * @return 与 s 等长的前缀函数数组 pi
 */
inline std::vector<int> prefix_function(const std::string& s) {
  int n = (int)s.size();
  std::vector<int> pi(n, 0);
  for (int i = 1; i < n; ++i) {
    int j = pi[i - 1];
    while (j > 0 && s[i] != s[j]) j = pi[j - 1];
    if (s[i] == s[j]) ++j;
    pi[i] = j;
  }
  return pi;
}

/**
 * @brief KMP 单模式匹配：借助前缀函数线性扫描文本。
 * @param pat 模式串
 * @param text 文本串
 * @return 所有匹配的「结束位置 + 1」（1 基闭区间端点）；pat 为空时返回空数组
 */
inline std::vector<int> kmp_search(const std::string& pat, const std::string& text) {
  int m = (int)pat.size();
  std::vector<int> pos;
  if (m == 0) return pos;
  std::vector<int> pi = prefix_function(pat);
  int j = 0;
  for (int i = 0; i < (int)text.size(); ++i) {
    while (j > 0 && text[i] != pat[j]) j = pi[j - 1];
    if (text[i] == pat[j]) ++j;
    if (j == m) {
      pos.push_back(i + 1);
      j = pi[j - 1];
    }
  }
  return pos;
}

/**
 * @brief 最短循环节：\f$s\f$ 的最小周期长度为 \f$n - \pi[n-1]\f$（能被整除时 \f$\frac{n}{n-\pi[n-1]}\f$ 为循环次数）。
 * @param s 输入字符串
 * @return 最短循环节长度
 */
inline int min_cycle(const std::string& s) {
  int n = (int)s.size();
  std::vector<int> pi = prefix_function(s);
  return n - pi[n - 1];
}

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_KMP_HPP