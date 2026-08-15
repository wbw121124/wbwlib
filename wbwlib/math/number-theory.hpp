#ifndef WBWLIB_MATH_NUMBER_THEORY_HPP
#define WBWLIB_MATH_NUMBER_THEORY_HPP

/**
 * @file number-theory.hpp
 * @brief 数论基础：快速幂、快速乘、gcd/lcm、扩展欧几里得、逆元、CRT/EXCRT、欧拉定理、整除分块。
 *
 * 依赖：core/base.hpp
 *
 * 函数均模板化，建议使用 i64（必要时借助 WBWLIB_HAS_INT128 的 wbwlib::i128 防溢出）。
 */

#include <utility>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace math {

// ================= 快速幂 =================
/**
 * a^p 对 mod 取模。p 可为任意非负整数，满足 0 <= p。
 * 使用倍增法，O(log p)。
 */
template<class T>
inline T qpow(T a, i64 p, T mod) {
  T r = 1 % mod;
  a %= mod;
  while (p > 0) {
    if (p & 1) r = T((i64)r * a % mod);
    a = T((i64)a * a % mod);
    p >>= 1;
  }
  return r;
}

/**
 * 无取模的快速幂（用于不进位的浮点/或溢出可接受的加速场景）。
 * 注意：不同语义，安全起见限制为非负指数。
 */
template<class T>
inline T qpow_plain(T a, i64 p) {
  T r = 1;
  while (p > 0) {
    if (p & 1) r = r * a;
    a = a * a;
    p >>= 1;
  }
  return r;
}

// ================= 快速乘（防溢出） =================
/**
 * a * b % mod 精确计算，适用于 mod 超过 1e18、i64 乘法溢出场景。
 * 有 __int128 时直接转 128 位；否则用「龟速乘」加法倍增，O(log b)。
 */
template<class T>
inline T mul_mod(T a, T b, T mod) {
#if WBWLIB_HAS_INT128
  return (T)((i128)a * b % mod);
#else
  T r = 0;
  a %= mod;
  while (b > 0) {
    if (b & 1) r = (r + a) >= mod ? r + a - mod : r + a;
    a = (a + a) >= mod ? a + a - mod : a + a;
    b >>= 1;
  }
  return r;
#endif
}

// ================= gcd / lcm / 扩展欧几里得 =================
/// 最大公约数（非负）
template<class T>
inline T gcd(T a, T b) {
  while (b) { T t = a % b; a = b; b = t; }
  return a < 0 ? -a : a;
}

/// 最小公倍数（结果以 T 截断，注意可能溢出，需要时转 i128）
template<class T>
inline T lcm(T a, T b) {
  return a / gcd(a, b) * b;
}

/// 扩展欧几里得：求 ax + by = gcd(a, b)，返回 gcd，解 (x, y)
template<class T>
inline T ext_gcd(T a, T b, T& x, T& y) {
  if (b == 0) { x = 1; y = 0; return a < 0 ? -a : a; }
  T x1, y1;
  T g = ext_gcd(b, a % b, x1, y1);
  x = y1;
  y = x1 - (a / b) * y1;
  return g;
}

// ================= 逆元 =================
/**
 * a 在模 mod 意义下的乘法逆元（要求 gcd(a, mod) == 1 且 mod 为质数？不要求，仅要求互质）。
 * 使用扩展欧几里得，O(log mod)。返回 0 表示无逆元。
 */
template<class T>
inline T inv(T a, T mod) {
  a %= mod;
  if (a < 0) a += mod;
  T x, y;
  if (ext_gcd(a, mod, x, y) != 1) return 0;   // 无逆元
  x %= mod;
  return x < 0 ? T(x + mod) : T(x);
}

/// 费马小定理逆元（仅当 mod 为质数），O(log mod)
template<class T>
inline T mod_inv_prime(T a, T mod) {
  return qpow(a, mod - 2, mod);
}

