#ifndef WBWLIB_CRYPTO_RSA_HPP
#define WBWLIB_CRYPTO_RSA_HPP

/**
 * @file rsa.hpp
 * @brief RSA 公钥密码（基于 BigInt）：密钥生成、加密解密、签名验签。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/core/random.hpp、wbwlib/misc/big-int.hpp
 *
 * @par 复杂度
 * 加密/解密/签名一次模幂 O((bits/9)^2 · bits)（平方乘）；密钥生成需 ~2 次素性筛选（Miller-Rabin）。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::crypto::RSA rsa = wbwlib::crypto::RSA::generate(512);   // 512 位密钥
 *   auto c = rsa.encrypt(wbwlib::misc::BigInt("123456789"));        // 加密
 *   auto m = rsa.decrypt(c);                                        // 解密
 *   auto s = rsa.sign(m);   bool ok = rsa.verify(m, s);             // 签名/验签
 * @endcode
 *
 * @attention 教学/演示用途：生产环境应使用带 OAEP/PSS 填充与安全随机源的实现。
 */

#include <string>
#include "wbwlib/core/base.hpp"
#include "wbwlib/core/random.hpp"
#include "wbwlib/misc/big-int.hpp"

namespace wbwlib {
namespace crypto {

using wbwlib::misc::BigInt;

namespace rsadetail {

/// 模幂：\f$base^{exp} \bmod mod\f$（平方乘）
inline BigInt mod_pow(BigInt base, BigInt exp, const BigInt& mod) {
  base = base % mod;
  BigInt res(1);
  while (exp > BigInt(0)) {
    if (exp.mod_small(2)) res = (res * base) % mod;
    base = (base * base) % mod;
    exp = exp / (i64)2;
  }
  return res;
}

/// 最大公约数
inline BigInt gcd(const BigInt& a, const BigInt& b) {
  BigInt x = a, y = b;
  while (!y.is_zero()) { BigInt r = x % y; x = y; y = r; }
  return x < BigInt(0) ? -x : x;
}

/// 扩展欧几里得：\f$ax+by=gcd(a,b)\f$
inline BigInt egcd(BigInt a, BigInt b, BigInt& x, BigInt& y) {
  if (b.is_zero()) { x = BigInt(1); y = BigInt(0); return a; }
  BigInt q = a / b;
  BigInt g = egcd(b, a % b, y, x);
  y = y - q * x;
  return g;
}

/// 模逆：\f$a^{-1} \bmod m\f$（gcd=1 时有效，否则返回 0）
inline BigInt mod_inv(const BigInt& a, const BigInt& m) {
  BigInt x, y;
  BigInt g = egcd(a % m, m, x, y);
  if (!(g == BigInt(1))) return BigInt(0);
  BigInt r = x % m;
  return r < BigInt(0) ? r + m : r;
}

/// 随机 bits 位大整数（近似，十进制字构造；最低位强制为 1）
inline BigInt random_bits(int bits, wbwlib::core::Rand& r) {
  static const long double LOG10_2 = 0.30102999566398119521L;
  int nd = (int)((long double)bits * LOG10_2) + 1;   // 十进制位数
  std::string s(nd, '0');
  for (int i = 0; i < nd; ++i) s[i] = (char)('0' + (int)r.below(10));
  if (s[0] == '0') s[0] = '1';
  s[nd - 1] = (s[nd - 1] & (char)~1) | 1;            // 奇数
  return BigInt(s);
}

/// Miller-Rabin 素性测试（随机基）
inline bool miller_rabin(const BigInt& n, int rounds = 20) {
  if (n < BigInt(2)) return false;
  if (n == BigInt(2) || n == BigInt(3)) return true;
  if (!n.mod_small(2)) return false;
  BigInt d = n - BigInt(1);
  int s = 0;
  while (!d.mod_small(2)) { d = d / (i64)2; ++s; }
  for (int i = 0; i < rounds; ++i) {
    BigInt a = random_bits(60, wbwlib::core::rng);
    a = a % (n - BigInt(3)) + BigInt(2);             // [2, n-2]
    BigInt x = mod_pow(a, d, n);
    if (x == BigInt(1) || x == n - BigInt(1)) continue;
    bool comp = true;
    for (int r = 1; r < s; ++r) {
      x = (x * x) % n;
      if (x == n - BigInt(1)) { comp = false; break; }
    }
    if (comp) return false;
  }
  return true;
}

/// 随机素数（Miller-Rabin 筛选）
inline BigInt random_prime(int bits, int rounds = 20) {
  BigInt p;
  do { p = random_bits(bits, wbwlib::core::rng); } while (!miller_rabin(p, rounds));
  return p;
}

} // namespace rsadetail

/**
 * @brief RSA 密钥与加解密/签名。
 *
 * 公钥为 \f$(n, e)\f$，私钥为 \f$(n, d)\f$。消息必须小于 \f$n\f$。
 */
class RSA {
  BigInt n_, e_, d_;

 public:
  RSA() {}
  /**
   * @brief 由已有密钥构造。
   * @param n 模数
   * @param e 公钥指数
   * @param d 私钥指数
   */
  RSA(const BigInt& n, const BigInt& e, const BigInt& d) : n_(n), e_(e), d_(d) {}

  /**
   * @brief 生成 RSA 密钥对。
   * @param bits  模数位数（推荐 ≥ 1024）
   * @param pub_e 公钥指数（通常 65537）
   * @return 密钥对
   */
  static RSA generate(int bits = 1024, u64 pub_e = 65537) {
    int half = bits / 2;
    for (;;) {
      BigInt p = rsadetail::random_prime(half);
      BigInt q = rsadetail::random_prime(half);
      if (p == q) continue;
      BigInt n = p * q;
      BigInt phi = (p - BigInt(1)) * (q - BigInt(1));
      BigInt e(pub_e);
      if (rsadetail::gcd(e, phi) != BigInt(1)) continue;
      BigInt d = rsadetail::mod_inv(e, phi);
      if (d.is_zero()) continue;
      return RSA(n, e, d);
    }
  }

  /**
   * @brief 加密：\f$c = m^e \bmod n\f$。
   * @param m 明文（\f$0 \le m < n\f$）
   * @return 密文
   */
  BigInt encrypt(const BigInt& m) const { return rsadetail::mod_pow(m, e_, n_); }

  /**
   * @brief 解密：\f$m = c^d \bmod n\f$。
   * @param c 密文
   * @return 明文
   */
  BigInt decrypt(const BigInt& c) const { return rsadetail::mod_pow(c, d_, n_); }

  /**
   * @brief 签名：\f$s = m^d \bmod n\f$（裸 RSA，教学用）。
   * @param m 消息
   * @return 签名
   */
  BigInt sign(const BigInt& m) const { return rsadetail::mod_pow(m, d_, n_); }

  /**
   * @brief 验签：\f$s^e \equiv m \pmod n\f$。
   * @param m 消息
   * @param s 签名
   * @return 是否有效
   */
  bool verify(const BigInt& m, const BigInt& s) const {
    return rsadetail::mod_pow(s, e_, n_) == m;
  }

  /// 模数 \f$n\f$
  const BigInt& n() const { return n_; }
  /// 公钥指数 \f$e\f$
  const BigInt& e() const { return e_; }
  /// 私钥指数 \f$d\f$
  const BigInt& d() const { return d_; }
};

} // namespace crypto
} // namespace wbwlib

#endif // WBWLIB_CRYPTO_RSA_HPP