#ifndef WBWLIB_DS_BIT_HPP
#define WBWLIB_DS_BIT_HPP

/**
 * @file bit.hpp
 * @brief 树状数组 Fenwick：单点加 + 区间和；RangeBIT 区间加 + 区间和；二维 BIT。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：单次操作 O(log n)（2D 为 O(log^2 n)）。
 * 下标约定：1 基（BIT[1..n]）。
 *
 * 用法：
 *   wbwlib::ds::Fenwick<long long> bit(n);
 *   bit.add(3, 5); bit.range_sum(1, 10);
 */

#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T = i64>
class Fenwick {
  int n_;
  std::vector<T> tr_;

 public:
  Fenwick() : n_(0) {}
  explicit Fenwick(int n) : n_(n), tr_(n + 1, T()) {}

  void init(int n) { n_ = n; tr_.assign(n + 1, T()); }

  /// 单点加：a[i] += v
  void add(int i, const T& v) {
    for (; i <= n_; i += i & -i) tr_[i] += v;
  }

  /// 前缀和 sum(a[1..i])
  T sum(int i) const {
    T s = T();
    for (; i > 0; i -= i & -i) s += tr_[i];
    return s;
  }

  /// 区间和 sum(a[l..r])
  T range_sum(int l, int r) const { return sum(r) - sum(l - 1); }

  /// 由数组构造（a 为 1 基，长度 n+1）
  void build(const std::vector<T>& a) {
    for (int i = 1; i <= n_; ++i) {
      tr_[i] += a[i];
      int j = i + (i & -i);
      if (j <= n_) tr_[j] += tr_[i];
    }
  }

  /// 第一个前缀和 >= k 的下标（要求元素非负）；不存在返回 n+1
  int lower_bound(const T& k) const {
    T cur = T(); int pos = 0, step = 1;
    while (step << 1 <= n_) step <<= 1;
    for (int i = step; i > 0; i >>= 1) {
      int np = pos + i;
      if (np <= n_ && cur + tr_[np] < k) {
        cur += tr_[np];
        pos = np;
      }
    }
    return pos + 1;
  }
};

/**
 * 区间加 + 区间和的差分树状数组（双 BIT）。
 * add_range(l, r, v)：a[i] += v, l<=i<=r；range_sum(l, r)。
 */
template<class T = i64>
class RangeBIT {
  int n_;
  Fenwick<T> b1_, b2_;

 public:
  explicit RangeBIT(int n) : n_(n), b1_(n), b2_(n) {}

  /// a[l..r] 整体加 v
  void add(int l, int r, const T& v) {
    b1_.add(l, v); b1_.add(r + 1, -v);
    b2_.add(l, v * T(l - 1)); b2_.add(r + 1, -v * T(r));
  }

  /// 前缀和 sum(a[1..x])
  T sum(int x) const {
    return b1_.sum(x) * T(x) - b2_.sum(x);
  }

  T range_sum(int l, int r) const { return sum(r) - sum(l - 1); }
};

/**
 * 二维树状数组（单点加、子矩阵和）。
 * 坐标 1 基。内存 O(n*m)，注意开销。
 */
template<class T = i64>
class Fenwick2D {
  int n_, m_;
  std::vector<std::vector<T>> tr_;

 public:
  Fenwick2D(int n, int m) : n_(n), m_(m), tr_(n + 1, std::vector<T>(m + 1, T())) {}

  void add(int x, int y, const T& v) {
    for (int i = x; i <= n_; i += i & -i)
      for (int j = y; j <= m_; j += j & -j)
        tr_[i][j] += v;
  }

  /// 前缀和 sum(a[1..x][1..y])
  T sum(int x, int y) const {
    T s = T();
    for (int i = x; i > 0; i -= i & -i)
      for (int j = y; j > 0; j -= j & -j)
        s += tr_[i][j];
    return s;
  }

  T range_sum(int x1, int y1, int x2, int y2) const {
    return sum(x2, y2) - sum(x1 - 1, y2) - sum(x2, y1 - 1) + sum(x1 - 1, y1 - 1);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_BIT_HPP