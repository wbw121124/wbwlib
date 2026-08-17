#ifndef WBWLIB_MATH_MATRIX_HPP
#define WBWLIB_MATH_MATRIX_HPP

/**
 * @file matrix.hpp
 * @brief 矩阵类：加/乘、快速幂、转置、单位阵、向量乘法。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 用法（配合 modint）：
 *   using M = wbwlib::math::modint<1000000007>;
 *   wbwlib::math::Mat<M> A(2, 2);
 *   A(0,1) = M(1); ...
 *   Mat<M> C = A * A.pow(k);
 *
 * @attention 复杂乘法可针对性优化（如 += 合并），OI 场景已足够。
 */

#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace math {

/**
 * @brief 稠密矩阵类：加/减/乘、快速幂、转置、单位阵与向量乘法。
 *
 * @tparam T 元素类型（需支持 +, -, * 与 T() 零元构造，如 modint）
 */
template<class T>
class Mat {
  int r_, c_;
  std::vector<T> a_;

public:
  /**
   * @brief 默认构造：0×0 空矩阵。
   */
  Mat() : r_(0), c_(0) {}

  /**
   * @brief 构造 r 行 c 列矩阵，元素初始化为 init。
   * @param r 行数
   * @param c 列数
   * @param init 初始元素值
   */
  Mat(int r, int c, const T& init = T()) : r_(r), c_(c), a_(r * c, init) {}

  /// 返回行数
  int rows() const { return r_; }

  /// 返回列数
  int cols() const { return c_; }

  /**
   * @brief 可写访问元素 (i, j)（行主序）。
   * @param i 行下标
   * @param j 列下标
   * @return 元素引用
   */
  T& operator()(int i, int j) { return a_[i * c_ + j]; }

  /**
   * @brief 只读访问元素 (i, j)。
   * @param i 行下标
   * @param j 列下标
   * @return 元素常量引用
   */
  const T& operator()(int i, int j) const { return a_[i * c_ + j]; }

  /**
   * @brief 构造 n 阶单位矩阵（要求方阵）。
   * @param n 阶数
   * @return 单位阵 \f$I_n\f$（对角元为 T(1)）
   */
  static Mat identity(int n) {
    Mat m(n, n);
    for (int i = 0; i < n; ++i) m(i, i) = T(1);
    return m;
  }

  /**
   * @brief 原地转置：\f$A \gets A^T\f$。
   */
  void transpose_inplace() {
    Mat t(c_, r_);
    for (int i = 0; i < r_; ++i)
      for (int j = 0; j < c_; ++j)
        t(j, i) = a_[i * c_ + j];
    *this = std::move(t);
  }

  /**
   * @brief 返回转置矩阵（不修改自身）。
   * @return \f$A^T\f$
   */
  Mat transposed() const {
    Mat t(c_, r_);
    for (int i = 0; i < r_; ++i)
      for (int j = 0; j < c_; ++j)
        t(j, i) = a_[i * c_ + j];
    return t;
  }

  /**
   * @brief 矩阵加法（要求同尺寸）。
   * @param o 加数矩阵
   * @return \f$A + B\f$
   */
  Mat operator+(const Mat& o) const {
    WBWLIB_ASSERT(r_ == o.r_ && c_ == o.c_);
    Mat r(r_, c_);
    for (int k = 0; k < r_ * c_; ++k) r.a_[k] = a_[k] + o.a_[k];
    return r;
  }

  /**
   * @brief 矩阵减法（要求同尺寸）。
   * @param o 减数矩阵
   * @return \f$A - B\f$
   */
  Mat operator-(const Mat& o) const {
    WBWLIB_ASSERT(r_ == o.r_ && c_ == o.c_);
    Mat r(r_, c_);
    for (int k = 0; k < r_ * c_; ++k) r.a_[k] = a_[k] - o.a_[k];
    return r;
  }

  /**
   * @brief 矩阵乘法（要求 A 的列数等于 B 的行数）。
   *
   * 公式：\f$(AB)_{ij} = \sum_{k} A_{ik} B_{kj}\f$，复杂度 \f$O(r \cdot c_2 \cdot k)\f$，含稀疏跳过。
   * @param o 乘数矩阵
   * @return \f$A \cdot B\f$
   */
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

  /**
   * @brief 方阵快速幂：\f$A^k\f$（倍增法，\f$O(\log k)\f$ 次矩阵乘）。
   * @param k 指数（k < 0 视为 0，返回单位阵）
   * @return \f$A^k\f$（k=0 时返回单位阵）
   */
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

  /**
   * @brief 矩阵乘列向量：\f$r_i = \sum_k A_{ik} v_k\f$。
   * @param v 列向量（长度须等于列数）
   * @return 长度等于行数的结果向量
   */
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