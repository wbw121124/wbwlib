#ifndef WBWLIB_DP_STATE_COMPRESS_HPP
#define WBWLIB_DP_STATE_COMPRESS_HPP

/**
 * @file state-compress.hpp
 * @brief 状态压缩 DP 工具：子集枚举、popcount/bit 工具、TSP 模板。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 子集枚举 O(3^n)（全体子集总和）；TSP 模板 O(2^n·n^2)。
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace dp {

/**
 * @brief 统计二进制中 1 的个数。
 * @param x 输入无符号整数。
 * @return x 的二进制表示中 1 的个数。
 */
inline int popcount(u32 x) { return __builtin_popcount(x); }

/**
 * @brief 求最低位 1 的位置。
 * @param x 输入无符号整数，须非零。
 * @return 最低位 1 的从 0 开始的下标。
 */
inline int lowbit(u32 x) { return __builtin_ctz(x); }   ///< 最低位 1 的位置（0 基）

/**
 * @brief 枚举 mask 的所有非空子集（含 mask 自身、不含空集）并调用 f(sub)。
 * @tparam F 回调类型，f 接受一个 u32 子集掩码。
 * @param mask 全集掩码。
 * @param f 对每个枚举到的子集调用的回调。
 */
template<class F>
void for_each_subset(u32 mask, const F& f) {
  for (u32 sub = mask; sub; sub = (sub - 1) & mask) f(sub);
}

/**
 * @brief 枚举 mask 的所有子集（含空集 0 与 mask 自身）并调用 f(sub)。
 * @tparam F 回调类型，f 接受一个 u32 子集掩码。
 * @param mask 全集掩码。
 * @param f 对每个枚举到的子集调用的回调。
 */
template<class F>
void for_each_subset_with_zero(u32 mask, const F& f) {
  f(0);
  for (u32 sub = mask; sub; sub = (sub - 1) & mask) f(sub);
}

/**
 * @brief 旅行商模板：完全图上从顶点 0 出发并回到 0 的最短哈密顿回路。
 *
 * 状态转移：\f$dp[S][j] = \min_{i \in S, i \neq j}\{dp[S \setminus \{j\}][i] + dist[i][j]\}\f$。
 *
 * @param dist n×n 距离矩阵，dist[i][j] 为 i → j 的距离（i, j ∈ [0, n)）。
 * @return 最短回路总代价；不可达视为 INF。
 * 例：
 *   std::vector<std::vector<i64>> d(n, std::vector<i64>(n));
 *   i64 ans = wbwlib::dp::tsp(d);
 */
inline i64 tsp(const std::vector<std::vector<i64>>& dist) {
  int n = (int)dist.size();
  if (n == 0) return 0;
  unsigned full = 1u << n;
  const i64 INF = (i64)4e18;
  std::vector<std::vector<i64>> f(full, std::vector<i64>(n, INF));
  f[1][0] = 0;
  for (unsigned mask = 1; mask < full; ++mask) {
    for (int j = 0; j < n; ++j) {
      if (!(mask & (1u << j))) continue;
      i64 d = f[mask][j];
      if (d >= INF) continue;
      for (int k = 0; k < n; ++k) {
        if (mask & (1u << k)) continue;
        f[mask | (1u << k)][k] =
            std::min(f[mask | (1u << k)][k], d + dist[j][k]);
      }
    }
  }
  i64 ans = INF;
  for (int j = 1; j < n; ++j)
    ans = std::min(ans, f[full - 1][j] + dist[j][0]);
  return ans;
}

} // namespace dp
} // namespace wbwlib

#endif // WBWLIB_DP_STATE_COMPRESS_HPP