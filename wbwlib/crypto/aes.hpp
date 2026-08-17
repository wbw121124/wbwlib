#ifndef WBWLIB_CRYPTO_AES_HPP
#define WBWLIB_CRYPTO_AES_HPP

/**
 * @file aes.hpp
 * @brief AES 分组密码（FIPS-197）：AES-128/192/256 模板化，块级 + ECB 模式。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 单块加密/解密 O(1)：AES-128 共 10 轮，AES-192 12 轮，AES-256 14 轮。
 *
 * @par 示例
 * @code{.cpp}
 *   u8 key[16] = {0x00,0x01,...};        // AES-128
 *   wbwlib::crypto::AES<4> aes(key);      // Nk=4 → AES-128；Nk=6 → AES-192；Nk=8 → AES-256
 *   u8 in[16] = {...}, out[16];
 *   aes.encrypt_block(in, out);
 *   aes.decrypt_block(out, in2);
 * @endcode
 *
 * @attention 仅块级加密；多块数据需自行选择模式（ECB 已提供，CBC/CTR 可基于块接口扩展）。
 */

#include <cstring>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace crypto {

namespace aesdetail {

/// GF(2^8) 中乘 2（xtime）
inline u8 xtime(u8 a) { return (u8)((a << 1) ^ (a & 0x80 ? 0x1b : 0)); }
/// GF(2^8) 多项式乘法
inline u8 gmul(u8 a, u8 b) {
  u8 r = 0;
  while (b) {
    if (b & 1) r ^= a;
    a = xtime(a);
    b >>= 1;
  }
  return r;
}
/// 逐字节求逆（x^254）
inline u8 ginv(u8 x) {
  u8 r = 1;
  for (int i = 0; i < 254; ++i) r = gmul(r, x);
  return r;
}
/// 循环左移 1 位
inline u8 rotl8(u8 b) { return (u8)((b << 1) | (b >> 7)); }
/// S-box 仿射变换
inline u8 sbox_affine(u8 b) {
  return (u8)(b ^ rotl8(b) ^ rotl8(rotl8(b)) ^ rotl8(rotl8(rotl8(b))) ^
              rotl8(rotl8(rotl8(rotl8(b)))) ^ 0x63);
}

/// 生成 S-box 与逆 S-box（运行时一次计算）
struct SBox {
  u8 f[256], inv[256];
  SBox() {
    f[0] = 0x63;
    for (int i = 1; i < 256; ++i) f[i] = sbox_affine(ginv((u8)i));
    for (int i = 0; i < 256; ++i) inv[f[i]] = (u8)i;
  }
};
/// S-box 单例（函数内静态，避免静态初始化顺序问题）
inline const SBox& sbox() {
  static const SBox sb;
  return sb;
}

} // namespace aesdetail

/**
 * @brief AES 分组密码：密钥长度模板化。
 * @tparam Nk 密钥字数（4 = AES-128，6 = AES-192，8 = AES-256）
 */
template<int Nk = 4>
class AES {
  static const int Nb = 4;          ///< 分组为 4 字（128 位）
  static const int Nr = Nk + 6;     ///< 轮数
  u8 w_[4 * Nb * (Nr + 1)];         ///< 扩展密钥（按字节存）

  static u32 rot_word(u32 x) { return (x << 8) | (x >> 24); }
  static u32 sub_word(u32 x) {
    const auto& sb = aesdetail::sbox();
    return ((u32)sb.f[x >> 24] << 24) | ((u32)sb.f[(x >> 16) & 0xff] << 16) |
           ((u32)sb.f[(x >> 8) & 0xff] << 8) | (u32)sb.f[x & 0xff];
  }
  static u8 rcon(int i) {           // 轮常量：0x01,0x02,0x04,...（GF 乘 2 递推）
    u8 r = 1;
    for (int k = 1; k < i; ++k) r = aesdetail::xtime(r);
    return r;
  }
  void key_expand(const u8 key[4 * Nk]) {
    for (int i = 0; i < 4 * Nk; ++i) w_[i] = key[i];
    for (int i = Nk; i < Nb * (Nr + 1); ++i) {
      u32 t = (u32)w_[4 * (i - 1)] << 24 | (u32)w_[4 * (i - 1) + 1] << 16 |
              (u32)w_[4 * (i - 1) + 2] << 8 | w_[4 * (i - 1) + 3];
      if (i % Nk == 0) t = sub_word(rot_word(t)) ^ ((u32)rcon(i / Nk) << 24);
      else if (Nk > 6 && i % Nk == 4) t = sub_word(t);
      for (int k = 0; k < 4; ++k) {
        u8 prev = w_[4 * (i - Nk) + k];
        w_[4 * i + k] = (u8)(prev ^ (t >> (24 - 8 * k)));
      }
    }
  }

