#ifndef WBWLIB_MATH_LINALG_HPP
#define WBWLIB_MATH_LINALG_HPP

/**
 * @file linalg.hpp
 * @brief 线性代数：高斯消元（复杂类型通用，支持浮点/模）、异或线性基。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 高斯消元返回秩以及每列是否自由；浮点时做了行交换（部分主元法）增强数值稳定。
 * 元素类型要求支持 0、1、+、-、*、/、==。
 */

#include <cmath>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace math {

// ================= 高斯消元 =================
/**
 * 对增广矩阵 a（n 行 × (m+1) 列，最后一列为常数列）进行高斯消元。
 * 返回秩 rank。
 *  - 算术类型（int/double 等）：做绝对值部分主元消元，稳定性好；
 *  - 自定义类型（如 modint）：做精确首非零主元消元。
 * fp_eps：浮点近零判定阈值（精确类型使用时忽略）。
 */
namespace details {
template<class T>
inline enable_if_t<std::is_arithmetic<T>::value, int>
gauss_impl(std::vector<std::vector<T>>& a, double fp_eps) {
  int n = (int)a.size(), m = (int)a[0].size();
  int rank = 0;
  for (int col = 0; col < m - 1 && rank < n; ++col) {
    int piv = rank;
    double best = (fp_eps > 0) ? fp_eps : 1e-9;
    for (int i = rank; i < n; ++i) {
      double cur = std::fabs(static_cast<double>(a[i][col]));
      if (cur > best) { best = cur; piv = i; }
    }
    if (std::fabs(static_cast<double>(a[piv][col])) <= best) continue;  // 整列为 0 → 自由列
    std::swap(a[piv], a[rank]);
    T div = a[rank][col];
    for (int j = col; j < m; ++j) a[rank][j] = a[rank][j] / div;
    for (int i = 0; i < n; ++i) {
      if (i == rank) continue;
      T coef = a[i][col];
      if (coef == T()) continue;
      for (int j = col; j < m; ++j) a[i][j] = a[i][j] - coef * a[rank][j];
    }
    ++rank;
  }
  return rank;
}

template<class T>
inline enable_if_t<!std::is_arithmetic<T>::value, int>
gauss_impl(std::vector<std::vector<T>>& a, double) {
  int n = (int)a.size(), m = (int)a[0].size();
  int rank = 0;
  for (int col = 0; col < m - 1 && rank < n; ++col) {
    int piv = -1;
    for (int i = rank; i < n; ++i)
      if (a[i][col] != T()) { piv = i; break; }
    if (piv == -1) continue;               // 自由列
    if (piv != rank) std::swap(a[piv], a[rank]);
    T div = a[rank][col];
    for (int j = col; j < m; ++j) a[rank][j] = a[rank][j] / div;
    for (int i = 0; i < n; ++i) {
      if (i == rank) continue;
      T coef = a[i][col];
      if (coef == T()) continue;
      for (int j = col; j < m; ++j) a[i][j] = a[i][j] - coef * a[rank][j];
    }
    ++rank;
  }
  return rank;
}
} // namespace details

template<class T>
int gauss_elim(std::vector<std::vector<T>>& a, double fp_eps = 1e-9) {
  return details::gauss_impl(a, fp_eps);
}

/**
 * 求解 n 元线性方程组 A x = b。
 * 返回 0 表示唯一解（解写入 x），1 表示无穷解，-1 表示无解。
 * 矩阵必须为方阵（n==m==A.size()）。
 */
template<class T>
int solve_linear(std::vector<std::vector<T>> A, std::vector<T> b, std::vector<T>& x) {
  int n = (int)A.size();
  WBWLIB_ASSERT(n > 0);
  for (int i = 0; i < n; ++i) A[i].push_back(b[i]);  // 增广
  int rank = gauss_elim(A, 0.0);                     // 精确消元（模意义下）
  // 检查无解：存在全零行但常数列非零
  for (int i = 0; i < n; ++i) {
    bool allz = true;
    for (int j = 0; j < n; ++j) if (A[i][j] != T()) { allz = false; break; }
    if (allz && A[i][n] != T()) return -1;
  }
  if (rank < n) return 1;   // 无穷解
  x.assign(n, T());
  for (int i = 0; i < n; ++i) x[i] = A[i][n];
  return 0;
}

