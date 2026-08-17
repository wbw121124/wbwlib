#ifndef WBWLIB_MATH_NUMBER_THEORY_HPP
#define WBWLIB_MATH_NUMBER_THEORY_HPP

/**
 * @file number-theory.hpp
 * @brief 数论基础：快速幂、快速乘、gcd/lcm、扩展欧几里得、逆元、CRT/EXCRT、欧拉定理、整除分块。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
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
 * @brief 快速幂：计算 \f$a^p \bmod mod\f$，p 可为任意非负整数。
 *
 * 使用倍增法，复杂度 \f$O(\log p)\f$。
 * @tparam T 整数类型
 * @param a 底数
 * @param p 指数（非负）
 * @param mod 模数
 * @return \f$a^p \bmod mod\f$
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
 * @brief 无取模的快速幂：直接计算 \f$a^p\f$（用于不进位的浮点/或溢出可接受的加速场景）。
 *
 * @attention 不同语义，安全起见限制为非负指数。
 * @tparam T 支持 * 运算的类型
 * @param a 底数
 * @param p 指数（非负）
 * @return \f$a^p\f$
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
 * @brief 快速乘：精确计算 \f$a \cdot b \bmod mod\f$，适用于 mod 超过 1e18、i64 乘法溢出的场景。
 *
 * 有 __int128 时直接转 128 位；否则用「龟速乘」加法倍增，\f$O(\log b)\f$。
 * @tparam T 整数类型
 * @param a 乘数
 * @param b 乘数
 * @param mod 模数
 * @return \f$a \cdot b \bmod mod\f$
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
/**
 * @brief 最大公约数（结果非负）。
 * @tparam T 整数类型
 * @param a 整数
 * @param b 整数
 * @return \f$\gcd(a, b)\f$（非负）
 */
template<class T>
inline T gcd(T a, T b) {
  while (b) { T t = a % b; a = b; b = t; }
  return a < 0 ? -a : a;
}

/**
 * @brief 最小公倍数：\f$\operatorname{lcm}(a, b) = \frac{a \cdot b}{\gcd(a, b)}\f$。
 *
 * 结果以 T 截断，注意可能溢出，需要时转 i128。
 * @tparam T 整数类型
 * @param a 整数
 * @param b 整数
 * @return \f$\operatorname{lcm}(a, b)\f$
 */
template<class T>
inline T lcm(T a, T b) {
  return a / gcd(a, b) * b;
}

/**
 * @brief 扩展欧几里得：求 \f$ax + by = \gcd(a, b)\f$ 的一组整数解 \f$(x, y)\f$。
 * @tparam T 整数类型
 * @param a 系数
 * @param b 系数
 * @param x 输出参数，方程的一组解 x
 * @param y 输出参数，方程的一组解 y
 * @return \f$\gcd(a, b)\f$（非负）
 */
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
 * @brief a 在模 mod 意义下的乘法逆元（仅要求 \f$\gcd(a, mod) = 1\f$，不要求 mod 为质数）。
 *
 * 使用扩展欧几里得，\f$O(\log mod)\f$。返回 0 表示无逆元。
 * @tparam T 整数类型
 * @param a 求逆的数
 * @param mod 模数
 * @return 满足 \f$a \cdot x \equiv 1 \pmod{mod}\f$ 的 x（[0, mod) 内），无逆元时返回 0
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

/**
 * @brief 费马小定理求逆元（仅当 mod 为质数时成立）。
 *
 * 由 \f$a^{mod-1} \equiv 1 \pmod{mod}\f$ 得 \f$a^{-1} \equiv a^{mod-2} \pmod{mod}\f$，\f$O(\log mod)\f$。
 * @tparam T 整数类型
 * @param a 求逆的数（a 与 mod 互质）
 * @param mod 质数模数
 * @return \f$a^{-1} \bmod mod\f$
 */
template<class T>
inline T mod_inv_prime(T a, T mod) {
  return qpow(a, mod - 2, mod);
}

// ================= 中国剩余定理 =================
/**
 * @brief 中国剩余定理（CRT）：解同余方程组 \f$x \equiv a_i \pmod{m_i}\f$（\f$m_i\f$ 两两互质）。
 *
 * @tparam T 整数类型
 * @param a 余数列表
 * @param m 模数列表（两两互质）
 * @param M 各模数之积（由调用方保证不溢出）
 * @return 最小非负解 \f$x\f$（\f$0 \le x < M\f$）
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
 * @brief 供 excrt 使用的 i64 逆元（调用处保证互质，逆元必存在）。
 * @param a 求逆的数（与 mod 互质）
 * @param mod 模数
 * @return \f$a^{-1} \bmod mod\f$（[0, mod) 内）
 */
inline i64 inv_inline(i64 a, i64 mod) {
  i64 x, y;
  ext_gcd(a, mod, x, y);
  return (x % mod + mod) % mod;
}

/**
 * @brief 扩展中国剩余定理（EXCRT）的求解结果。
 */
struct ExcrtResult {
  bool valid;   ///< 是否有解
  i64 x;        ///< 最小非负解
  i64 lcm;      ///< 解的周期（模意义下的最小公倍数）
};

/**
 * @brief 扩展中国剩余定理（EXCRT）：解同余方程组 \f$x \equiv a_i \pmod{m_i}\f$，\f$m_i\f$ 不必两两互质。
 *
 * 复杂度 \f$O(n \log m)\f$。
 * @param a 余数列表
 * @param m 模数列表（不必两两互质）
 * @return 求解结果，无解时 valid 为 false
 */
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
/**
 * @brief 欧拉函数：1..n 中与 n 互质的个数，\f$\varphi(n) = n \prod_{p \mid n} (1 - \frac{1}{p})\f$。
 *
 * 复杂度 \f$O(\sqrt n)\f$ 试除法。
 * @tparam T 整数类型
 * @param n 正整数
 * @return \f$\varphi(n)\f$
 */
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
 * @brief 整除分块：按 \f$\lfloor n / i \rfloor\f$ 取值相同分块，用于 \f$\sum_{i=1}^{n} f(\lfloor n/i \rfloor)\f$ 类求和。
 *
 * 返回每个块的左右端点 (l, r)，块内 \f$\lfloor n/i \rfloor\f$ 恒定。复杂度 \f$O(\sqrt n)\f$。
 * @param n 正整数
 * @return 块区间列表
 *
 * 用法：
 *   for (auto [l,r] : floor_blocks(n)) { ... floor(n/l) ... }
 */
struct IntBlock {
  i64 l, r;
};

/**
 * @brief 求所有 \f$\lfloor n/i \rfloor\f$ 取值相同的块 \f$[l, r]\f$。
 * @param n 正整数
 * @return 块列表（\f$O(\sqrt n)\f$ 个），块内 \f$\lfloor n/i \rfloor\f$ 相同
 */
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
/**
 * @brief 计算 \f$\sum_{i=1}^{n} \lfloor n / i \rfloor\f$ 的经典整除分块公式，复杂度 \f$O(\sqrt n)\f$。
 * @param n 正整数
 * @return \f$\sum_{i=1}^{n} \lfloor n / i \rfloor\f$
 */
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