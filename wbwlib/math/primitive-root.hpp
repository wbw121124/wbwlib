#ifndef WBWLIB_MATH_PRIMITIVE_ROOT_HPP
#define WBWLIB_MATH_PRIMITIVE_ROOT_HPP

/**
 * @file primitive-root.hpp
 * @brief 原根求解：求模 p 意义下的最小原根（常用 NTT 前置，p 为质数）。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp, wbwlib/math/primes.hpp
 *
 * 原理：g 是模 p 的原根 ⇔ ord(g) = p-1 ⇔ 对所有 p-1 的质因子 q，g^((p-1)/q) != 1。
 * 复杂度：O(sqrt(p) · ω(p) · log p)，实际枚举 g 很小。
 */

#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"
#include "wbwlib/math/primes.hpp"

namespace wbwlib {
namespace math {

/// 求模 p（质数）的最小原根；p <= 2 时不存在，返回 -1
inline i64 primitive_root(i64 p) {
  if (p < 3) return -1;
  std::vector<u64> fs64 = factorize(p - 1);
  std::vector<i64> fs;
  for (u64 x : fs64) fs.push_back((i64)x);
  fs.erase(std::unique(fs.begin(), fs.end()), fs.end());  // 互不相同的质因子
  for (i64 g = 2; g < p; ++g) {
    bool ok = true;
    for (i64 q : fs)
      if (qpow(g, (p - 1) / q, p) == 1) { ok = false; break; }
    if (ok) return g;
  }
  return -1;
}

/// 模 p（质数）的原根个数 = phi(p-1)（用于理解，不验证）
inline i64 count_primitive_roots(i64 p) { return phi(p - 1); }

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_PRIMITIVE_ROOT_HPP