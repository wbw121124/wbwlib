#ifndef WBWLIB_CRYPTO_SM3_HPP
#define WBWLIB_CRYPTO_SM3_HPP

/**
 * @file sm3.hpp
 * @brief SM3 密码杂凑算法（GB/T 32905-2016），256 位输出。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(n)（512 位分组，每轮 64 步压缩）。
 *
 * @par 示例
 * @code{.cpp}
 *   std::string h = wbwlib::crypto::SM3::hex("abc");
 *   // h == "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"
 * @endcode
 */

#include <string>
#include <cstring>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace crypto {

/// SM3：256 位密码杂凑（国密）。
class SM3 {
  static u32 rotl(u32 x, int c) { return (x << c) | (x >> (32 - c)); }

  static u32 p0(u32 x) { return x ^ rotl(x, 9) ^ rotl(x, 17); }
  static u32 p1(u32 x) { return x ^ rotl(x, 15) ^ rotl(x, 23); }

  static void compress(u32 v[8], const u8 block[64]) {
    u32 w[68], w1[64];
    for (int i = 0; i < 16; ++i)
      w[i] = (u32)block[4 * i] << 24 | (u32)block[4 * i + 1] << 16 |
             (u32)block[4 * i + 2] << 8 | block[4 * i + 3];
    for (int j = 16; j < 68; ++j)
      w[j] = p1(w[j - 16] ^ w[j - 9] ^ rotl(w[j - 3], 15)) ^ rotl(w[j - 13], 7) ^ w[j - 6];
    for (int j = 0; j < 64; ++j) w1[j] = w[j] ^ w[j + 4];

    u32 a = v[0], b = v[1], c = v[2], d = v[3], e = v[4], f = v[5], g = v[6], h = v[7];
    for (int j = 0; j < 64; ++j) {
      u32 t = j < 16 ? 0x79cc4519u : 0x7a879d8au;
      u32 ss1 = rotl(rotl(a, 12) + e + rotl(t, j), 7);
      u32 ss2 = ss1 ^ rotl(a, 12);
      u32 tt1, tt2;
      if (j < 16) {
        tt1 = (a ^ b ^ c) + d + ss2 + w1[j];
        tt2 = (e ^ f ^ g) + h + ss1 + w[j];
      } else {
        tt1 = ((a & b) | (a & c) | (b & c)) + d + ss2 + w1[j];
        tt2 = ((e & f) | ((~e) & g)) + h + ss1 + w[j];
      }
      d = c; c = rotl(b, 9); b = a; a = tt1;
      h = g; g = rotl(f, 19); f = e; e = p0(tt2);
    }
    v[0] ^= a; v[1] ^= b; v[2] ^= c; v[3] ^= d;
    v[4] ^= e; v[5] ^= f; v[6] ^= g; v[7] ^= h;
  }

 public:
  /**
   * @brief 计算数据摘要。
   * @param data 输入数据指针
   * @param len  输入长度（字节）
   * @param out  输出缓冲区（至少 32 字节）
   */
  static void digest(const void* data, size_t len, u8 out[32]) {
    const u8* in = (const u8*)data;
    u32 v[8] = {0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
                0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e};

    size_t pad = ((len + 8) / 64 + 1) * 64;
    std::string buf(pad, 0);
    if (len) std::memcpy(&buf[0], in, len);
    buf[len] = (char)0x80;
    u64 bits = (u64)len * 8;
    for (int i = 0; i < 8; ++i) buf[pad - 8 + i] = (char)((bits >> (8 * (7 - i))) & 0xff);  // 大端

    for (size_t off = 0; off < pad; off += 64) compress(v, (const u8*)&buf[off]);

    for (int i = 0; i < 8; ++i) {
      out[4 * i]     = (u8)(v[i] >> 24);
      out[4 * i + 1] = (u8)(v[i] >> 16);
      out[4 * i + 2] = (u8)(v[i] >> 8);
      out[4 * i + 3] = (u8)v[i];
    }
  }

  /**
   * @brief 计算字符串的 SM3 十六进制串。
   * @param data 输入字符串（任意字节）
   * @return 64 位小写十六进制
   */
  static std::string hex(const std::string& data) {
    u8 out[32];
    digest(data.data(), data.size(), out);
    const char* t = "0123456789abcdef";
    std::string r(64, '0');
    for (int i = 0; i < 32; ++i) {
      r[2 * i] = t[out[i] >> 4];
      r[2 * i + 1] = t[out[i] & 15];
    }
    return r;
  }
};

} // namespace crypto
} // namespace wbwlib

#endif // WBWLIB_CRYPTO_SM3_HPP