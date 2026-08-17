#ifndef WBWLIB_DP_DIGIT_DP_HPP
#define WBWLIB_DP_DIGIT_DP_HPP

/**
 * @file digit-dp.hpp
 * @brief 数位 DP：经典统计模板（上限 ≤ N）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 说明：
 *   提供三个可直接用的经典问题：
 *     1) sum_of_digits_leq(N)  —— [0,N] 各位数字之和的总和。
 *     2) count_mod_leq(N, M)   —— 数位和能被 M 整除的数的个数。
 *     3) count_no_62_leq(N)    —— 不含连续 "62" 的数的个数。
 *   更一般场景请参照实现自行扩展状态（pos, tight, 其他约束）。
 */

#include <string>
#include <vector>
#include <cstring>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace dp {

/**
 * @brief 统计 \f$\sum_{x=0}^{N}\f$ 所有数 x 的十进制数位之和的总和。
 * @param N 上界，以十进制字符串传入（无前导零）。
 * @return [0, N] 内所有数的十进制数位之和的总和。
 */
inline i64 sum_of_digits_leq(const std::string& N) {
  int n = (int)N.size();
  std::vector<std::vector<i64>> memo(n + 1, std::vector<i64>(9 * n + 1, -1));
  std::function<i64(int, int, bool)> dfs =
      [&](int pos, int sum, bool tight) -> i64 {
    if (pos == n) return sum;
    if (!tight && memo[pos][sum] >= 0) return memo[pos][sum];
    i64 res = 0;
    int up = tight ? N[pos] - '0' : 9;
    for (int d = 0; d <= up; ++d)
      res += dfs(pos + 1, sum + d, tight && d == up);
    if (!tight) memo[pos][sum] = res;
    return res;
  };
  return dfs(0, 0, true);
}

/**
 * @brief 统计 [0, N] 中数位总和能被 M 整除的数的个数。
 * @param N 上界，以十进制字符串传入。
 * @param M 模数，要求 M >= 1。
 * @return 满足 \f$digit\_sum(x) \equiv 0 \pmod M\f$ 的数 x 的个数。
 */
inline i64 count_mod_leq(const std::string& N, int M) {
  int n = (int)N.size();
  std::vector<std::vector<i64>> memo(n + 1, std::vector<i64>(M, -1));
  std::function<i64(int, int, bool)> dfs =
      [&](int pos, int rem, bool tight) -> i64 {
    if (pos == n) return rem == 0 ? 1 : 0;
    if (!tight && memo[pos][rem] >= 0) return memo[pos][rem];
    i64 res = 0;
    int up = tight ? N[pos] - '0' : 9;
    for (int d = 0; d <= up; ++d)
      res += dfs(pos + 1, (rem + d) % M, tight && d == up);
    if (!tight) memo[pos][rem] = res;
    return res;
  };
  return dfs(0, 0, true);
}

/**
 * @brief 统计 [0, N] 中十进制表示不含连续两位 "62" 的数的个数。
 * @param N 上界，以十进制字符串传入。
 * @return 满足条件的数的个数。
 */
inline i64 count_no_62_leq(const std::string& N) {
  int n = (int)N.size();
  std::vector<std::vector<i64>> memo(n + 1, std::vector<i64>(11, -1));
  // memo[pos][prev]：prev=10 表示没有前一位
  std::function<i64(int, int, bool)> dfs =
      [&](int pos, int prev, bool tight) -> i64 {
    if (pos == n) return 1;
    if (!tight && memo[pos][prev] >= 0) return memo[pos][prev];
    i64 res = 0;
    int up = tight ? N[pos] - '0' : 9;
    for (int d = 0; d <= up; ++d) {
      if (prev == 6 && d == 2) continue;   // 禁止 62
      res += dfs(pos + 1, d, tight && d == up);
    }
    if (!tight) memo[pos][prev] = res;
    return res;
  };
  return dfs(0, 10, true);
}

} // namespace dp
} // namespace wbwlib

#endif // WBWLIB_DP_DIGIT_DP_HPP