#ifndef WBWLIB_CRYPTO_SM2_HPP
#define WBWLIB_CRYPTO_SM2_HPP

/**
 * @file sm2.hpp
 * @brief SM2 椭圆曲线公钥密码（GB/T 32918-2016）：签名/验签与加密/解密，基于 ECC 点运算 + SM3。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/core/random.hpp、wbwlib/misc/big-int.hpp、wbwlib/crypto/ecc.hpp、wbwlib/crypto/sm3.hpp
 *
 * @par 复杂度
 * 签名/解密各 1 次标量乘，验签 2 次标量乘（约 256 位域运算）。
 *
 * @par 示例
 * @code{.cpp}
 *   using SM2 = wbwlib::crypto::SM2;
 *   wbwlib::misc::BigInt d = SM2::keygen();            // 随机私钥
 *   auto pub = SM2::public_key(d);                     // 公钥点
 *   auto [r, s] = SM2::sign(d, "hello");               // 签名（默认 ID）
 *   bool ok = SM2::verify(pub, "hello", r, s);         // 验签
 *   std::string ct = SM2::encrypt(pub, "secret");      // 加密
 *   std::string pt = SM2::decrypt(d, ct);              // 解密
 * @endcode
 *
 * @attention 教学/演示用途；生产环境应使用经安全审查的实现（含随机源、侧信道防护）。
 */

#include <string>
#include <vector>
#include <cstring>
#include "wbwlib/core/base.hpp"
#include "wbwlib/core/random.hpp"
#include "wbwlib/misc/big-int.hpp"
#include "wbwlib/crypto/ecc.hpp"
#include "wbwlib/crypto/sm3.hpp"

namespace wbwlib {
namespace crypto {

using wbwlib::misc::BigInt;

namespace sm2detail {
/// 模 n 域运算（n 为基点阶，与曲线 p 不同）
inline BigInt n_add(const BigInt& a, const BigInt& b, const BigInt& n) { return (a % n + b % n) % n; }
inline BigInt n_sub(const BigInt& a, const BigInt& b, const BigInt& n) {
  BigInt r = (a % n - b % n) % n;
  return r < BigInt(0) ? r + n : r;
}
inline BigInt n_mul(const BigInt& a, const BigInt& b, const BigInt& n) { return (a % n) * (b % n) % n; }
inline BigInt n_inv(const BigInt& a, const BigInt& n) {
  // 扩展欧几里得求逆
  BigInt x, y, m = n;
  BigInt b = a % n;
  BigInt m0 = n, a0 = b, t1(0), t2(1);   // t1/t2: m0/a0 的 a 系数
  while (!a0.is_zero()) {
    BigInt q = m0 / a0;
    BigInt tmp = m0 - q * a0; m0 = a0; a0 = tmp;
    tmp = t1 - q * t2; t1 = t2; t2 = tmp;
  }
  BigInt r = t1 % n;
  return r < BigInt(0) ? r + n : r;
}
} // namespace sm2detail

/// SM2 曲线参数（sm2p256v1）
struct Sm2Curve {
  static BigInt P()  { return BigInt::from_hex("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"); }
  static BigInt A()  { return BigInt::from_hex("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC"); }
  static BigInt B()  { return BigInt::from_hex("28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93"); }
  static BigInt GX() { return BigInt::from_hex("32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7"); }
  static BigInt GY() { return BigInt::from_hex("BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0"); }
  static BigInt N()  { return BigInt::from_hex("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123"); }
};

/**
 * @brief SM2：签名/验签、加密/解密（GB/T 32918-2016）。
 */
class SM2 {
  using EC = ECC<Sm2Curve>;
  static const int PSZ = 32;   ///< 256 位曲线，坐标定长 32 字节

  /// 大整数 → 32 字节定长（高字节优先）
  static void big_to_bytes(const BigInt& v, u8 out[PSZ]) {
    std::string s = v.to_string();
    // 逐位十进制 → 二进制（借 BigInt 除法）
    BigInt x = v;
    for (int i = PSZ - 1; i >= 0; --i) {
      out[i] = (u8)x.mod_small(256);
      x = x / (i64)256;
    }
  }
  /// 32 字节定长 → 大整数
  static BigInt bytes_to_big(const u8* p) {
    BigInt r(0);
    for (int i = 0; i < PSZ; ++i) r = r * BigInt(256) + BigInt((i64)p[i]);
    return r;
  }

  /// 派生函数 KDF：SM3 计数器模式，输出 klen 字节
  static std::string kdf(const u8* z, size_t zlen, size_t klen) {
    std::string out;
    u32 ct = 1;
    u8 buf[2 * PSZ + 4];
    while (out.size() < klen) {
      std::memcpy(buf, z, zlen);
      buf[zlen] = (u8)(ct >> 24); buf[zlen + 1] = (u8)(ct >> 16);
      buf[zlen + 2] = (u8)(ct >> 8); buf[zlen + 3] = (u8)ct;
      u8 d[32];
      SM3::digest(buf, zlen + 4, d);
      out.append((const char*)d, 32);
      ++ct;
    }
    out.resize(klen);
    return out;
  }

