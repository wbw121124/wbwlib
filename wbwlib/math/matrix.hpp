#ifndef WBWLIB_MATH_MATRIX_HPP
#define WBWLIB_MATH_MATRIX_HPP

/**
 * @file matrix.hpp
 * @brief 矩阵类：加/乘、快速幂、转置、单位阵、向量乘法。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 用法（配合 modint）：
 *   using M = wbwlib::math::modint<1000000007>;
 *   wbwlib::math::Mat<M> A(2, 2);
 *   A(0,1) = M(1); ...
 *   Mat<M> C = A * A.pow(k);
 *
 * 注意：复杂乘法可针对性优化（如 += 合并），OI 场景已足够。
 */

#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace math {

template<class T>
class Mat {
  int r_, c_;
  std::vector<T> a_;

public:
  Mat() : r_(0), c_(0) {}
  Mat(int r, int c, const T& init = T()) : r_(r), c_(c), a_(r * c, init) {}

  int rows() const { return r_; }
  int cols() const { return c_; }

  T& operator()(int i, int j) { return a_[i * c_ + j]; }
  const T& operator()(int i, int j) const { return a_[i * c_ + j]; }

  /// 单位矩阵（要求方阵）
  static Mat identity(int n) {
    Mat m(n, n);
    for (int i = 0; i < n; ++i) m(i, i) = T(1);
    return m;
  }

  void transpose_inplace() {
    Mat t(c_, r_);
    for (int i = 0; i < r_; ++i)
      for (int j = 0; j < c_; ++j)
        t(j, i) = a_[i * c_ + j];
    *this = std::move(t);
  }
  Mat transposed() const {
    Mat t(c_, r_);
    for (int i = 0; i < r_; ++i)
      for (int j = 0; j < c_; ++j)
        t(j, i) = a_[i * c_ + j];
    return t;
  }

  Mat operator+(const Mat& o) const {
    WBWLIB_ASSERT(r_ == o.r_ && c_ == o.c_);
    Mat r(r_, c_);
    for (int k = 0; k < r_ * c_; ++k) r.a_[k] = a_[k] + o.a_[k];
    return r;
  }

  Mat operator-(const Mat& o) const {
    WBWLIB_ASSERT(r_ == o.r_ && c_ == o.c_);
    Mat r(r_, c_);
    for (int k = 0; k < r_ * c_; ++k) r.a_[k] = a_[k] - o.a_[k];
    return r;
  }

  /// 矩阵乘法 O(r*c2*c)
  Mat operator*(const Mat& o) const {
    WBWLIB_ASSERT(c_ == o.r_);
    int rn = r_, cn = o.c_, kk = c_;
    Mat r(rn, cn);
    for (int i = 0; i < rn; ++i) {
      for (int k = 0; k < kk; ++k) {
        T aik = (*this)(i, k);
        if (aik == T()) continue;   // 稀疏跳过
        for (int j = 0; j < cn; ++j)
          r(i, j) += aik * o(k, j);
      }
    }
    return r;
  }

  /// 方阵快速幂（k<0 视为 0）
  Mat pow(i64 k) const {
    WBWLIB_ASSERT(r_ == c_);
    Mat res = Mat::identity(r_), base = *this;
    while (k > 0) {
      if (k & 1) res = res * base;
      base = base * base;
      k >>= 1;
    }
    return res;
  }

  /// A * v（列向量）
  std::vector<T> operator*(const std::vector<T>& v) const {
    WBWLIB_ASSERT(c_ == (int)v.size());
    std::vector<T> r(r_, T());
    for (int i = 0; i < r_; ++i)
      for (int k = 0; k < c_; ++k)
        r[i] += (*this)(i, k) * v[k];
    return r;
  }
};

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_MATRIX_HPP