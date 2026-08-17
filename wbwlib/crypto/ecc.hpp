#ifndef WBWLIB_CRYPTO_ECC_HPP
#define WBWLIB_CRYPTO_ECC_HPP

/**
 * @file ecc.hpp
 * @brief 椭圆曲线密码（ECC）：有限域点运算、标量乘、ECDSA 签名/验签（曲线模板化）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/core/random.hpp、wbwlib/misc/big-int.hpp
 *
 * @par 复杂度
 * 点加/倍点 O((bits/9)^2)；标量乘 O(bits) 次点运算；ECDSA 签名/验签各 1~2 次标量乘。
 *
 * @par 示例
 * @code{.cpp}
 *   using EC = wbwlib::crypto::ECC<wbwlib::crypto::Secp256k1>;
 *   EC ec;
 *   EC::Point g = ec.base();                       // 基点 G
 *   wbwlib::misc::BigInt d("1c2...");              // 私钥
 *   EC::Point q = ec.mul(d, g);                    // 公钥 Q = d*G
 *   auto [r, s] = ec.sign(d, z);                   // 对消息摘要 z 签名
 *   bool ok = ec.verify(q, z, r, s);               // 验签
 * @endcode
 *
 * @attention 教学/演示用途；生产环境需侧信道防护与安全随机数。
 */

#include "wbwlib/core/base.hpp"
#include "wbwlib/core/random.hpp"
#include "wbwlib/misc/big-int.hpp"

