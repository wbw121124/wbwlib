#ifndef WBWLIB_MATH_FWT_HPP
#define WBWLIB_MATH_FWT_HPP

/**
 * @file fwt.hpp
 * @brief 快速沃尔什变换 FWT：集合幂级数的 OR / AND / XOR 卷积。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/math/modular.hpp
 *
 * @par 复杂度
 * 每维 O(n log n)。
 *
 * @par 示例
 * @code{.cpp}
 *   using M = wbwlib::math::modint<998244353>;
 *   std::vector<M> a(n), b(n);
 *   std::vector<M> c = wbwlib::math::convolution_xor(a, b);  // 或 _or/_and
 * @endcode
 *
 * 原理：在变换域做点乘再逆变换。
 */

#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/modular.hpp"

namespace wbwlib {
namespace math {

namespace details {

/// FWT 核心蝶形：kind 0/1/2 对应 OR/AND/XOR，inverse 为真时做逆变换（XOR 逆变换需除以 2）
template<class M, bool inverse>
inline void fwt_core(std::vector<M>& a, int kind) {
  int n = (int)a.size();
  for (int len = 1; len < n; len <<= 1) {
    for (int i = 0; i < n; i += len << 1) {
      for (int j = 0; j < len; ++j) {
        M &x = a[i + j], &y = a[i + j + len];
        if (kind == 0) {            // OR
          if (!inverse) y += x; else y -= x;
        } else if (kind == 1) {     // AND
          if (!inverse) x += y; else x -= y;
        } else {                    // XOR
          M u = x + y, v = x - y;
          if (inverse) u /= M(2), v /= M(2);
          x = u; y = v;
        }
      }
    }
  }
}

} // namespace details

/**
 * @brief OR 卷积正变换（原地；长度需为 2 的幂）。
 * @tparam M 模整数类
 * @param a 待变换序列
 */
template<class M> void fwt_or(std::vector<M>& a)   { details::fwt_core<M, false>(a, 0); }

/**
 * @brief OR 卷积逆变换（原地；长度需为 2 的幂）。
 * @tparam M 模整数类
 * @param a 待逆变换序列
 */
template<class M> void ifwt_or(std::vector<M>& a)  { details::fwt_core<M, true>(a, 0); }

/**
 * @brief AND 卷积正变换（原地；长度需为 2 的幂）。
 * @tparam M 模整数类
 * @param a 待变换序列
 */
template<class M> void fwt_and(std::vector<M>& a)  { details::fwt_core<M, false>(a, 1); }

/**
 * @brief AND 卷积逆变换（原地；长度需为 2 的幂）。
 * @tparam M 模整数类
 * @param a 待逆变换序列
 */
template<class M> void ifwt_and(std::vector<M>& a) { details::fwt_core<M, true>(a, 1); }

/**
 * @brief XOR 卷积正变换（原地；长度需为 2 的幂）。
 * @tparam M 模整数类
 * @param a 待变换序列
 */
template<class M> void fwt_xor(std::vector<M>& a)  { details::fwt_core<M, false>(a, 2); }

/**
 * @brief XOR 卷积逆变换（原地；长度需为 2 的幂）。
 * @tparam M 模整数类
 * @param a 待逆变换序列
 */
template<class M> void ifwt_xor(std::vector<M>& a) { details::fwt_core<M, true>(a, 2); }

/**
 * @brief OR 卷积：\f$c_k = \sum_{i | j = k} a_i b_j\f$。
 * @tparam M 模整数类
 * @param a 序列 1
 * @param b 序列 2
 * @return 卷积结果（长度向上取整到 2 的幂）
 */
template<class M>
std::vector<M> convolution_or(std::vector<M> a, std::vector<M> b) {
  int n = 1; while (n < (int)std::max(a.size(), b.size())) n <<= 1;
  a.resize(n); b.resize(n);
  fwt_or(a); fwt_or(b);
  for (int i = 0; i < n; ++i) a[i] *= b[i];
  ifwt_or(a);
  return a;
}

/**
 * @brief AND 卷积：\f$c_k = \sum_{i \& j = k} a_i b_j\f$。
 * @tparam M 模整数类
 * @param a 序列 1
 * @param b 序列 2
 * @return 卷积结果（长度向上取整到 2 的幂）
 */
template<class M>
std::vector<M> convolution_and(std::vector<M> a, std::vector<M> b) {
  int n = 1; while (n < (int)std::max(a.size(), b.size())) n <<= 1;
  a.resize(n); b.resize(n);
  fwt_and(a); fwt_and(b);
  for (int i = 0; i < n; ++i) a[i] *= b[i];
  ifwt_and(a);
  return a;
}

/**
 * @brief XOR 卷积：\f$c_k = \sum_{i \oplus j = k} a_i b_j\f$。
 * @tparam M 模整数类
 * @param a 序列 1
 * @param b 序列 2
 * @return 卷积结果（长度向上取整到 2 的幂）
 */
template<class M>
std::vector<M> convolution_xor(std::vector<M> a, std::vector<M> b) {
  int n = 1; while (n < (int)std::max(a.size(), b.size())) n <<= 1;
  a.resize(n); b.resize(n);
  fwt_xor(a); fwt_xor(b);
  for (int i = 0; i < n; ++i) a[i] *= b[i];
  ifwt_xor(a);
  return a;
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_FWT_HPP