#ifndef WBWLIB_DS_SQRT_BLOCK_HPP
#define WBWLIB_DS_SQRT_BLOCK_HPP

/**
 * @file sqrt-block.hpp
 * @brief 序列分块：区间加 + 区间和 + 区间最值。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：修改/查询 O(sqrt(n))。
 *
 * 用法：
 *   std::vector<long long> a = {1,2,3};
 *   wbwlib::ds::SqrtDecomp<long long> sb(a);      // a 为 1 基（a[0] 占位）
 *   sb.add(2, 5, 10);          // [2,5] += 10
 *   sb.sum(1, n); sb.max(1, n);
 */

#include <algorithm>
#include <cmath>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T = i64>
class SqrtDecomp {
  int n_;
  int B_;                      ///< 块大小
  std::vector<T> a_;           ///< 原值（含块懒标记）
  std::vector<T> sum_, mx_, lazy_;
  std::vector<int> bel_;

  void rebuild(int b) {
    int l = b * B_ + 1, r = (std::min)(n_, (b + 1) * B_);
    sum_[b] = T(); mx_[b] = a_[l] + lazy_[b];   // 初始占位
    T s = 0, m = a_[l];
    for (int i = l; i <= r; ++i) {
      T v = a_[i] + lazy_[b];
      s += v;
      m = (std::max)(m, v);
    }
    sum_[b] = s;
    mx_[b] = m;
  }

 public:
  explicit SqrtDecomp(const std::vector<T>& a) : n_((int)a.size() - 1) {
    B_ = (int)std::max(1, (int)std::sqrt((double)n_));
    int nb = (n_ + B_ - 1) / B_;
    a_ = a;
    sum_.assign(nb + 1, T());
    mx_.assign(nb + 1, T());
    lazy_.assign(nb + 1, T());
    bel_.assign(n_ + 2, 0);
    for (int i = 1; i <= n_; ++i) bel_[i] = (i - 1) / B_;
    for (int b = 0; b < nb; ++b) rebuild(b);
  }

  /// 区间加 [l, r]
  void add(int l, int r, const T& v) {
    int bl = bel_[l], br = bel_[r];
    if (bl == br) {
      for (int i = l; i <= r; ++i) a_[i] += v;
      rebuild(bl);
      return;
    }
    for (int i = l; i <= (std::min)(n_, (bl + 1) * B_); ++i) a_[i] += v;
    rebuild(bl);
    for (int i = (bl + 1); i < br; ++i) {
      lazy_[i] += v;
      sum_[i] += v * T(B_);
      mx_[i] += v;
    }
    for (int i = br * B_ + 1; i <= r; ++i) a_[i] += v;
    rebuild(br);
  }

  /// 区间和
  T sum(int l, int r) {
    int bl = bel_[l], br = bel_[r];
    if (bl == br) {
      T s = T();
      for (int i = l; i <= r; ++i) s += a_[i] + lazy_[bl];
      return s;
    }
    T res = T();
    for (int i = l; i <= (std::min)(n_, (bl + 1) * B_); ++i) res += a_[i] + lazy_[bl];
    for (int i = bl + 1; i < br; ++i) res += sum_[i];
    for (int i = br * B_ + 1; i <= r; ++i) res += a_[i] + lazy_[br];
    return res;
  }

  /// 区间最大值
  T max(int l, int r) {
    int bl = bel_[l], br = bel_[r];
    if (bl == br) {
      T m = a_[l] + lazy_[bl];
      for (int i = l; i <= r; ++i) m = (std::max)(m, a_[i] + lazy_[bl]);
      return m;
    }
    T res = a_[l] + lazy_[bl];
    for (int i = l; i <= (std::min)(n_, (bl + 1) * B_); ++i)
      res = (std::max)(res, a_[i] + lazy_[bl]);
    for (int i = bl + 1; i < br; ++i) res = (std::max)(res, mx_[i]);
    for (int i = br * B_ + 1; i <= r; ++i)
      res = (std::max)(res, a_[i] + lazy_[br]);
    return res;
  }

  int block_size() const { return B_; }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_SQRT_BLOCK_HPP