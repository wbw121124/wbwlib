#ifndef WBWLIB_MATH_FOURIER_HPP
#define WBWLIB_MATH_FOURIER_HPP

/**
 * @file fourier.hpp
 * @brief 卷积加速：复数 FFT 与数论变换 NTT。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/math/modular.hpp
 *
 * @par 复杂度
 * O(n log n)。
 *
 * NTT 说明：迭代版蝶形，根 g=3 适配模 998244353（质数且 2^23 整除 p-1）。
 * 若要用其他模数，请使用 ntt_g<M,G> 系列（要求该模存在 G 次单位根）。
 *
 * @par 示例
 * @code{.cpp}
 *   // 实数卷积（或取整）：
 *   auto c = wbwlib::math::convolution_fft(a, b);   // vector<double>
 *   // 模数卷积（可配合 modint）：
 *   vector<M> c = wbwlib::math::convolution_ntt(a, b);
 * @endcode
 */

#include <cmath>
#include <complex>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/modular.hpp"

namespace wbwlib {
namespace math {

// ================= 复数 FFT =================
/**
 * @brief 迭代版 Cooley-Tukey FFT：计算离散傅里叶变换 \f$X_k = \sum_{j=0}^{n-1} x_j \omega_n^{jk}\f$。
 *
 * n 必须是 2 的幂；invert 为真时做逆变换并除以 n。原地修改 a，复杂度 \f$O(n \log n)\f$。
 * @param a 输入/输出序列（原地变换）
 * @param invert false 为正变换，true 为逆变换
 */
inline void fft(std::vector<std::complex<double>>& a, bool invert) {
  int n = (int)a.size();
  // 位逆序置换
  for (int i = 1, j = 0; i < n; ++i) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (int len = 2; len <= n; len <<= 1) {
    double ang = 2.0 * 3.14159265358979323846 / len * (invert ? -1 : 1);
    std::complex<double> wlen(std::cos(ang), std::sin(ang));
    for (int i = 0; i < n; i += len) {
      std::complex<double> w(1.0, 0.0);
      for (int j = 0; j < len / 2; ++j) {
        std::complex<double> u = a[i + j], v = a[i + j + len / 2] * w;
        a[i + j] = u + v;
        a[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }
  if (invert) for (int i = 0; i < n; ++i) a[i] /= n;
}

/**
 * @brief 两个双精度序列的卷积（FFT）：\f$c_k = \sum_{i+j=k} a_i b_j\f$。
 *
 * 结果含浮点误差，取整请用 round。
 * @param a 序列 1
 * @param b 序列 2
 * @return 卷积结果，长度 \f$|a| + |b| - 1\f$
 */
inline std::vector<double> convolution_fft(const std::vector<double>& a,
                                           const std::vector<double>& b) {
  int n = 1;
  while (n < (int)(a.size() + b.size() - 1)) n <<= 1;
  std::vector<std::complex<double>> fa(n), fb(n);
  for (size_t i = 0; i < a.size(); ++i) fa[i] = a[i];
  for (size_t i = 0; i < b.size(); ++i) fb[i] = b[i];
  fft(fa, false); fft(fb, false);
  for (int i = 0; i < n; ++i) fa[i] *= fb[i];
  fft(fa, true);
  std::vector<double> res(a.size() + b.size() - 1);
  for (size_t i = 0; i < res.size(); ++i) res[i] = fa[i].real();
  return res;
}

// ================= NTT（模板模数版） =================
namespace details {
/// 位逆序置换 + 蝶形运算；invert 为真时用根逆元并最后乘以 n 的逆
template<class M>
inline void ntt_core(std::vector<M>& a, bool invert, M root) {
  int n = (int)a.size();
  const i64 mod = (i64)M::mod_value();
  for (int i = 1, j = 0; i < n; ++i) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (int len = 2; len <= n; len <<= 1) {
    M wlen = root.pow((mod - 1) / len);   // g^((mod-1)/len)
    if (invert) wlen = wlen.inv();
    for (int i = 0; i < n; i += len) {
      M w = M(1);
      for (int j = 0; j < len / 2; ++j) {
        M u = a[i + j], v = a[i + j + len / 2] * w;
        a[i + j] = u + v;
        a[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }
  if (invert) {
    M inv_n = M(n).inv();
    for (int i = 0; i < n; ++i) a[i] *= inv_n;
  }
}
} // namespace details

/**
 * @brief 标准 NTT：模 998244353，原根 3；a 长度必须是 2 的幂。
 *
 * 用原根 \f$g\f$ 的第 \f$\frac{p-1}{len}\f$ 次幂作为 len 阶单位根。
 * @tparam M 模整数类（mod_value() 须为 998244353）
 * @param a 输入/输出序列（原地变换）
 * @param invert false 为正变换，true 为逆变换
 */
template<class M>
inline void ntt(std::vector<M>& a, bool invert) {
  details::ntt_core(a, invert, M(3));
}

/**
 * @brief 通用 NTT：可指定原根 g（要求模数存在对应阶的单位根）。
 * @tparam M 模整数类
 * @param a 输入/输出序列（原地变换，长度需为 2 的幂）
 * @param invert false 为正变换，true 为逆变换
 * @param g 模意义下的原根
 */
template<class M>
inline void ntt_g(std::vector<M>& a, bool invert, int g) {
  details::ntt_core(a, invert, M(g));
}

/**
 * @brief 模数卷积（NTT）：\f$c_k = \sum_{i+j=k} a_i b_j \bmod p\f$。
 * @tparam M 模整数类（需 NTT 友好，如 modint<998244353>）
 * @param a 序列 1
 * @param b 序列 2
 * @return 卷积结果，长度 \f$|a| + |b| - 1\f$
 */
template<class M>
inline std::vector<M> convolution_ntt(std::vector<M> a, std::vector<M> b) {
  if (a.empty() || b.empty()) return {};
  int need = (int)a.size() + (int)b.size() - 1;
  int n = 1;
  while (n < need) n <<= 1;
  a.resize(n); b.resize(n);
  ntt(a, false); ntt(b, false);
  for (int i = 0; i < n; ++i) a[i] *= b[i];
  ntt(a, true);
  a.resize(need);
  return a;
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_FOURIER_HPP