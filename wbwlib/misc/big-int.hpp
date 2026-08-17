#ifndef WBWLIB_MISC_BIG_INT_HPP
#define WBWLIB_MISC_BIG_INT_HPP

/**
 * @file big-int.hpp
 * @brief 高精度整数：十进制 1e9 基（符号 + 幅值）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 设施：构造（字符串/i64/拷贝）、比较、+ - * / %（被 BigInt 或 i64）、
 *       幂 pow、to_string、流输出。常规竞赛规模（≤ 1e5 位以内）够用。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::misc::BigInt a("12345678901234567890"), b(12345);
 *   auto c = a * b;
 *   std::cout << c / b << "\n";
 * @endcode
 */

#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <ostream>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace misc {

using ll = wbwlib::i64;   ///< 本文件内部别名
using ull = wbwlib::u64;

/**
 * @brief 高精度整数：符号 + 幅值，十进制 \f$10^9\f$ 基（每 9 位一组，小端存储）。
 */
class BigInt {
  static const int BASE = 1000000000;
  static const int WIDTH = 9;

  std::vector<int> d;   ///< 小端：d[0] 为低位
  bool neg;

  /// 去除高位前导零；零统一为非负
  void trim() {
    while (d.size() > 1 && d.back() == 0) d.pop_back();
    if (d.size() == 1 && d[0] == 0) neg = false;
  }

  /// 幅值比较（忽略符号）：本数大返回 +1，小返回 -1，相等返回 0
  int abs_cmp(const BigInt& o) const {
    if (d.size() != o.d.size()) return d.size() < o.d.size() ? -1 : 1;
    for (int i = (int)d.size() - 1; i >= 0; --i)
      if (d[i] != o.d[i]) return d[i] < o.d[i] ? -1 : 1;
    return 0;
  }

  /// 从十进制字符串解析（支持 + / - 号与前导零）
  void parse(const std::string& s) {
    neg = false;
    d.clear();
    int n = (int)s.size(), p = 0;
    if (p < n && (s[p] == '-' || s[p] == '+')) { neg = (s[p] == '-'); ++p; }
    int i = n;
    while (i > p) {
      int l = std::max(p, i - WIDTH);
      int v = 0;
      for (int k = l; k < i; ++k) v = v * 10 + (s[k] - '0');
      d.push_back(v);
      i = l;
    }
    if (d.empty()) d.push_back(0);
    trim();
  }

 public:
  /**
   * @brief 默认构造：值 0。
   */
  BigInt() : neg(false) { d.push_back(0); }
  /**
   * @brief 从 i64 构造。
   * @param v 输入值。
   */
  BigInt(i64 v) {
    neg = (v < 0);
    u64 x = neg ? (u64)(-(v + 1)) + 1 : (u64)v;
    d.clear();
    if (x == 0) d.push_back(0);
    while (x) { d.push_back((int)(x % BASE)); x /= BASE; }
    trim();
  }
  /**
   * @brief 从十进制字符串构造。
   * @param s 十进制数字串，可含 + / - 号与前导零。
   */
  BigInt(const std::string& s) { parse(s); }