/// 行列式（消元后对角积；浮点可用）
template<class T>
T det(std::vector<std::vector<T>> a) {
  int n = (int)a.size();
  T d = T(1);
  for (int col = 0; col < n; ++col) {
    int piv = col;
    while (piv < n && a[piv][col] == T()) ++piv;
    if (piv == n) return T(0);
    if (piv != col) { std::swap(a[piv], a[col]); d = -d; }
    T div = a[col][col];
    d = d * div;
    for (int i = col + 1; i < n; ++i) {
      T coef = a[i][col];
      if (coef == T()) continue;
      for (int j = col; j < n; ++j)
        a[i][j] = a[i][j] - coef / div * a[col][j];
    }
  }
  return d;
}

// ================= 异或线性基 =================
/**
 * 64 位异或线性基：
 *  - insert(x)：插入成功返回 true（增加秩）；
 *  - max_xor()：与已有元素异或得到的最大值；可传入初值 base；
 *  - min_xor(base)：最小值（异或 base）也支持；
 *  - has(x)：x 是否可被表示；
 *  - kth_min(k)：第 k 小（1 基）的可表示数，需先 build_kth()。
 */
class XorBasis {
  static const int B = 63;
  u64 p_[B];            ///< 主元数组
  bool ok_kth_;
  std::vector<u64> kth_;  ///< 用于第 k 小查询的化简基
 public:
  XorBasis() : ok_kth_(false) { reset(); }
  void reset() { for (int i = 0; i < B; ++i) p_[i] = 0; ok_kth_ = false; kth_.clear(); }

  bool insert(u64 x) {
    ok_kth_ = false;
    for (int b = B - 1; b >= 0; --b) {
      if (!((x >> b) & 1)) continue;
      if (!p_[b]) { p_[b] = x; return true; }
      x ^= p_[b];
    }
    return false;
  }

  u64 max_xor(u64 base = 0) const {
    u64 r = base;
    for (int b = B - 1; b >= 0; --b)
      if (p_[b] && (r ^ p_[b]) > r) r ^= p_[b];
    return r;
  }

  u64 min_xor(u64 base = 0) const {
    u64 r = base;
    for (int b = B - 1; b >= 0; --b)
      if (p_[b] && (r ^ p_[b]) < r) r ^= p_[b];
    return r == base ? 0 : r;
  }

  bool has(u64 x) const {
    for (int b = B - 1; b >= 0; --b)
      if ((x >> b) & 1) x ^= p_[b];
    return x == 0;
  }

  /// 线性基张成的向量种类数（含 0）总能表示：2^rank 种
  int rank() const { int r = 0; for (int i = 0; i < B; ++i) if (p_[i]) ++r; return r; }

  /// 构建第 k 小查询所需的化简基（需在查询前调用一次）
  void build_kth() {
    ok_kth_ = true;
    kth_.clear();
    for (int i = 0; i < B; ++i) {
      if (!p_[i]) continue;
      for (int j = i - 1; j >= 0; --j)
        if (p_[j] && ((p_[i] >> j) & 1)) p_[i] ^= p_[j];
    }
    for (int i = 0; i < B; ++i) if (p_[i]) kth_.push_back(p_[i]);
  }

  /// 第 k 小（1 基）；需先 build_kth。0 是否可表示与无零值时的偏移由调用方处理。
  u64 kth_min(u64 k) const {
    if (!ok_kth_) wbw_error("kth_min 前需先调用 build_kth");
    u64 r = 0;
    u64 idx = k;
    for (size_t i = 0; i < kth_.size(); ++i)
      if (idx & (1ULL << i)) r ^= kth_[i];
    return r;
  }
};

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_LINALG_HPP