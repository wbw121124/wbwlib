#ifndef WBWLIB_STR_KMP_HPP
#define WBWLIB_STR_KMP_HPP

/**
 * @file kmp.hpp
 * @brief KMP 单模式匹配。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O(n+m)。
 *
 * 用法：
 *   std::string t = "aba", s = "ababa";
 *   auto pi = wbwlib::str::prefix_function(t);     // 失配数组
 *   auto pos = wbwlib::str::kmp_search(t, s);      // 返回所有匹配结束位置(1 基)
 */

#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

/// 前缀函数：pi[i] = 子串 [0..i] 的最长相等真前后缀长度
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

/// KMP：返回模式 pat 在 text 中所有匹配的「结束位置+1」（1 基闭区间端点）
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

/// 最短循环节：len - pi[n-1]（能整除则 n/(n-pi[n-1]) 为循环次数）
inline int min_cycle(const std::string& s) {
  int n = (int)s.size();
  std::vector<int> pi = prefix_function(s);
  return n - pi[n - 1];
}

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_KMP_HPP