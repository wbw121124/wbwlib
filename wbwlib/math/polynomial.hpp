#ifndef WBWLIB_MATH_POLYNOMIAL_HPP
#define WBWLIB_MATH_POLYNOMIAL_HPP

/**
 * @file polynomial.hpp
 * @brief 多项式运算（形式幂级数，截断到指定长度）。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/modular.hpp, wbwlib/math/fourier.hpp
 *
 * 复杂度：deriv/integ O(n)；inv/ln/exp/sqrt/pow O(n log n)；divmod O(n log n)。
 *
 * 固定使用 poly_mod = modint<998244353>（质数、NTT 友好）。
 * 前提条件（违反时触发断言）：
 *   inv  要求 f[0] != 0；
 *   ln   要求 f[0] == 1；
 *   sqrt 要求 f[0] == 1；
 *   exp  要求 f[0] == 0；
 *   pow  要求 f[0] == 1（否则先对常数项提公因式）。
 *
 * 所有函数以「前 need 项」为长度语义。
 * 用法示例：
 *   std::vector<poly_mod> f = {poly_mod(1), poly_mod(1), poly_mod(2)};
 *   auto g = poly::invf(f, 10);          // 1/f 的前 10 项
 */

#include <algorithm>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/fourier.hpp"

namespace wbwlib {
namespace math {

using poly_mod = modint<998244353>;       ///< 默认多项式环元素
constexpr i64 kPolyMod = 998244353;

namespace poly {
using M = poly_mod;

/// 截断到前 lim 项
inline std::vector<M> trim(std::vector<M> a, int lim) {
  a.resize((std::min)((int)a.size(), lim));
  return a;
}

/// 完整卷积后取前 lim 项
inline std::vector<M> mul(std::vector<M> a, std::vector<M> b, int lim) {
  std::vector<M> c = convolution_ntt(a, b);
  if ((int)c.size() > lim) c.resize(lim);
  return c;
}

/// 0..n 的模逆表（线性递推）
inline std::vector<M> inverses(int n) {
  std::vector<M> iv(n + 1);
  if (n >= 1) iv[1] = M(1);
  for (int i = 2; i <= n; ++i)
    iv[i] = M(-(kPolyMod / i)) * iv[kPolyMod % i];
  return iv;
}

/// 求导（长度减一）
inline std::vector<M> deriv(const std::vector<M>& f) {
  if (f.size() <= 1) return {};
  std::vector<M> r(f.size() - 1);
  for (size_t i = 1; i < f.size(); ++i) r[i - 1] = f[i] * M((i64)i);
  return r;
}

/// 积分（常数项补 0）
inline std::vector<M> integ(const std::vector<M>& f) {
  auto iv = inverses((int)f.size());
  std::vector<M> r(f.size() + 1);
  for (size_t i = 1; i <= f.size(); ++i) r[i] = f[i - 1] * iv[i];
  return r;
}

/// 乘法逆 mod x^need；要求 f[0] != 0
inline std::vector<M> invf(const std::vector<M>& f, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] != M(0));
  std::vector<M> g(1, M(1) / f[0]);
  int m = 1;
  while (m < need) {
    m <<= 1;
    std::vector<M> fm = trim(f, m);
    std::vector<M> fg = mul(fm, g, m);        // f*g mod x^m
    for (auto& v : fg) v = -v;
    fg[0] += M(2);                            // 2 - f*g
    g = mul(g, fg, m);
    g.resize(m);
  }
  g.resize(need);
  return g;
}

/// ln f mod x^need；要求 f[0] == 1
inline std::vector<M> lnf(const std::vector<M>& f, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] == M(1));
  std::vector<M> df = deriv(f);               // f'
  std::vector<M> fi = invf(f, need);          // 1/f
  std::vector<M> t = mul(df, fi, need - 1);   // f'/f，长度 need-1
  auto iv = inverses(need);
  std::vector<M> res(need);
  for (int i = 1; i < need; ++i) res[i] = t[i - 1] * iv[i];   // 积分
  return res;
}

/// exp f mod x^need；要求 f[0] == 0
inline std::vector<M> expf(const std::vector<M>& f, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] == M(0));
  std::vector<M> g(1, M(1));
  int m = 1;
  while (m < need) {
    m <<= 1;
    std::vector<M> lg = lnf(g, m);
    for (auto& v : lg) v = -v;
    lg[0] += M(1);                            // 1 - ln(g)
    std::vector<M> fm = trim(f, m);
    for (int i = 0; i < m; ++i)
      lg[i] += (i < (int)fm.size() ? fm[i] : M(0));   // 1 - ln(g) + f
    g = mul(g, lg, m);
    g.resize(m);
  }
  g.resize(need);
  return g;
}

/// sqrt f mod x^need；要求 f[0] == 1
inline std::vector<M> sqrtf(const std::vector<M>& f, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] == M(1));
  static const M inv2 = M(2).inv();
  std::vector<M> g(1, M(1));
  int m = 1;
  while (m < need) {
    m <<= 1;
    std::vector<M> gi = invf(g, m);           // 1/g
    std::vector<M> fm = trim(f, m);
    std::vector<M> t = mul(fm, gi, m);        // f/g
    for (int i = 0; i < m; ++i) {
      M gv = (i < (int)g.size()) ? g[i] : M(0);
      t[i] = (t[i] + gv) * inv2;              // (f/g + g)/2
    }
    g.swap(t);
    g.resize(m);
  }
  g.resize(need);
  return g;
}

/// f^k mod x^need；要求 f[0] == 1
inline std::vector<M> powf(std::vector<M> f, i64 k, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] == M(1));
  std::vector<M> l = lnf(f, need);
  M coeff(k % kPolyMod);
  for (auto& v : l) v *= coeff;
  return expf(l, need);
}

/// 多项式带余除法：q = f / g，r = f % g（deg r < deg g）
/// 说明：要求 g 的最高次系数非 0（g.back() != 0）。R 仅截断到 m-1 项。
inline void divmod(const std::vector<M>& f, const std::vector<M>& g,
                   std::vector<M>& q, std::vector<M>& r) {
  int n = (int)f.size(), m = (int)g.size();
  if (n < m) { q.clear(); r = f; return; }
  std::vector<M> rf(f.rbegin(), f.rend());    // 系数反转
  std::vector<M> rgv(g.rbegin(), g.rend());
  int need = n - m + 1;
  std::vector<M> ig = invf(trim(rgv, need), need);   // 1/reverse(g)
  std::vector<M> qrev = mul(rf, ig, need);
  q.assign(qrev.rbegin(), qrev.rend());       // 反转为 q
  std::vector<M> qg = mul(q, g, n);
  int rlen = (std::max)(m - 1, 0);
  r.assign(rlen, M(0));
  for (int i = 0; i < rlen; ++i) {
    M a = (i < n) ? f[i] : M(0);
    M b = (i < (int)qg.size()) ? qg[i] : M(0);
    r[i] = a - b;
  }
}

} // namespace poly
} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_POLYNOMIAL_HPP