  /// ZA 的字节形式（32 字节 SM3 摘要）
  static std::string za_bytes(const std::string& id, const BigInt& px, const BigInt& py) {
    u8 entl[2];
    size_t idlen = id.size() * 8;
    entl[0] = (u8)(idlen >> 8); entl[1] = (u8)idlen;
    u8 a[PSZ], b[PSZ], gx[PSZ], gy[PSZ], x[PSZ], y[PSZ];
    big_to_bytes(Sm2Curve::A(), a);
    big_to_bytes(Sm2Curve::B(), b);
    big_to_bytes(Sm2Curve::GX(), gx);
    big_to_bytes(Sm2Curve::GY(), gy);
    big_to_bytes(px, x);
    big_to_bytes(py, y);
    std::string msg((const char*)entl, 2);
    msg += id;
    msg.append((const char*)a, PSZ);
    msg.append((const char*)b, PSZ);
    msg.append((const char*)gx, PSZ);
    msg.append((const char*)gy, PSZ);
    msg.append((const char*)x, PSZ);
    msg.append((const char*)y, PSZ);
    u8 d[32];
    SM3::digest(msg.data(), msg.size(), d);
    return std::string((const char*)d, 32);
  }

 public:
  /// 默认标识
  static const char* DEFAULT_ID;

  /**
   * @brief 生成随机私钥。
   * @return \f$d \in [1, n-2]\f$
   */
  static BigInt keygen() {
    const BigInt& n = Sm2Curve::N();
    for (;;) {
      std::string s(78, '0');
      for (auto& c : s) c = (char)('0' + (int)wbwlib::core::rng.below(10));
      BigInt d = BigInt(s) % (n - BigInt(2)) + BigInt(1);
      if (d >= BigInt(1) && d <= n - BigInt(2)) return d;
    }
  }

  /// 私钥 → 公钥 \f$Q = d \cdot G\f$
  static EC::Point public_key(const BigInt& d) { return EC::mul(d, EC::base()); }

  /// 标量乘（调试/测试用）：\f$k \cdot G\f$
  static EC::Point mul_k(const BigInt& k) { return EC::mul(k, EC::base()); }

  /**
   * @brief 签名（e = SM3(ZA||M)）。
   * @param priv 私钥
   * @param msg  消息（任意字节）
   * @param id   标识（默认 DEFAULT_ID）
   * @return (r, s)，失败时 (0,0)
   */
  static std::pair<BigInt, BigInt> sign(const BigInt& priv, const std::string& msg,
                                        const std::string& id = std::string()) {
    const std::string& uid = id.empty() ? DEFAULT_ID : id;
    EC::Point q = public_key(priv);
    std::string za = za_bytes(uid, q.x, q.y);
    std::string em = za + msg;
    u8 h[32];
    SM3::digest(em.data(), em.size(), h);
    BigInt e = bytes_to_big(h);
    const BigInt& n = Sm2Curve::N();
    for (int attempt = 0; attempt < 64; ++attempt) {
      BigInt k = keygen();
      EC::Point p = EC::mul(k, EC::base());
      BigInt r = sm2detail::n_add(e, p.x % n, n);
      if (r.is_zero() || r > n - BigInt(1)) continue;
      BigInt denom = sm2detail::n_inv(BigInt(1) + priv, n);
      if (denom.is_zero()) continue;
      BigInt s = sm2detail::n_mul(denom, sm2detail::n_sub(k, r * priv, n), n);
      if (s.is_zero()) continue;
      return std::make_pair(r, s);
    }
    return std::make_pair(BigInt(0), BigInt(0));
  }

  /**
   * @brief 指定随机数 k 的签名（标准测试向量用）。
   * @param priv 私钥
   * @param msg  消息
   * @param k    固定随机数
   * @param id   标识（默认 DEFAULT_ID）
   * @return (r, s)
   */
  static std::pair<BigInt, BigInt> sign_k(const BigInt& priv, const std::string& msg, const BigInt& k,
                                          const std::string& id = std::string()) {
    const std::string& uid = id.empty() ? DEFAULT_ID : id;
    EC::Point q = public_key(priv);
    std::string za = za_bytes(uid, q.x, q.y);
    std::string em = za + msg;
    u8 h[32];
    SM3::digest(em.data(), em.size(), h);
    BigInt e = bytes_to_big(h);
    const BigInt& n = Sm2Curve::N();
    EC::Point p = EC::mul(k, EC::base());
    BigInt r = sm2detail::n_add(e, p.x % n, n);
    BigInt denom = sm2detail::n_inv(BigInt(1) + priv, n);
    BigInt s = sm2detail::n_mul(denom, sm2detail::n_sub(k, r * priv, n), n);
    return std::make_pair(r, s);
  }

