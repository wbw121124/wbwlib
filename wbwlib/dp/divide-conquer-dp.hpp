#ifndef WBWLIB_DP_DIVIDE_DP_HPP
#define WBWLIB_DP_DIVIDE_DP_HPP

/**
 * @file divide-conquer-dp.hpp
 * @brief 决策单调性优化（分治）/ 四边形不等式简化版分治 DP。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(K * n log n * (cost 一次的开销))。
 *
 * 适用：dp[j][i] = min_{p < i} dp[j-1][p] + cost(p+1, i)，且最优决策单调
 *       （转移点随 i 单调不减）。
 *
 * @par 示例
 * @code{.cpp}
 *   auto cost = [&](int l, int r) -> i64 { return sum(r)-sum(l-1); };   // O(1)
 *   std::vector<i64> ans = wbwlib::dp::divide_conquer_dp( n, K, cost );
 *   // ans[i] = dp[K][i]
 * @endcode
 */

#include <vector>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace dp {

namespace dcp_detail {
/// 递归求解区间 [l, r] 的 dp 值，转移点搜索范围限制在 [optl, optr]
template<class Cost>
void solve(int l, int r, int optl, int optr,
           const std::vector<i64>& prev, std::vector<i64>& cur,
           const Cost& cost) {
  if (l > r) return;
  int mid = (l + r) >> 1;
  i64 best = (i64)4e18;
  int bestk = optl;
  int hi = std::min(optr, mid - 1);
  for (int k = optl; k <= hi; ++k) {
    i64 v = prev[k] + cost(k + 1, mid);
    if (v < best) { best = v; bestk = k; }
  }
  cur[mid] = best;
  solve(l, mid - 1, optl, bestk, prev, cur, cost);
  solve(mid + 1, r, bestk, optr, prev, cur, cost);
}
} // namespace dcp_detail

/**
 * @brief 决策单调性分治 DP：\f$dp_j[i] = \min_{p < i}\{dp_{j-1}[p] + cost(p+1, i)\}\f$。
 * @tparam Cost 代价函数类型，cost(l, r) 返回区间 [l, r] 的代价。
 * @param n 状态数（i 取值范围 [1, n]）。
 * @param K DP 层数。
 * @param cost 代价函数，需满足四边形不等式以保证最优决策点随 i 单调不减。
 * @return 长度为 n+1 的数组，ans[i] = dp[K][i]（i >= 1）。
 */
template<class Cost>
std::vector<i64> divide_conquer_dp(int n, int K, const Cost& cost) {
  std::vector<i64> prev(n + 1, (i64)4e18), cur(n + 1, (i64)4e18);
  prev[0] = 0;
  for (int j = 1; j <= K; ++j) {
    std::fill(cur.begin(), cur.end(), (i64)4e18);
    dcp_detail::solve(1, n, 0, n - 1, prev, cur, cost);
    prev.swap(cur);
  }
  return prev;
}

} // namespace dp
} // namespace wbwlib

#endif // WBWLIB_DP_DIVIDE_DP_HPP