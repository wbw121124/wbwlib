#ifndef WBWLIB_STR_ROLLING_HASH_HPP
#define WBWLIB_STR_ROLLING_HASH_HPP

/**
 * @file rolling-hash.hpp
 * @brief 字符串双哈希（模板化双模数），取模 + 自然溢出可选。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 预处理 O(n)，子串哈希 O(1)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::StringHash<> hs(s, 131);            // 默认 1e9+7 / 1e9+9
 *   if (hs.sub(l1, r1) == hs.sub(l2, r2)) ...        // l,r 为 1 基
 *   wbwlib::str::StringHash<998244353ULL, 1000000007ULL> hs2(s);  // 自定义双模数
 * @endcode
 */

#include <string>
#include <utility>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief 双哈希值：两条路哈希以相等比较（a 路 mod MOD1，b 路 mod MOD2）。
 */
struct HashVal {
  u64 a, b;                 ///< 两条路哈希值
  /**
   * @brief 判断两个哈希值是否相等。
   * @param o 另一个哈希值
   * @return a 与 b 均相等返回 true
   */
  bool operator==(const HashVal& o) const { return a == o.a && b == o.b; }
  /**
   * @brief 判断两个哈希值是否不等。
   * @param o 另一个哈希值
   * @return a 或 b 任一路不等返回 true
   */
  bool operator!=(const HashVal& o) const { return !(*this == o); }
};

namespace hashingdetail {

/// 任意模数安全乘法：$ (a cdot b) mod m $（__int128 中间值，任意 64 位模数可用）
inline u64 mulmod(u64 a, u64 b, u64 m) { return (u64)((__int128)a * b % m); }

} // namespace hashingdetail

/**
 * @brief 字符串双哈希：模数模板化（默认 1e9+7 / 1e9+9），预处理 O(n) 后子串哈希 O(1)。
 * @tparam MOD1 第一路模数（默认 1000000007）
 * @tparam MOD2 第二路模数（默认 1000000009）
 */
template<u64 MOD1 = 1000000007ULL, u64 MOD2 = 1000000009ULL>
class StringHash {
  std::vector<u64> h1_, h2_, p1_, p2_;
  size_t n_;
  u64 base_;

 public:
  /// 构造空哈希，需再调用 build。
  StringHash() {}
  /**
   * @brief 以 base 为基底构造哈希（s 下标按字符码）。
   * @param s 待哈希字符串
   * @param base 哈希基底（默认 131）
   */
  StringHash(const std::string& s, u64 base = 131ULL) { build(s, base); }

  /**
   * @brief 构建哈希前缀与基底幂表：\f$h[i] = (h[i-1]\cdot base + s[i-1]) \bmod M\f$。
   * @param s 待哈希字符串
   * @param base 哈希基底（默认 131）
   */
  void build(const std::string& s, u64 base = 131ULL) {
    n_ = s.size(); base_ = base % MOD1;
    h1_.assign(n_ + 1, 0); h2_.assign(n_ + 1, 0);
    p1_.assign(n_ + 1, 1); p2_.assign(n_ + 1, 1);
    for (size_t i = 0; i < n_; ++i) {
      h1_[i + 1] = (hashingdetail::mulmod(h1_[i], base_, MOD1) + (u64)s[i]) % MOD1;
      h2_[i + 1] = (hashingdetail::mulmod(h2_[i], base_, MOD2) + (u64)s[i]) % MOD2;
      p1_[i + 1] = hashingdetail::mulmod(p1_[i], base_, MOD1);
      p2_[i + 1] = hashingdetail::mulmod(p2_[i], base_, MOD2);
    }
  }

  /**
   * @brief 子串 [l, r]（1 基闭区间）的双哈希：\f$h([l,r]) = h(r) - h(l-1) \cdot base^{r-l+1} \pmod{M}\f$。
   * @param l 区间左端点（1 基）
   * @param r 区间右端点（1 基）
   * @return 子串双哈希值
   */
  HashVal sub(int l, int r) const {
    u64 a = (h1_[r] + MOD1 - hashingdetail::mulmod(h1_[l - 1], p1_[r - l + 1], MOD1)) % MOD1;
    u64 b = (h2_[r] + MOD2 - hashingdetail::mulmod(h2_[l - 1], p2_[r - l + 1], MOD2)) % MOD2;
    return {a, b};
  }

  /**
   * @brief 整个字符串的哈希值（拼接子串哈希时用）。
   * @return 整串双哈希值
   */
  HashVal all() const { return {h1_[n_], h2_[n_]}; }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_ROLLING_HASH_HPP