  /**
   * @brief 验签。
   * @param pub 公钥点
   * @param msg 消息
   * @param r   签名分量
   * @param s   签名分量
   * @param id  标识（默认 DEFAULT_ID）
   * @return 是否有效
   */
  static bool verify(const EC::Point& pub, const std::string& msg,
                     const BigInt& r, const BigInt& s,
                     const std::string& id = std::string()) {
    const std::string& uid = id.empty() ? DEFAULT_ID : id;
    const BigInt& n = Sm2Curve::N();
    if (r < BigInt(1) || r > n - BigInt(1)) return false;
    if (s < BigInt(1) || s > n - BigInt(1)) return false;
    std::string za = za_bytes(uid, pub.x, pub.y);
    std::string em = za + msg;
    u8 h[32];
    SM3::digest(em.data(), em.size(), h);
    BigInt e = bytes_to_big(h);
    BigInt t = sm2detail::n_add(r, s, n);
    if (t.is_zero()) return false;
    EC::Point p = EC::add(EC::mul(s, EC::base()), EC::mul(t, pub));
    if (p.inf) return false;
    return sm2detail::n_add(e, p.x % n, n) == r;
  }

  /**
   * @brief 加密：\f$C = C1 \| C3 \| C2\f$（C1 = kG，C3 = SM3(x2||M||y2)，C2 = M⊕KDF(x2||y2)）。
   * @param pub 公钥点
   * @param msg 明文（任意字节）
   * @return 密文字节串
   */
  static std::string encrypt(const EC::Point& pub, const std::string& msg) {
    const BigInt& n = Sm2Curve::N();
    for (int attempt = 0; attempt < 64; ++attempt) {
      BigInt k = keygen();
      EC::Point c1 = EC::mul(k, EC::base());
      EC::Point s = EC::mul(k, pub);
      if (s.inf) continue;
      u8 x2[PSZ], y2[PSZ];
      big_to_bytes(s.x, x2);
      big_to_bytes(s.y, y2);
      std::string z((const char*)x2, PSZ);
      z.append((const char*)y2, PSZ);
      std::string t = kdf((const u8*)z.data(), z.size(), msg.size());
      bool all_zero = true;
      for (char c : t) if (c) { all_zero = false; break; }
      if (all_zero) continue;
      std::string c2 = msg;
      for (size_t i = 0; i < c2.size(); ++i) c2[i] ^= t[i];
      u8 xy[2 * PSZ];
      std::memcpy(xy, x2, PSZ);
      std::memcpy(xy + PSZ, y2, PSZ);
      std::string m3((const char*)xy, PSZ); m3 += msg; m3.append((const char*)xy + PSZ, PSZ);
      u8 c3[32];
      SM3::digest(m3.data(), m3.size(), c3);
      u8 c1b[PSZ * 2];
      big_to_bytes(c1.x, c1b);
      big_to_bytes(c1.y, c1b + PSZ);
      std::string out((const char*)c1b, PSZ * 2);
      out.append((const char*)c3, 32);
      out += c2;
      return out;
    }
    return std::string();
  }

  /**
   * @brief 解密（要求密文为 encrypt 的输出格式）。
   * @param priv 私钥
   * @param ct   密文
   * @return 明文；失败返回空串
   */
  static std::string decrypt(const BigInt& priv, const std::string& ct) {
    const size_t klen = ct.size() - 2 * PSZ - 32;
    if (ct.size() < 2 * PSZ + 32 + 1) return std::string();
    EC::Point c1(bytes_to_big((const u8*)ct.data()), bytes_to_big((const u8*)ct.data() + PSZ));
    EC::Point s = EC::mul(priv, c1);
    if (s.inf) return std::string();
    u8 x2[PSZ], y2[PSZ];
    big_to_bytes(s.x, x2);
    big_to_bytes(s.y, y2);
    std::string z((const char*)x2, PSZ);
    z.append((const char*)y2, PSZ);
    std::string t = kdf((const u8*)z.data(), z.size(), klen);
    std::string m = ct.substr(2 * PSZ + 32);
    for (size_t i = 0; i < m.size(); ++i) m[i] ^= t[i];
    std::string m3((const char*)x2, PSZ);
    m3 += m;
    m3.append((const char*)y2, PSZ);
    u8 c3[32];
    SM3::digest(m3.data(), m3.size(), c3);
    if (std::memcmp(c3, ct.data() + 2 * PSZ, 32) != 0) return std::string();
    return m;
  }
};

const char* SM2::DEFAULT_ID = "1234567812345678";

} // namespace crypto
} // namespace wbwlib

#endif // WBWLIB_CRYPTO_SM2_HPP