#ifndef WBWLIB_MATH_BSGS_HPP
#define WBWLIB_MATH_BSGS_HPP

/**
 * @file bsgs.hpp
 * @brief 大步小步法 Baby-Step-Giant-Step：解离散对数 a^x ≡ b (mod p)。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp
 * 复杂度：O(sqrt(p)) 时间与空间（哈希表）。
 *
 * 要求 gcd(a, p) == 1（返回 -1 表示无解）。求最小非负 x。
 *
 * 用法：
 *   long long x = wbwlib::math::bsgs(a, b, p);
 */

#include <cmath>
#include <unordered_map>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"

namespace wbwlib {
namespace math {

inline i64 bsgs(i64 a, i64 b, i64 p) {
  a %= p; b %= p;
  if (b == 1) return 0;               // a^0 ≡ 1
  if (a == 0) return b == 0 ? 1 : -1; // a^x ≡ 0 (mod p) 仅当 b=0 且 x>=1
  i64 m = (i64)std::ceil(std::sqrt((double)p));
  // 小步：记录 a^j 的 j（b * a^j -> j）
  std::unordered_map<i64, i64> mp;
  mp.reserve(m + 5);
  i64 base = 1;                       // a^0
  for (i64 j = 0; j < m; ++j) {
    i64 key = mul_mod(base, b, p);
    if (mp.find(key) == mp.end()) mp[key] = j;
    base = mul_mod(base, a, p);
  }
  // 大步：枚举 i，求 a^{i*m}
  i64 am = mul_mod(base, i64(1), p);   // 此时 base == a^m
  i64 cur = am;
  for (i64 i = 1; i <= m; ++i) {
    auto it = mp.find(cur);
    if (it != mp.end()) return i * m - it->second;
    cur = mul_mod(cur, am, p);
  }
  return -1;                          // 无解
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_BSGS_HPP