  void add_round_key(u8 s[16], int round) const {
    for (int i = 0; i < 16; ++i) s[i] ^= w_[16 * round + i];
  }
  static void sub_bytes(u8 s[16]) {
    const auto& sb = aesdetail::sbox();
    for (int i = 0; i < 16; ++i) s[i] = sb.f[s[i]];
  }
  static void inv_sub_bytes(u8 s[16]) {
    const auto& sb = aesdetail::sbox();
    for (int i = 0; i < 16; ++i) s[i] = sb.inv[s[i]];
  }
  static void shift_rows(u8 s[16]) {
    u8 t[16];
    std::memcpy(t, s, 16);
    for (int r = 1; r < 4; ++r)
      for (int c = 0; c < 4; ++c) s[4 * c + r] = t[4 * ((c + r) % 4) + r];
  }
  static void inv_shift_rows(u8 s[16]) {
    u8 t[16];
    std::memcpy(t, s, 16);
    for (int r = 1; r < 4; ++r)
      for (int c = 0; c < 4; ++c) s[4 * c + r] = t[4 * ((c - r + 4) % 4) + r];
  }
  static u8 mul2(u8 b) { return aesdetail::xtime(b); }
  static u8 mul3(u8 b) { return (u8)(aesdetail::xtime(b) ^ b); }
  static void mix_columns(u8 s[16]) {
    for (int c = 0; c < 4; ++c) {
      u8 a[4] = {s[4 * c], s[4 * c + 1], s[4 * c + 2], s[4 * c + 3]};
      s[4 * c]     = (u8)(mul2(a[0]) ^ mul3(a[1]) ^ a[2] ^ a[3]);
      s[4 * c + 1] = (u8)(a[0] ^ mul2(a[1]) ^ mul3(a[2]) ^ a[3]);
      s[4 * c + 2] = (u8)(a[0] ^ a[1] ^ mul2(a[2]) ^ mul3(a[3]));
      s[4 * c + 3] = (u8)(mul3(a[0]) ^ a[1] ^ a[2] ^ mul2(a[3]));
    }
  }
  static void inv_mix_columns(u8 s[16]) {
    for (int c = 0; c < 4; ++c) {
      u8 a[4] = {s[4 * c], s[4 * c + 1], s[4 * c + 2], s[4 * c + 3]};
      s[4 * c]     = (u8)(aesdetail::gmul(a[0], 0x0e) ^ aesdetail::gmul(a[1], 0x0b) ^
                          aesdetail::gmul(a[2], 0x0d) ^ aesdetail::gmul(a[3], 0x09));
      s[4 * c + 1] = (u8)(aesdetail::gmul(a[0], 0x09) ^ aesdetail::gmul(a[1], 0x0e) ^
                          aesdetail::gmul(a[2], 0x0b) ^ aesdetail::gmul(a[3], 0x0d));
      s[4 * c + 2] = (u8)(aesdetail::gmul(a[0], 0x0d) ^ aesdetail::gmul(a[1], 0x09) ^
                          aesdetail::gmul(a[2], 0x0e) ^ aesdetail::gmul(a[3], 0x0b));
      s[4 * c + 3] = (u8)(aesdetail::gmul(a[0], 0x0b) ^ aesdetail::gmul(a[1], 0x0d) ^
                          aesdetail::gmul(a[2], 0x09) ^ aesdetail::gmul(a[3], 0x0e));
    }
  }

 public:
  /**
   * @brief 以密钥构造 AES 实例（执行密钥扩展）。
   * @param key 密钥，4*Nk 字节（AES-128:16 / AES-192:24 / AES-256:32）
   */
  explicit AES(const u8 key[4 * Nk]) { key_expand(key); }

  /**
   * @brief 加密一个 128 位分组。
   * @param in  输入明文（16 字节）
   * @param out 输出密文（16 字节，可与 in 重叠）
   */
  void encrypt_block(const u8 in[16], u8 out[16]) const {
    u8 s[16];
    std::memcpy(s, in, 16);
    add_round_key(s, 0);
    for (int r = 1; r < Nr; ++r) {
      sub_bytes(s);
      shift_rows(s);
      mix_columns(s);
      add_round_key(s, r);
    }
    sub_bytes(s);
    shift_rows(s);
    add_round_key(s, Nr);
    std::memcpy(out, s, 16);
  }

  /**
   * @brief 解密一个 128 位分组。
   * @param in  输入密文（16 字节）
   * @param out 输出明文（16 字节，可与 in 重叠）
   */
  void decrypt_block(const u8 in[16], u8 out[16]) const {
    u8 s[16];
    std::memcpy(s, in, 16);
    add_round_key(s, Nr);
    for (int r = Nr - 1; r >= 1; --r) {
      inv_shift_rows(s);
      inv_sub_bytes(s);
      add_round_key(s, r);
      inv_mix_columns(s);
    }
    inv_shift_rows(s);
    inv_sub_bytes(s);
    add_round_key(s, 0);
    std::memcpy(out, s, 16);
  }

  /**
   * @brief ECB 模式加密（等长，无填充）。
   * @param in   输入明文指针
   * @param out  输出密文指针（可与 in 重叠）
   * @param nblocks 分组数（总字节数 = 16*nblocks）
   */
  void ecb_encrypt(const u8* in, u8* out, size_t nblocks) const {
    for (size_t i = 0; i < nblocks; ++i) encrypt_block(in + 16 * i, out + 16 * i);
  }

  /**
   * @brief ECB 模式解密（等长，无填充）。
   * @param in   输入密文指针
   * @param out  输出明文指针（可与 in 重叠）
   * @param nblocks 分组数
   */
  void ecb_decrypt(const u8* in, u8* out, size_t nblocks) const {
    for (size_t i = 0; i < nblocks; ++i) decrypt_block(in + 16 * i, out + 16 * i);
  }
};

} // namespace crypto
} // namespace wbwlib

#endif // WBWLIB_CRYPTO_AES_HPP