  /**
   * @brief 从十六进制字符串构造（可含前导零，大小写均可；负号不支持，前缀 0x 会自动跳过）。
   * @param s 十六进制数字串。
   * @return 对应的大整数。
   */
  static BigInt from_hex(const std::string& s) {
    std::string dec = "0";
    size_t p = 0;
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) p = 2;
    for (; p < s.size(); ++p) {
      char ch = s[p];
      int v = (ch >= '0' && ch <= '9') ? ch - '0'
            : (ch >= 'a' && ch <= 'f') ? ch - 'a' + 10
            : (ch >= 'A' && ch <= 'F') ? ch - 'A' + 10 : 0;
      int carry = v;
      for (int i = (int)dec.size() - 1; i >= 0; --i) {
        int t = (dec[i] - '0') * 16 + carry;
        dec[i] = (char)('0' + t % 10);
        carry = t / 10;
      }
      while (carry) { dec.insert(dec.begin(), (char)('0' + carry % 10)); carry /= 10; }
    }
    return BigInt(dec);
  }

  /**
   * @brief 取负。
   * @return 相反数；0 取负仍为 0。
   */
  BigInt operator-() const {
    BigInt r = *this;
    if (!is_zero()) r.neg = !neg;
    return r;
  }

  /// 判断是否为零
  bool is_zero() const { return d.size() == 1 && d[0] == 0; }

  // ---- 比较 ----
  /**
   * @brief 相等比较。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return 两数数值相等时返回 true。
   */
  friend bool operator==(const BigInt& a, const BigInt& b) {
    return a.neg == b.neg && a.d == b.d;
  }
  /**
   * @brief 不等比较。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return 两数数值不等时返回 true。
   */
  friend bool operator!=(const BigInt& a, const BigInt& b) { return !(a == b); }
  /**
   * @brief 小于比较。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return a < b 时返回 true。
   */
  friend bool operator<(const BigInt& a, const BigInt& b) {
    if (a.neg != b.neg) return a.neg;
    int c = a.abs_cmp(b);
    return a.neg ? c > 0 : c < 0;
  }
  /**
   * @brief 大于比较。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return a > b 时返回 true。
   */
  friend bool operator>(const BigInt& a, const BigInt& b) { return b < a; }
  /**
   * @brief 小于等于比较。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return a <= b 时返回 true。
   */
  friend bool operator<=(const BigInt& a, const BigInt& b) { return !(b < a); }
  /**
   * @brief 大于等于比较。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return a >= b 时返回 true。
   */
  friend bool operator>=(const BigInt& a, const BigInt& b) { return !(a < b); }

  // ---- 加/减（幅值） ----
  /**
   * @brief 加法。
   * @param o 加数。
   * @return 两数之和。
   */
  BigInt operator+(const BigInt& o) const {
    if (neg == o.neg) {
      BigInt r;
      r.neg = neg;
      r.d.resize(std::max(d.size(), o.d.size()));
      ull carry = 0;
      for (size_t i = 0; i < r.d.size(); ++i) {
        ull s = carry;
        if (i < d.size()) s += d[i];
        if (i < o.d.size()) s += o.d[i];
        r.d[i] = (int)(s % BASE);
        carry = s / BASE;
      }
      while (carry) { r.d.push_back((int)(carry % BASE)); carry /= BASE; }
      r.trim();
      return r;
    }
    // 异号 → 差
    const BigInt* bigger; const BigInt* smaller;
    if (abs_cmp(o) >= 0) { bigger = this; smaller = &o; }
    else { bigger = &o; smaller = this; }
    BigInt r;
    r.neg = bigger->neg;
    r.d.resize(bigger->d.size());
    int borrow = 0;
    for (size_t i = 0; i < bigger->d.size(); ++i) {
      ll t = (ll)bigger->d[i] - borrow;
      if (i < smaller->d.size()) t -= smaller->d[i];
      if (t < 0) { t += BASE; borrow = 1; } else borrow = 0;
      r.d[i] = (int)t;
    }
    r.trim();
    return r;
  }

  /**
   * @brief 减法。
   * @param o 减数。
   * @return 两数之差。
   */
  BigInt operator-(const BigInt& o) const { return *this + (-o); }

  // ---- 乘法 ----
  /**
   * @brief 乘法。
   * @param o 乘数。
   * @return 两数之积。
   */
  BigInt operator*(const BigInt& o) const {
    BigInt r;
    r.neg = neg ^ o.neg;
    r.d.assign(d.size() + o.d.size(), 0);
    for (size_t i = 0; i < d.size(); ++i) {
      ull carry = 0;
      for (size_t j = 0; j < o.d.size(); ++j) {
        ull cur = r.d[i + j] + (ull)d[i] * o.d[j] + carry;
        r.d[i + j] = (int)(cur % BASE);
        carry = cur / BASE;
      }
      r.d[i + o.d.size()] += (int)carry;
    }
    r.trim();
    return r;
  }

  // ---- 除以/模 i64 ----
  /**
   * @brief 除以 i64。
   * @param v 除数（非零）。
   * @return 商。
   */
  BigInt operator/(i64 v) const;       // 外部定义见下（就地定义更简）
  /**
   * @brief 对 i64 取模。
   * @param v 模数（非零）。
   * @return 余数（符号与被除数一致）。
   */
  BigInt operator%(i64 v) const;
  /**
   * @brief 对 i64 取模（直接逐位计算余数，不构造 BigInt）。
   * @param v 模数（非零）。
   * @return 余数（符号与被除数一致）。
   */
  i64 mod_small(i64 v) const;

  // ---- 除以/模 BigInt（二分商 + 乘法），幅值除法 ----
  /**
   * @brief 除以 BigInt（对幅值二分求商）。
   * @param o 除数（非零）。
   * @return 商。
   */
  BigInt operator/(const BigInt& o) const;
  /**
   * @brief 对 BigInt 取模。
   * @param o 模数（非零）。
   * @return 余数。
   */
  BigInt operator%(const BigInt& o) const;

  // ---- 幂 ----
  /**
   * @brief 快速幂。
   * @param e 指数（非负）。
   * @return 本数的 e 次方；负底数且 e 为奇数时结果为负，0 的非零次方为 0。
   */
  BigInt pow(i64 e) const {
    bool negpow = neg && (e & 1);      // 负底数且指数为奇数 → 结果为负
    BigInt base = *this, res(1);
    base.neg = false;                  // 幅值快速幂，符号最后决定
    while (e > 0) {
      if (e & 1) res = res * base;
      base = base * base;
      e >>= 1;
    }
    if (!res.is_zero()) res.neg = negpow;
    return res;
  }

  /**
   * @brief 转为十进制字符串。
   * @return 十进制表示：零为 "0"，负数带 '-' 前缀。
   */
  std::string to_string() const {
    if (is_zero()) return "0";
    std::string s;
    if (neg) s += '-';
    char buf[32];
    s += std::to_string(d.back());
    for (int i = (int)d.size() - 2; i >= 0; --i) {
      std::snprintf(buf, sizeof buf, "%09d", d[i]);
      s += buf;
    }
    return s;
  }

  /**
   * @brief 流输出。
   * @param os 输出流。
   * @param b 待输出的大整数。
   * @return 输出流引用。
   */
  friend std::ostream& operator<<(std::ostream& os, const BigInt& b) {
    return os << b.to_string();
  }
};

