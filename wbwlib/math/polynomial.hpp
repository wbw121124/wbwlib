#ifndef WBWLIB_MATH_POLYNOMIAL_HPP
#define WBWLIB_MATH_POLYNOMIAL_HPP

/**
 * @file polynomial.hpp
 * @brief 多项式运算（形式幂级数，截断到指定长度）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/math/modular.hpp, wbwlib/math/fourier.hpp
 *
 * @par 复杂度
 * deriv/integ O(n)；inv/ln/exp/sqrt/pow O(n log n)；divmod O(n log n)。
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
 * @par 示例
 * @code{.cpp}
 *   std::vector<poly_mod> f = {poly_mod(1), poly_mod(1), poly_mod(2)};
 *   auto g = poly::invf(f, 10);          // 1/f 的前 10 项
 * @endcode
 */

#include <algorithm>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/fourier.hpp"

namespace wbwlib {
namespace math {

using poly_mod = modint<998244353>;       ///< 默认多项式环元素
constexpr i64 kPolyMod = 998244353;       ///< 默认多项式环模数

namespace poly {
using M = poly_mod;

/**
 * @brief 截断多项式到前 lim 项。
 * @param a 多项式系数
 * @param lim 保留项数
 * @return 截断后的系数（长度 \f$\min(|a|, lim)\f$）
 */
inline std::vector<M> trim(std::vector<M> a, int lim) {
  a.resize((std::min)((int)a.size(), lim));
  return a;
}

/**
 * @brief 完整卷积后取前 lim 项：\f$(f \cdot g) \bmod x^{lim}\f$。
 * @param a 多项式 f
 * @param b 多项式 g
 * @param lim 截断项数
 * @return 卷积结果的前 lim 项
 */
inline std::vector<M> mul(std::vector<M> a, std::vector<M> b, int lim) {
  std::vector<M> c = convolution_ntt(a, b);
  if ((int)c.size() > lim) c.resize(lim);
  return c;
}

/**
 * @brief 0..n 的模逆表（线性递推）。
 *
 * 递推式：\f$i^{-1} \equiv -\lfloor p/i \rfloor \cdot (p \bmod i)^{-1} \pmod p\f$。
 * @param n 上界
 * @return iv[i] = \f$i^{-1} \bmod p\f$（长度 n+1，iv[0] 未定义）
 */
inline std::vector<M> inverses(int n) {
  std::vector<M> iv(n + 1);
  if (n >= 1) iv[1] = M(1);
  for (int i = 2; i <= n; ++i)
    iv[i] = M(-(kPolyMod / i)) * iv[kPolyMod % i];
  return iv;
}

/**
 * @brief 多项式求导：\f$(f')_i = (i+1) f_{i+1}\f$。
 * @param f 多项式系数
 * @return 导数系数（长度 \f$|f| - 1\f$；|f| <= 1 时为空）
 */
inline std::vector<M> deriv(const std::vector<M>& f) {
  if (f.size() <= 1) return {};
  std::vector<M> r(f.size() - 1);
  for (size_t i = 1; i < f.size(); ++i) r[i - 1] = f[i] * M((i64)i);
  return r;
}

/**
 * @brief 多项式积分：\f$(\int f)_i = f_{i-1} / i\f$（常数项补 0）。
 * @param f 多项式系数
 * @return 积分系数（长度 \f$|f| + 1\f$）
 */
inline std::vector<M> integ(const std::vector<M>& f) {
  auto iv = inverses((int)f.size());
  std::vector<M> r(f.size() + 1);
  for (size_t i = 1; i <= f.size(); ++i) r[i] = f[i - 1] * iv[i];
  return r;
}

/**
 * @brief 形式幂级数乘法逆：计算 \f$1 / f \bmod x^{need}\f$。
 *
 * 牛顿迭代倍增：\f$g \leftarrow g(2 - f g)\f$。要求 \f$f[0] \ne 0\f$。
 * @param f 多项式 f
 * @param need 所需项数
 * @return \f$1/f\f$ 的前 need 项
 */
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

/**
 * @brief 形式幂级数对数：\f$\ln f = \int f' / f\f$，计算 \f$\ln f \bmod x^{need}\f$。
 *
 * 要求 \f$f[0] = 1\f$。
 * @param f 多项式 f
 * @param need 所需项数
 * @return \f$\ln f\f$ 的前 need 项
 */
inline std::vector<M> lnf(const std::vector<M>& f, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] == M(1));
  std::vector<M> res(need, M(0));
  std::vector<M> df = deriv(f);               // f'
  if (df.empty()) return res;                 // f 为常数 → ln f = 0
  std::vector<M> fi = invf(f, need);          // 1/f
  std::vector<M> t = mul(df, fi, need - 1);   // f'/f，长度 need-1
  auto iv = inverses(need);
  for (int i = 1; i < need; ++i) res[i] = t[i - 1] * iv[i];   // 积分
  return res;
}

/**
 * @brief 形式幂级数指数：计算 \f$\exp f \bmod x^{need}\f$。
 *
 * 牛顿迭代：\f$g \leftarrow g(1 - \ln g + f)\f$。要求 \f$f[0] = 0\f$。
 * @param f 多项式 f
 * @param need 所需项数
 * @return \f$\exp f\f$ 的前 need 项
 */
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

/**
 * @brief 形式幂级数平方根：计算 \f$\sqrt{f} \bmod x^{need}\f$。
 *
 * 牛顿迭代：\f$g \leftarrow \frac{1}{2}(f/g + g)\f$。要求 \f$f[0] = 1\f$。
 * @param f 多项式 f
 * @param need 所需项数
 * @return \f$\sqrt{f}\f$ 的前 need 项
 */
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

/**
 * @brief 形式幂级数幂：\f$f^k = \exp(k \ln f)\f$，计算 \f$f^k \bmod x^{need}\f$。
 *
 * 要求 \f$f[0] = 1\f$（否则需先对常数项提公因式）。
 * @param f 多项式 f
 * @param k 指数
 * @param need 所需项数
 * @return \f$f^k\f$ 的前 need 项
 */
inline std::vector<M> powf(std::vector<M> f, i64 k, int need) {
  WBWLIB_ASSERT(!f.empty() && f[0] == M(1));
  std::vector<M> l = lnf(f, need);
  M coeff(k % kPolyMod);
  for (auto& v : l) v *= coeff;
  return expf(l, need);
}

/**
 * @brief 多项式带余除法：\f$f = q \cdot g + r\f$，其中 \f$\deg r < \deg g\f$。
 *
 * 做法：反转系数后求 \f$1/\mathrm{rev}(g)\f$ 截断相乘再反转得 q。要求 g 的最高次系数非 0。
 * @param f 被除多项式
 * @param g 除式多项式（g.back() != 0）
 * @param q 输出参数，商（deg q = deg f - deg g，若 f 长度小于 g 则为空）
 * @param r 输出参数，余式（长度 \f$\max(\deg g, 1)\f$；f 长度小于 g 时 r = f）
 */
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