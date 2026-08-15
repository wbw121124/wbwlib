#ifndef WBWLIB_DS_SPARSE_TABLE_HPP
#define WBWLIB_DS_SPARSE_TABLE_HPP

/**
 * @file sparse-table.hpp
 * @brief Sparse Table ST 表：静态区间幂等查询（取 min/max/gcd/…），O(1) 查询。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：预处理 O(n log n)，查询 O(1)。
 * 限制：仅适用于「幂等半群」（联合两次不影响结果），即要求 f(f(a,b),b)=f(a,b)，
 *      区间最值、gcd、位与、位或均适用；区间和不可用（请用线段树/前缀和）。
 *
 * 用法：
 *   wbwlib::ds::SparseTable<i64> st(a, [](i64 x, i64 y){ return std::min(x, y); });
 *   i64 ans = st.query(l, r);    // 1 基
 */

#include <algorithm>
#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T, class Op>
class SparseTable {
  std::vector<std::vector<T>> f_;  ///< f_[k][i] = 区间 [i, i+2^k-1]
  Op op_;

 public:
  SparseTable() {}
  /**
   * @param a 1 基数组（size = n+1）
   * @param op 幂等半群运算
   */
  SparseTable(const std::vector<T>& a, Op op = Op()) : op_(op) {
    int n = (int)a.size() - 1;
    int k = 0;
    while ((1 << k) <= n) ++k;
    f_.assign(k, std::vector<T>(n + 1));
    for (int i = 1; i <= n; ++i) f_[0][i] = a[i];
    for (int j = 1; j < k; ++j)
      for (int i = 1; i + (1 << j) - 1 <= n; ++i)
        f_[j][i] = op_(f_[j - 1][i], f_[j - 1][i + (1 << (j - 1))]);
  }

  /// 查询 [l, r]（1 基闭区间）的 op 值
  T query(int l, int r) const {
    int len = r - l + 1;
    int j = 0;
    while ((1 << (j + 1)) <= len) ++j;
    return op_(f_[j][l], f_[j][r - (1 << j) + 1]);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_SPARSE_TABLE_HPP