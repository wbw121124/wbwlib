#ifndef WBWLIB_CORE_FASTIO_HPP
#define WBWLIB_CORE_FASTIO_HPP

/**
 * @file fastio.hpp
 * @brief 快速输入输出（信息学竞赛必备）。
 *
 * 依赖：core/base.hpp
 * 复杂度：O(输入字节数)，比 scanf/cin 快数倍。
 *
 * 用法：
 *   wbwlib::FastIO io;                 // 默认绑定 stdin/stdout
 *   int n; io.read(n);
 *   long long x; io.read(x);
 *   io.write(x), io.print(" "), io.writeln(n);
 *   支持字符串（读 token）与 char。
 *   // 重定向文件：
 *   wbwlib::FastIO io("data.in", "data.out");
 *
 * 说明：默认不解析正负号更快？这里支持负号；读 token 式字符串。
 * 并发安全：单线程 OI 场景。多实例混用不安全，请全局唯一。
 */

#include <cstdio>
#include <cstring>
#include <string>
#include "base.hpp"

namespace wbwlib {
namespace core {

class FastIO {
  static const int bufsz = 1 << 20;   ///< 缓冲大小
  char inbuf[bufsz], *inp, *inend;
  char outbuf[bufsz], *outp, *outend;
  bool ownin, ownout;                 ///< 是否由本对象打开的 FILE
  FILE *fin, *fout;

  inline char getc() {
    if (inp == inend) {
      inend = inbuf + std::fread(inbuf, 1, bufsz, fin);
      inp = inbuf;
      if (inp == inend) return 0;   // EOF
    }
    return *inp++;
  }

  inline void putc(char c) {
    if (outp == outend) {
      std::fwrite(outbuf, 1, outp - outbuf, fout);
      outp = outbuf;
    }
    *outp++ = c;
  }

  void flsh() {
    if (outp != outbuf) {
      std::fwrite(outbuf, 1, outp - outbuf, fout);
      outp = outbuf;
    }
    if (ownout) std::fflush(fout);
  }

  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value), void>
  readi(T& x) {
    x = 0; int sgn = 1; char c = getc();
    while (c <= ' ') { if (!c) return; c = getc(); }
    if (c == '-') { sgn = -1; c = getc(); }
    for (; c > ' '; c = getc()) x = T(x * 10 + (c - '0'));
    x *= T(sgn);
  }

  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value), void>
  readi(T& x) {
    x = 0; char c = getc();
    while (c <= ' ') { if (!c) return; c = getc(); }
    for (; c > ' '; c = getc()) x = T(x * 10 + (c - '0'));
  }

  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value), void>
  writei(T x) {
    i64 v = static_cast<i64>(x);
    i64 base = v < 0 ? -v : v;
    char tmp[24]; int len = 0;
    if (base == 0) tmp[len++] = '0';
    while (base > 0) { tmp[len++] = char('0' + base % 10); base /= 10; }
    if (v < 0) putc('-');
    while (len) putc(tmp[--len]);
  }

  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value), void>
  writei(T x) {
    char tmp[24]; int len = 0;
    do { tmp[len++] = char('0' + x % 10); x /= 10; } while (x);
    while (len) putc(tmp[--len]);
  }

public:
  FastIO() : inp(inbuf), inend(inbuf), outp(outbuf), outend(outbuf + bufsz),
             ownin(false), ownout(false), fin(stdin), fout(stdout) {}

  FastIO(const char* inpath, const char* outpath = nullptr)
      : inp(inbuf), inend(inbuf), outp(outbuf), outend(outbuf + bufsz),
        ownin(true), ownout(outpath != nullptr),
        fin(std::fopen(inpath, "rb")),
        fout(outpath ? std::fopen(outpath, "wb") : stdout) {
    if (!fin) { wbw_error("FastIO: 无法打开输入文件"); }
    if (outpath && !fout) { wbw_error("FastIO: 无法打开输出文件"); }
  }

  // 重置到 stdin/stdout（关闭文件流）
  void reset_to_stdio() {
    flsh();
    if (ownin) std::fclose(fin);
    if (ownout) std::fclose(fout);
    fin = stdin; fout = stdout; ownin = ownout = false;
  }

  ~FastIO() { flsh(); if (ownin) std::fclose(fin); if (ownout) std::fclose(fout); }

  bool eof() { return inp == inend && std::feof(fin); }

  // ---------- 读 ----------
  template<class T>
  enable_if_t<std::is_integral<T>::value, FastIO&>
  read(T& x) { readi(x); return *this; }

  void read(char& c) { c = getc(); }

  void read(char* s) {  // 读一个不含空白符的 token
    char c = getc();
    while (c <= ' ') { if (!c) { *s = 0; return; } c = getc(); }
    for (; c > ' '; c = getc()) *s++ = c;
    *s = 0;
  }

  void read(std::string& s) {
    s.clear();
    char c = getc();
    while (c <= ' ') { if (!c) return; c = getc(); }
    for (; c > ' '; c = getc()) s += c;
  }

  // ---------- 写 ----------
  template<class T>
  enable_if_t<std::is_integral<T>::value, FastIO&>
  write(const T& x) { writei(x); return *this; }

  FastIO& write(const char* s) { while (*s) putc(*s++); return *this; }
  FastIO& write(char c) { putc(c); return *this; }
  FastIO& write(const std::string& s) { for (char c : s) putc(c); return *this; }
#if WBWLIB_HAS_INT128
  void write(const i128& x) {
    if (x == 0) { putc('0'); return; }
    bool neg = x < 0;
    u128 base = neg ? u128(-(x + 1)) + 1 : u128(x);
    char tmp[44]; int len = 0;
    while (base) { tmp[len++] = char('0' + base % 10); base /= 10; }
    if (neg) putc('-');
    while (len) putc(tmp[--len]);
  }
#endif

  // 打印空格 / 换行等便捷方法
  FastIO& print(const char* s) { return write(s); }
  FastIO& println(const char* s) { write(s); return write('\n'); }
  FastIO& sp() { return write(' '); }          ///< 输出一个空格
  FastIO& endl() { return write('\n'); }       ///< 输出换行
  template<class T>
  FastIO& writeln(const T& x) { write(x); return write('\n'); }
};

} // namespace core
} // namespace wbwlib

#endif // WBWLIB_CORE_FASTIO_HPP