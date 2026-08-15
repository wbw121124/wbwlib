#ifndef WBWLIB_STR_ROLLING_HASH_HPP
#define WBWLIB_STR_ROLLING_HASH_HPP

/**
 * @file rolling-hash.hpp
 * @brief 字符串双哈希（取模 + 自然溢出可选）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：预处理 O(n)，区间哈希 O(1)。
 *
 * 用法：
 *   wbwlib::str::StringHash hs(s, B);
 *   if (hs.sub(l1, r1) == hs.sub(l2, r2)) ... // l,r 为 1 基
 */

#include <string>
#include <utility>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

struct HashVal {
  u64 a, b;                 ///< 两路哈希值（a: mod 1e9+7；b: mod 1e9+9）
  bool operator==(const HashVal& o) const { return a == o.a && b == o.b; }
  bool operator!=(const HashVal& o) const { return !(*this == o); }
};

class StringHash {
  static const u64 M1 = 1000000007ULL, M2 = 1000000009ULL;
  std::vector<u64> h1_, h2_, p1_, p2_;
  size_t n_;
  u64 base_;

 public:
  StringHash() {}
  /// 以 base 为基底构建哈希表（s 为普通字符串，下标视为字符码）
  StringHash(const std::string& s, u64 base = 131ULL) { build(s, base); }

  void build(const std::string& s, u64 base = 131ULL) {
    n_ = s.size(); base_ = base % M1;
    h1_.assign(n_ + 1, 0); h2_.assign(n_ + 1, 0);
    p1_.assign(n_ + 1, 1); p2_.assign(n_ + 1, 1);
    for (size_t i = 0; i < n_; ++i) {
      h1_[i + 1] = (h1_[i] * base_ + (u64)s[i]) % M1;
      h2_[i + 1] = (h2_[i] * base_ + (u64)s[i]) % M2;
      p1_[i + 1] = p1_[i] * base_ % M1;
      p2_[i + 1] = p2_[i] * base_ % M2;
    }
  }

  /// 子串 [l, r]（1 基闭区间）的哈希
  HashVal sub(int l, int r) const {
    u64 a = (h1_[r] + M1 - h1_[l - 1] * p1_[r - l + 1] % M1) % M1;
    u64 b = (h2_[r] + M2 - h2_[l - 1] * p2_[r - l + 1] % M2) % M2;
    return {a, b};
  }

  /// 拼接辅助：整个字符串哈希值
  HashVal all() const { return {h1_[n_], h2_[n_]}; }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_ROLLING_HASH_HPP