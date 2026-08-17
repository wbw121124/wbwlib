#ifndef WBWLIB_DS_BIT_HPP
#define WBWLIB_DS_BIT_HPP

/**
 * @file bit.hpp
 * @brief 树状数组 Fenwick：单点加 + 区间和；RangeBIT 区间加 + 区间和；二维 BIT。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 单次操作 O(log n)（2D 为 O(log^2 n)）。
 * 下标约定：1 基（BIT[1..n]）。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::ds::Fenwick<long long> bit(n);
 *   bit.add(3, 5); bit.range_sum(1, 10);
 * @endcode
 */

#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/**
 * @brief 树状数组（Fenwick）：单点加、前缀和与区间和。
 * @tparam T 元素类型（默认 i64），需支持 += 与 -=。
 */
template<class T = i64>
class Fenwick {
  int n_;
  std::vector<T> tr_;

 public:
  /**
   * @brief 默认构造空树（n=0），使用前需调用 init。
   */
  Fenwick() : n_(0) {}

  /**
   * @brief 构造大小为 n 的树状数组（下标 1..n，元素初始为 0）。
   * @param n 元素个数
   */
  explicit Fenwick(int n) : n_(n), tr_(n + 1, T()) {}

  /**
   * @brief 重置为大小为 n 的树状数组并清空所有元素。
   * @param n 元素个数
   */
  void init(int n) { n_ = n; tr_.assign(n + 1, T()); }

  /**
   * @brief 单点加：\f$a[i] += v\f$。
   * @param i 下标（1 基）
   * @param v 增加量
   */
  void add(int i, const T& v) {
    for (; i <= n_; i += i & -i) tr_[i] += v;
  }

  /**
   * @brief 前缀和 \f$sum(a[1..i])\f$。
   * @param i 下标（1 基）
   * @return \f$\sum_{j=1}^{i} a[j]\f$
   */
  T sum(int i) const {
    T s = T();
    for (; i > 0; i -= i & -i) s += tr_[i];
    return s;
  }

  /**
   * @brief 区间和 \f$sum(a[l..r])\f$。
   * @param l 区间左端点（1 基）
   * @param r 区间右端点（1 基）
   * @return \f$\sum_{j=l}^{r} a[j]\f$
   */
  T range_sum(int l, int r) const { return sum(r) - sum(l - 1); }

  /**
   * @brief 由 1 基数组 a 构造（a[0] 占位，长度 n+1）。
   * @param a 1 基数组，仅使用 a[1..n]
   */
  void build(const std::vector<T>& a) {
    for (int i = 1; i <= n_; ++i) {
      tr_[i] += a[i];
      int j = i + (i & -i);
      if (j <= n_) tr_[j] += tr_[i];
    }
  }

  /**
   * @brief 求最小的 i 使前缀和 \f$sum(a[1..i]) \ge k\f$（要求元素非负）。
   * @param k 阈值
   * @return 满足条件的下标（1 基）；不存在时返回 n+1
   */
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
 * @brief 区间加 + 区间和的差分树状数组（双 BIT）。
 *
 * add(l, r, v)：\f$a[l..r]\f$ 整体加 v；range_sum(l, r)：查询区间和。
 * @tparam T 元素类型（默认 i64），需支持 +=、-= 与乘法。
 */
template<class T = i64>
class RangeBIT {
  int n_;
  Fenwick<T> b1_, b2_;

 public:
  /**
   * @brief 构造大小为 n 的差分树状数组（元素初始为 0）。
   * @param n 元素个数
   */
  explicit RangeBIT(int n) : n_(n), b1_(n), b2_(n) {}

  /**
   * @brief 区间加：\f$a[l..r] += v\f$。
   * @param l 区间左端点（1 基）
   * @param r 区间右端点（1 基）
   * @param v 增加量
   */
  void add(int l, int r, const T& v) {
    b1_.add(l, v); b1_.add(r + 1, -v);
    b2_.add(l, v * T(l - 1)); b2_.add(r + 1, -v * T(r));
  }

  /**
   * @brief 前缀和 \f$sum(a[1..x])\f$。
   * @param x 下标（1 基）
   * @return \f$\sum_{j=1}^{x} a[j]\f$
   */
  T sum(int x) const {
    return b1_.sum(x) * T(x) - b2_.sum(x);
  }

  /**
   * @brief 区间和 \f$sum(a[l..r])\f$。
   * @param l 区间左端点（1 基）
   * @param r 区间右端点（1 基）
   * @return \f$\sum_{j=l}^{r} a[j]\f$
   */
  T range_sum(int l, int r) const { return sum(r) - sum(l - 1); }
};

/**
 * @brief 二维树状数组（单点加、子矩阵和），坐标 1 基。
 *
 * 内存 \f$O(n \cdot m)\f$，注意开销。
 * @tparam T 元素类型（默认 i64）
 */
template<class T = i64>
class Fenwick2D {
  int n_, m_;
  std::vector<std::vector<T>> tr_;

 public:
  /**
   * @brief 构造 n 行 m 列的二维树状数组（坐标 1 基，元素初始为 0）。
   * @param n 行数
   * @param m 列数
   */
  Fenwick2D(int n, int m) : n_(n), m_(m), tr_(n + 1, std::vector<T>(m + 1, T())) {}

  /**
   * @brief 单点加：\f$a[x][y] += v\f$。
   * @param x 行号（1 基）
   * @param y 列号（1 基）
   * @param v 增加量
   */
  void add(int x, int y, const T& v) {
    for (int i = x; i <= n_; i += i & -i)
      for (int j = y; j <= m_; j += j & -j)
        tr_[i][j] += v;
  }

  /**
   * @brief 前缀和 \f$sum(a[1..x][1..y])\f$。
   * @param x 行号（1 基）
   * @param y 列号（1 基）
   * @return \f$\sum_{i=1}^{x}\sum_{j=1}^{y} a[i][j]\f$
   */
  T sum(int x, int y) const {
    T s = T();
    for (int i = x; i > 0; i -= i & -i)
      for (int j = y; j > 0; j -= j & -j)
        s += tr_[i][j];
    return s;
  }

  /**
   * @brief 子矩阵和（容斥）：\f$sum(a[x1..x2][y1..y2])\f$。
   * @param x1 左上角行号（1 基）
   * @param y1 左上角列号（1 基）
   * @param x2 右下角行号（1 基）
   * @param y2 右下角列号（1 基）
   * @return \f$\sum_{i=x1}^{x2}\sum_{j=y1}^{y2} a[i][j]\f$
   */
  T range_sum(int x1, int y1, int x2, int y2) const {
    return sum(x2, y2) - sum(x1 - 1, y2) - sum(x2, y1 - 1) + sum(x1 - 1, y1 - 1);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_BIT_HPP