#ifndef WBWLIB_MATH_FOURIER_HPP
#define WBWLIB_MATH_FOURIER_HPP

/**
 * @file fourier.hpp
 * @brief 卷积加速：复数 FFT 与数论变换 NTT。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/modular.hpp
 *
 * 复杂度：O(n log n)。
 *
 * NTT 说明：迭代版蝶形，根 g=3 适配模 998244353（质数且 2^23 整除 p-1）。
 * 若要用其他模数，请使用 ntt_g<M,G> 系列（要求该模存在 G 次单位根）。
 *
 * 用法：
 *   // 实数卷积（或取整）：
 *   auto c = wbwlib::math::convolution_fft(a, b);   // vector<double>
 *   // 模数卷积（可配合 modint）：
 *   vector<M> c = wbwlib::math::convolution_ntt(a, b);
 */

#include <cmath>
#include <complex>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/modular.hpp"

namespace wbwlib {
namespace math {

// ================= 复数 FFT =================
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

/// 两个双精度序列的卷积（FFT，结果含浮点误差，取整用 round）
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
// 位逆序置换 + 蝶形，invert 为真时用根逆元并最后乘以 n 的逆
template<class M>
inline void ntt_core(std::vector<M>& a, bool invert, M root) {
  int n = (int)a.size();
  for (int i = 1, j = 0; i < n; ++i) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (int len = 2; len <= n; len <<= 1) {
    M wlen = root.pow((i64)((n) / len));
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

/// 标准 NTT：模 998244353，原根 3；a 长度必须是 2 的幂
template<class M>
inline void ntt(std::vector<M>& a, bool invert) {
  details::ntt_core(a, invert, M(3));
}

/// 通用 NTT：root 为模意义下的原根（g）
template<class M>
inline void ntt_g(std::vector<M>& a, bool invert, int g) {
  details::ntt_core(a, invert, M(g));
}

/// 模数卷积（结果长度 = a.size()+b.size()-1）
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