#ifndef WBWLIB_MATH_PRIMES_HPP
#define WBWLIB_MATH_PRIMES_HPP

/**
 * @file primes.hpp
 * @brief 质数工具：线性筛（素数/欧拉函数/莫比乌斯函数）、Miller-Rabin 素性测试、
 *        Pollard-Rho 质因数分解。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp
 *
 * 复杂度：
 *   线性筛 O(n)；Miller-Rabin O(k log n)（64 位内确定性 7 基底）；
 *   Pollard-Rho 期望 O(n^{1/4})。
 */

#include <algorithm>
#include <random>
#include <utility>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"

namespace wbwlib {
namespace math {

// ================= 线性筛 =================
/**
 * 线性欧拉筛。返回 [0, n] 的相关数组；仅计算素数列表（is_prime 可选）。
 *
 * 用法：
 *   vector<int> primes; vector<bool> is_p;
 *   primitive_sieve(n, primes, is_p);   // or sieve(n)
 */
inline std::vector<int> sieve(int n) {
  std::vector<int> primes;
  std::vector<bool> not_p(n + 1, false);
  for (int i = 2; i <= n; ++i) {
    if (!not_p[i]) primes.push_back(i);
    for (int p : primes) {
      if ((long long)i * p > n) break;
      not_p[i * p] = true;
      if (i % p == 0) break;
    }
  }
  return primes;
}

/**
 * 线性筛同时求欧拉函数 phi 与莫比乌斯函数 mu（n<=1e7）。
 * 返回 phi（n+1 长度，phi[1]=1），mu（n+1 长度，mu[1]=1）。
 * 说明：mu 用 int 存储，值为 /0/1。
 */
struct SieveResult {
  std::vector<int> primes;
  std::vector<int> phi;   ///< 欧拉函数
  std::vector<int> mu;    ///< 莫比乌斯函数
  std::vector<bool> is_prime;
};

inline SieveResult linear_sieve(int n) {
  SieveResult r;
  std::vector<bool> not_p(n + 1, false);
  r.phi.assign(n + 1, 0);
  r.mu.assign(n + 1, 0);
  r.is_prime.assign(n + 1, false);
  r.mu[1] = 1; r.phi[1] = 1;
  for (int i = 2; i <= n; ++i) {
    if (!not_p[i]) {
      r.primes.push_back(i);
      r.phi[i] = i - 1;
      r.mu[i] = -1;
      r.is_prime[i] = true;
    }
    for (int p : r.primes) {
      long long v = 1LL * i * p;
      if (v > n) break;
      not_p[(int)v] = true;
      if (i % p == 0) {
        r.phi[(int)v] = r.phi[i] * p;
        r.mu[(int)v] = 0;
        break;
      } else {
        r.phi[(int)v] = r.phi[i] * (p - 1);
        r.mu[(int)v] = -r.mu[i];
      }
    }
  }
  return r;
}

// ================= Miller-Rabin 素性测试 =================
/**
 * 对 64 位整数做确定性 Miller-Rabin。
 * 摩尔：a^(d*2^s)-1 的分解检测；7 组基底覆盖 < 2^64 全部整数。
 */
inline bool is_prime64(u64 n) {
  if (n < 2) return false;
  // 小素数快速排除
  static const u32 small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
  for (u32 p : small) {
    if (n % p == 0) return n == p;
  }
  if (n < 2181) { // 已经过上面小素数试除
    return true;  // n 大于全部 small 且小于 2181 时必为素数（推理保证）
  }
  u64 d = n - 1, s = 0;
  while ((d & 1) == 0) { d >>= 1; ++s; }
  static const u64 bases[] = {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL,
                              9780504ULL, 1795265022ULL};
  for (u64 a : bases) {
    if (a % n == 0) continue;          // a 是 n 的倍数时省略
    u64 x = qpow(a, (i64)d, n);        // 用模幂
    if (x == 1 || x == n - 1) continue;
    bool ok = false;
    for (u64 i = 1; i < s; ++i) {      // 检验 x^2^k == n-1 是否出现
      x = mul_mod(x, x, n);
      if (x == n - 1) { ok = true; break; }
      if (x == 1) break;               // 已非自同态，退化为合数
    }
    if (!ok) return false;
  }
  return true;
}

template<class T>
inline bool is_prime_small(T n) {
  if (n < 2) return false;
  for (T p = 2; p * p <= n; ++p)
    if (n % p == 0) return false;
  return true;
}

// ================= Pollard-Rho =================
// 用于找到 n 的一个非平凡因子。f(x) = (x*x + c) % n 的伪随机游走。
inline u64 pollard_rho(u64 n) {
  if (n % 2 == 0) return 2;
  // 用自增 C，配合随机种子保证不同 C 重新尝试
  std::mt19937_64 rng2((u64)(std::uintptr_t)&n ^ 0x9e3779b97f4a7c15ULL);
  while (true) {
    u64 x = rng2() % (n - 1) + 1;
    u64 y = x, c = rng2() % (n - 1) + 1;
    u64 d = 1;
    auto f = [&](u64 v) { return (mul_mod(v, v, n) + c) % n; };
    while (d == 1) {
      x = f(x);
      y = f(f(y));
      d = gcd(x > y ? x - y : y - x, n);   // |x-y| 与 n 求 gcd
    }
    if (d != n) return d;                   // d == n 说明偶遇退化，换 c
  }
}

/// 递归分解，把 n 的所有质因子（含重数）放入 factors
inline void rho_factor(u64 n, std::vector<u64>& factors) {
  if (n == 1) return;
  if (is_prime64(n)) { factors.push_back(n); return; }
  u64 d = pollard_rho(n);
  rho_factor(d, factors);
  rho_factor(n / d, factors);
}

/// 分解质因数，返回排序后的质因子列表（含重复）
inline std::vector<u64> factorize(u64 n) {
  std::vector<u64> fs;
  if (n > 1) rho_factor(n, fs);
  std::sort(fs.begin(), fs.end());
  return fs;
}

/// 返回 (质因子, 指数) 的去重列表
inline std::vector<std::pair<u64, int>> factor_exp(u64 n) {
  std::vector<u64> fs = factorize(n);
  std::vector<std::pair<u64, int>> res;
  for (u64 p : fs) {
    if (res.empty() || res.back().first != p) res.push_back({p, 1});
    else res.back().second++;
  }
  return res;
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_PRIMES_HPP