namespace wbwlib {
namespace crypto {

using wbwlib::misc::BigInt;

namespace eccdetail {
/// 模幂（复用，独立实现避免依赖 rsa.hpp）
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
/// 模逆（扩展欧几里得，\f$a^{-1} \bmod p\f$）
inline BigInt inv(const BigInt& a, const BigInt& p) {
  BigInt m0 = p, a0 = a % p, t1(0), t2(1);   // t1/t2: m0/a0 的 a 系数
  while (!a0.is_zero()) {
    BigInt q = m0 / a0;
    BigInt tmp = m0 - q * a0;
    m0 = a0;
    a0 = tmp;
    tmp = t1 - q * t2;
    t1 = t2;
    t2 = tmp;
  }
  BigInt r = t1 % p;
  return r < BigInt(0) ? r + p : r;
}
} // namespace eccdetail

/**
 * @brief 椭圆曲线参数（模板定制点）：提供 P/A/B/GX/GY/N 静态函数返回 BigInt。
 */
struct Secp256k1 {
  /// 素数 \f$p = 2^{256}-2^{32}-977\f$
  static BigInt P()  { return BigInt::from_hex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"); }
  /// 系数 a
  static BigInt A()  { return BigInt(0); }
  /// 系数 b
  static BigInt B()  { return BigInt(7); }
  /// 基点 x 坐标
  static BigInt GX() { return BigInt::from_hex("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798"); }
  /// 基点 y 坐标
  static BigInt GY() { return BigInt::from_hex("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8"); }
  /// 基点阶 \f$n\f$
  static BigInt N()  { return BigInt::from_hex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141"); }
};

/**
 * @brief 椭圆曲线域上的运算与 ECDSA。
 * @tparam Curve 曲线参数（提供 P/A/B/GX/GY/N 静态函数），默认 secp256k1
 */
template<class Curve = Secp256k1>
class ECC {
 public:
  /// 曲线上点（inf=true 表示无穷远点）
  struct Point {
    BigInt x, y;
    bool inf;
    Point() : x(0), y(0), inf(true) {}
    Point(const BigInt& x_, const BigInt& y_) : x(x_), y(y_), inf(false) {}
    bool operator==(const Point& o) const {
      if (inf || o.inf) return inf && o.inf;
      return x == o.x && y == o.y;
    }
  };

 private:
  static BigInt pm(const BigInt& v) { return v % Curve::P(); }
  static BigInt padd(const BigInt& a, const BigInt& b) {
    BigInt r = (a % Curve::P() + b % Curve::P()) % Curve::P();
    return r < BigInt(0) ? r + Curve::P() : r;
  }
  static BigInt psub(const BigInt& a, const BigInt& b) { return padd(a, -b); }
  static BigInt pmul(const BigInt& a, const BigInt& b) { return (a % Curve::P()) * (b % Curve::P()) % Curve::P(); }
  static BigInt pneg(const BigInt& a) { return a.is_zero() ? BigInt(0) : Curve::P() - a; }

  /// 点加（含倍点与无穷远点）
  static Point point_add(const Point& a, const Point& b) {
    if (a.inf) return b;
    if (b.inf) return a;
    if (a == b) {
      if (a.y.is_zero()) return Point();
      BigInt lam = pmul(pmul(BigInt(3), pmul(a.x, a.x)) + Curve::A(), eccdetail::inv(pmul(BigInt(2), a.y), Curve::P()));
      BigInt x3 = psub(psub(pmul(lam, lam), a.x), a.x);
      BigInt y3 = psub(pmul(lam, psub(a.x, x3)), a.y);
      return Point(x3, y3);
    }
    if (a.x == b.x) return Point();          // 垂直 → 无穷远
    BigInt lam = pmul(psub(b.y, a.y), eccdetail::inv(psub(b.x, a.x), Curve::P()));
    BigInt x3 = psub(psub(pmul(lam, lam), a.x), b.x);
    BigInt y3 = psub(pmul(lam, psub(a.x, x3)), a.y);
    return Point(x3, y3);
  }

 public:
  /// 基点
  static Point base() { return Point(Curve::GX(), Curve::GY()); }

  /// 点加（含倍点与无穷远点）
  static Point add(const Point& a, const Point& b) { return point_add(a, b); }

  /**
   * @brief 标量乘：\f$k \cdot P\f$（double-and-add，从高位）。
   * @param k 标量（非负）
   * @param p 点
   * @return 结果点
   */
  static Point mul(BigInt k, const Point& p) {
    Point r;                                  // 无穷远
    Point b = p;
    while (k > BigInt(0)) {
      if (k.mod_small(2)) r = add(r, b);
      b = add(b, b);
      k = k / (i64)2;
    }
    return r;
  }

  /**
   * @brief ECDSA 签名。
   * @param priv 私钥 \f$d \in [1, n-1]\f$
   * @param z    消息摘要（截断到 n 的位长）
   * @return (r, s)，若失败返回 (0, 0)
   */
  static std::pair<BigInt, BigInt> sign(const BigInt& priv, BigInt z) {
    const BigInt& n = Curve::N();
    z = z % n;
    for (int attempt = 0; attempt < 64; ++attempt) {
      BigInt k = random_k(n);
      if (k.is_zero()) continue;
      Point p = mul(k, base());
      BigInt r = p.x % n;
      if (r.is_zero()) continue;
      BigInt s = pmul_any(eccdetail::inv(k, n), padd_any(z, pmul_any(r, priv, n), n), n);
      if (s.is_zero()) continue;
      return std::make_pair(r, s);
    }
    return std::make_pair(BigInt(0), BigInt(0));
  }

  /**
   * @brief ECDSA 验签。
   * @param pub 公钥点 \f$Q\f$
   * @param z   消息摘要
   * @param r   签名分量
   * @param s   签名分量
   * @return 是否有效
   */
  static bool verify(const Point& pub, BigInt z, const BigInt& r, const BigInt& s) {
    const BigInt& n = Curve::N();
    if (r < BigInt(1) || r > n - BigInt(1)) return false;
    if (s < BigInt(1) || s > n - BigInt(1)) return false;
    z = z % n;
    BigInt w = eccdetail::inv(s, n);
    BigInt u1 = pmul_any(z, w, n);
    BigInt u2 = pmul_any(r, w, n);
    Point p = add(mul(u1, base()), mul(u2, pub));
    if (p.inf) return false;
    return (p.x % n) == (r % n);
  }

 private:
  static BigInt random_k(const BigInt& n) {
    // 生成 ≤ 2*位长 的随机数再取模
    std::string s(78, '0');                    // 约 256 位 → 78 位十进制
    for (auto& c : s) c = (char)('0' + (int)wbwlib::core::rng.below(10));
    BigInt k = BigInt(s) % (n - BigInt(1)) + BigInt(1);
    return k;
  }
  /// 模 n 运算辅助
  static BigInt pmul_any(const BigInt& a, const BigInt& b, const BigInt& m) {
    return (a % m) * (b % m) % m;
  }
  static BigInt padd_any(const BigInt& a, const BigInt& b, const BigInt& m) {
    return (a % m + b % m) % m;
  }
};

} // namespace crypto
} // namespace wbwlib

#endif // WBWLIB_CRYPTO_ECC_HPP