// ---- i64 除法 ----
inline BigInt BigInt::operator/(i64 v) const {
  bool nsign = neg ^ (v < 0);
  u64 b = v < 0 ? (u64)(-(v + 1)) + 1 : (u64)v;
  BigInt r;
  r.neg = nsign;
  if (!is_zero() && nsign) r.neg = true;
  r.d.assign(d.size(), 0);
  ull rem = 0;
  for (int i = (int)d.size() - 1; i >= 0; --i) {
    ull cur = rem * BASE + (u64)d[i];
    r.d[i] = (int)(cur / b);
    rem = cur % b;
  }
  r.trim();
  return r;
}

inline i64 BigInt::mod_small(i64 v) const {
  u64 b = v < 0 ? (u64)(-(v + 1)) + 1 : (u64)v;
  ull rem = 0;
  for (int i = (int)d.size() - 1; i >= 0; --i)
    rem = (rem * BASE + (u64)d[i]) % b;
  i64 re = (i64)rem;
  if (neg) re = -re;
  return re;
}

inline BigInt BigInt::operator%(i64 v) const {
  BigInt r(mod_small(v));
  return r;
}

// ---- BigInt 除法：绝对值商二分 ----
inline BigInt BigInt::operator/(const BigInt& o) const {
  if (o.is_zero()) return BigInt();      // 除以 0：返回 0（不抛，OI 场景由用户保证）
  bool nsign = neg ^ o.neg;
  const BigInt& a = *this;
  const BigInt& b = o;                    // 改成 abs
  // 构造幅值 a、b
  BigInt aa = a, bb = b;
  aa.neg = bb.neg = false;
  if (aa.abs_cmp(bb) < 0) return BigInt();   // |a| < |b|
  // 二分答案：商位数为 |a| - |b| + 1
  int n = (int)aa.d.size() - (int)bb.d.size() + 1;
  BigInt lo(0), hi(1), mid;
  hi.d.assign(n + 1, 0);
  hi.d[n] = 1;                                // 10^n（BASE^n）
  // 二分最大 x 使 bb*x <= aa
  while (lo + (i64)1 < hi) {
    mid = (lo + hi) / (i64)2;
    if ((bb * mid).abs_cmp(aa) <= 0) lo = mid;
    else hi = mid;
  }
  BigInt q = lo;
  q.neg = nsign && !lo.is_zero();
  q.trim();
  return q;
}

// ---- 幅值取模 ----
inline BigInt BigInt::operator%(const BigInt& o) const {
  if (o.is_zero()) return BigInt();
  BigInt q = *this / o;
  BigInt r = *this - q * o;
  return r;
}

} // namespace misc
} // namespace wbwlib

#endif // WBWLIB_MISC_BIG_INT_HPP