// ================= 中国剩余定理 =================
/**
 * CRT：解同余方程组 x = a_i (mod m_i)，其中 m_i 两两互质。
 * 返回最小非负解。M 为各 m_i 之积（由调用方保证不溢出）。
 */
template<class T>
inline T crt(const std::vector<T>& a, const std::vector<T>& m, T M) {
  T x = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    T Mi = M / m[i];
    x = (x + mul_mod(mul_mod(a[i], Mi, M), inv(Mi % m[i], m[i]), M)) % M;
  }
  return x;
}

/**
 * EXCRT：m_i 不必两两互质。返回最小非负解与最小公倍周期，
 * 无解时返回 -1（可通过 TBound 区分）。（返回小技巧：用 pair<bool,...> 更清晰）
 */
// 供 excrt 使用的 i64 逆元（调用处保证互质，逆元必存在）
inline i64 inv_inline(i64 a, i64 mod) {
  i64 x, y;
  ext_gcd(a, mod, x, y);
  return (x % mod + mod) % mod;
}

struct ExcrtResult {
  bool valid;   ///< 是否有解
  i64 x;        ///< 最小非负解
  i64 lcm;      ///< 解的周期（模意义下的最小公倍数）
};

inline ExcrtResult excrt(const std::vector<i64>& a, const std::vector<i64>& m) {
  i64 x = 0, cur = 1;   // 已处理方程的通解 x + k*cur（cur 为已处理模数的最小公倍数）
  for (size_t i = 0; i < a.size(); ++i) {
    i64 g = gcd(cur, m[i]);              // cur 与 m[i] 的公因子
    i64 q = m[i] / g;                    // 解 t 的模数
    i64 diff = ((a[i] - x) % m[i] + m[i]) % m[i];
    if (diff % g != 0) return {false, 0, 0};       // 无解
    // 求 t：t ≡ (diff/g) * inv(cur/g, q) (mod q)
    i64 invc = inv_inline(cur / g, q);   // gcd(cur/g, q)=1，逆元必存在
    i64 t = mul_mod(diff / g, invc, q);
    t = (t % q + q) % q;
    x += cur * t;
    cur *= q;                            // 更新 lcm
    x %= cur;
  }
  return {true, x, cur};
}

// ================= 欧拉定理 / 欧拉函数 =================
/// 欧拉函数 phi(n)：1..n 中与 n 互质的个数。O(sqrt(n)) 试除法。
template<class T>
inline T phi(T n) {
  T r = n;
  for (T p = 2; p * p <= n; ++p) {
    if (n % p == 0) {
      r = r / p * (p - 1);
      while (n % p == 0) n /= p;
    }
  }
  if (n > 1) r = r / n * (n - 1);
  return r;
}

// ================= 整除分块 =================
/**
 * 求 sum_{i=1}^{n} f(i)，且 i 在相同 floor(n/i) 的块内 f 恒定或可快速求和时使用。
 * 返回每个块的 (l, r)。复杂度 O(sqrt(n))。
 * 用法：
 *   for (auto [l,r] : floor_blocks(n)) { ... floor(n/l) ... }
 */
struct IntBlock {
  i64 l, r;
};
inline std::vector<IntBlock> floor_blocks(i64 n) {
  std::vector<IntBlock> res;
  for (i64 l = 1; l <= n; ) {
    i64 v = n / l;
    i64 r = n / v;
    res.push_back({l, r});
    l = r + 1;
  }
  return res;
}

// ================= 数论求和工具 =================
/// sum_{i=1}^n floor(n/i) 经典公式，O(sqrt(n))
inline i64 sum_floor(i64 n) {
  i64 s = 0;
  for (i64 l = 1; l <= n; ) {
    i64 v = n / l, r = n / v;
    s += v * (r - l + 1);
    l = r + 1;
  }
  return s;
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_NUMBER_THEORY_HPP