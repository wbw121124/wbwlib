#ifndef WBWLIB_CRYPTO_MD5_HPP
#define WBWLIB_CRYPTO_MD5_HPP

/**
 * @file md5.hpp
 * @brief MD5 消息摘要（RFC 1321），128 位输出。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(n)（按 512 位分组，每轮 64 步）。
 *
 * @par 示例
 * @code{.cpp}
 *   std::string h = wbwlib::crypto::MD5::hex("abc");
 *   // h == "900150983cd24fb0d6963f7d28e17f72"
 *   u8 out[16];
 *   wbwlib::crypto::MD5::digest("abc", 3, out);
 * @endcode
 *
 * @attention 仅供教学/完整性校验，勿用于需要碰撞抗性的安全场景（MD5 已可碰撞）。
 */

#include <string>
#include <cstring>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace crypto {

/// MD5：128 位摘要，标准 RFC 1321 实现。
class MD5 {
  static u32 rotl(u32 x, int c) { return (x << c) | (x >> (32 - c)); }

  /// 每步常量：\f$K[i] = \lfloor 2^{32} \cdot |\sin(i+1)| \rfloor\f$
  static const u32 K[64];
  /// 每步循环左移位数
  static const u8 S[64];

 public:
  /**
   * @brief 计算数据摘要。
   * @param data 输入数据指针
   * @param len  输入长度（字节）
   * @param out  输出缓冲区（至少 16 字节）
   */
  static void digest(const void* data, size_t len, u8 out[16]) {
    const u8* in = (const u8*)data;
    u32 A = 0x67452301, B = 0xefcdab89, C = 0x98badcfe, D = 0x10325476;

    // 填充：0x80 + 0x00... + 64 位小端长度
    size_t pad = ((len + 8) / 64 + 1) * 64;
    std::string buf(pad, 0);
    if (len) std::memcpy(&buf[0], in, len);
    buf[len] = (char)0x80;
    u64 bits = (u64)len * 8;
    for (int i = 0; i < 8; ++i) buf[pad - 8 + i] = (char)((bits >> (8 * i)) & 0xff);

    for (size_t off = 0; off < pad; off += 64) {
      u32 M[16];
      for (int i = 0; i < 16; ++i) {
        M[i] = (u32)(u8)buf[off + 4 * i] | ((u32)(u8)buf[off + 4 * i + 1] << 8) |
               ((u32)(u8)buf[off + 4 * i + 2] << 16) | ((u32)(u8)buf[off + 4 * i + 3] << 24);
      }
      u32 a = A, b = B, c = C, d = D;
      for (int i = 0; i < 64; ++i) {
        u32 f, g;
        if (i < 16)      { f = (b & c) | (~b & d);         g = i; }
        else if (i < 32) { f = (d & b) | (~d & c);         g = (5 * i + 1) % 16; }
        else if (i < 48) { f = b ^ c ^ d;                  g = (3 * i + 5) % 16; }
        else             { f = c ^ (b | ~d);               g = (7 * i) % 16; }
        u32 tmp = d;
        d = c;
        c = b;
        b = b + rotl(a + f + K[i] + M[g], S[i]);
        a = tmp;
      }
      A += a; B += b; C += c; D += d;
    }
    // 小端输出
    for (int i = 0; i < 4; ++i) {
      out[i]      = (u8)(A >> (8 * i));
      out[i + 4]  = (u8)(B >> (8 * i));
      out[i + 8]  = (u8)(C >> (8 * i));
      out[i + 12] = (u8)(D >> (8 * i));
    }
  }

  /**
   * @brief 计算字符串的 MD5 十六进制串。
   * @param data 输入字符串（任意字节）
   * @return 32 位小写十六进制
   */
  static std::string hex(const std::string& data) {
    u8 out[16];
    digest(data.data(), data.size(), out);
    const char* t = "0123456789abcdef";
    std::string r(32, '0');
    for (int i = 0; i < 16; ++i) {
      r[2 * i] = t[out[i] >> 4];
      r[2 * i + 1] = t[out[i] & 15];
    }
    return r;
  }
};

const u32 MD5::K[64] = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

const u8 MD5::S[64] = {
  7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
  5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

} // namespace crypto
} // namespace wbwlib

#endif // WBWLIB_CRYPTO_MD5_HPP