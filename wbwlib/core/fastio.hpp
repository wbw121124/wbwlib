#ifndef WBWLIB_CORE_FASTIO_HPP
#define WBWLIB_CORE_FASTIO_HPP

/**
 * @file fastio.hpp
 * @brief 快速输入输出（信息学竞赛必备）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 * @par 复杂度
 * O(输入字节数)，比 scanf/cin 快数倍。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::FastIO io;                 // 默认绑定 stdin/stdout
 *   int n; io.read(n);
 *   long long x; io.read(x);
 *   io.write(x), io.print(" "), io.writeln(n);
 *   支持字符串（读 token）与 char。
 *   // 重定向文件：
 *   wbwlib::FastIO io("data.in", "data.out");
 * @endcode
 *
 * 说明：默认不解析正负号更快？这里支持负号；读 token 式字符串。
 * 并发安全：单线程 OI 场景。多实例混用不安全，请全局唯一。
 */

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace core {

/**
 * @brief 快速输入输出类：带缓冲的 stdin/stdout（或文件）读写，支持整数、字符串与 char。
 */
class FastIO {
  static const int bufsz = 1 << 20;   ///< 缓冲大小
  std::vector<char> inbuf_, outbuf_;  ///< 堆上缓冲（避免 2MB 栈开销）
  char *inp, *inend;
  char *outp, *outend;
  bool ownin, ownout;                 ///< 是否由本对象打开的 FILE
  FILE *fin, *fout;

  /// 从输入缓冲读一个字符；缓冲耗尽时用 fread 填充，EOF 返回 0
  inline char getc() {
    if (inp == inend) {
      inend = inbuf_.data() + std::fread(inbuf_.data(), 1, bufsz, fin);
      inp = inbuf_.data();
      if (inp == inend) return 0;   // EOF
    }
    return *inp++;
  }

  /// 向输出缓冲写一个字符；缓冲满时用 fwrite 冲刷到 fout
  inline void putc(char c) {
    if (outp == outend) {
      std::fwrite(outbuf_.data(), 1, outp - outbuf_.data(), fout);
      outp = outbuf_.data();
    }
    *outp++ = c;
  }

  /// 将输出缓冲剩余内容冲刷到 fout，ownout 时额外 fflush
  void flsh() {
    if (outp != outbuf_.data()) {
      std::fwrite(outbuf_.data(), 1, outp - outbuf_.data(), fout);
      outp = outbuf_.data();
    }
    if (ownout) std::fflush(fout);
  }

  /// 有符号整数读取核心：跳过空白，处理负号
  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value), void>
  readi(T& x) {
    x = 0; int sgn = 1; char c = getc();
    while (c <= ' ') { if (!c) return; c = getc(); }
    if (c == '-') { sgn = -1; c = getc(); }
    for (; c > ' '; c = getc()) x = T(x * 10 + (c - '0'));
    x *= T(sgn);
  }

  /// 无符号整数读取核心：跳过空白，逐位累加
  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value), void>
  readi(T& x) {
    x = 0; char c = getc();
    while (c <= ' ') { if (!c) return; c = getc(); }
    for (; c > ' '; c = getc()) x = T(x * 10 + (c - '0'));
  }

  /// 有符号整数输出核心：处理负号与逐位逆序
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

  /// 无符号整数输出核心：do-while 保证 0 也能输出
  template<class T>
  inline enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value), void>
  writei(T x) {
    char tmp[24]; int len = 0;
    do { tmp[len++] = char('0' + x % 10); x /= 10; } while (x);
    while (len) putc(tmp[--len]);
  }

public:
  /**
   * @brief 构造并绑定 stdin/stdout。
   */
  FastIO() : inbuf_(bufsz), outbuf_(bufsz),
             inp(inbuf_.data()), inend(inbuf_.data()),
             outp(outbuf_.data()), outend(outbuf_.data() + bufsz),
             ownin(false), ownout(false), fin(stdin), fout(stdout) {}

  /**
   * @brief 构造并绑定输入/输出文件（outpath 为空时输出到 stdout）。
   * @param inpath 输入文件路径，以 "rb" 打开
   * @param outpath 输出文件路径，以 "wb" 打开；为 nullptr 时输出到 stdout
   */
  FastIO(const char* inpath, const char* outpath = nullptr)
      : inbuf_(bufsz), outbuf_(bufsz),
        inp(inbuf_.data()), inend(inbuf_.data()),
        outp(outbuf_.data()), outend(outbuf_.data() + bufsz),
        ownin(true), ownout(outpath != nullptr),
        fin(std::fopen(inpath, "rb")),
        fout(outpath ? std::fopen(outpath, "wb") : stdout) {
    if (!fin) { wbw_error("FastIO: 无法打开输入文件"); }
    if (outpath && !fout) { wbw_error("FastIO: 无法打开输出文件"); }
  }

  /**
   * @brief 冲刷输出并重置到 stdin/stdout（同时关闭由本对象打开的文件流）。
   */
  void reset_to_stdio() {
    flsh();
    if (ownin) std::fclose(fin);
    if (ownout) std::fclose(fout);
    fin = stdin; fout = stdout; ownin = ownout = false;
  }

  /**
   * @brief 析构：冲刷输出缓冲并关闭由本对象打开的文件流。
   */
  ~FastIO() { flsh(); if (ownin) std::fclose(fin); if (ownout) std::fclose(fout); }

  /**
   * @brief 判断是否已到输入末尾。
   * @return 缓冲耗尽且底层文件到达 EOF 时为 true
   */
  bool eof() { return inp == inend && std::feof(fin); }

  // ---------- 读 ----------
  /**
   * @brief 读取一个整数到 x。
   * @param x 输出参数，保存读到的整数
   * @return 返回 *this 以支持链式调用
   */
  template<class T>
  enable_if_t<std::is_integral<T>::value, FastIO&>
  read(T& x) { readi(x); return *this; }

  /**
   * @brief 读取一个字符到 c。
   * @param c 输出参数，保存读到的字符（EOF 时为 0）
   */
  void read(char& c) { c = getc(); }

  /**
   * @brief 读取一个不含空白符的 token 到 s（以 \0 结尾）。
   * @param s 输出参数，目标 C 字符串缓冲区
   */
  void read(char* s) {
    char c = getc();
    while (c <= ' ') { if (!c) { *s = 0; return; } c = getc(); }
    for (; c > ' '; c = getc()) *s++ = c;
    *s = 0;
  }

  /**
   * @brief 读取一个不含空白符的 token 到字符串 s。
   * @param s 输出参数，保存读到的 token
   */
  void read(std::string& s) {
    s.clear();
    char c = getc();
    while (c <= ' ') { if (!c) return; c = getc(); }
    for (; c > ' '; c = getc()) s += c;
  }

  // ---------- 写 ----------
  /**
   * @brief 输出一个整数。
   * @param x 待输出的整数
   * @return 返回 *this 以支持链式调用
   */
  template<class T>
  enable_if_t<std::is_integral<T>::value, FastIO&>
  write(const T& x) { writei(x); return *this; }

  /**
   * @brief 输出 C 字符串。
   * @param s 待输出的字符串（以 \0 结尾）
   * @return 返回 *this 以支持链式调用
   */
  FastIO& write(const char* s) { while (*s) putc(*s++); return *this; }

  /**
   * @brief 输出单个字符。
   * @param c 待输出的字符
   * @return 返回 *this 以支持链式调用
   */
  FastIO& write(char c) { putc(c); return *this; }

  /**
   * @brief 输出 std::string。
   * @param s 待输出的字符串
   * @return 返回 *this 以支持链式调用
   */
  FastIO& write(const std::string& s) { for (char c : s) putc(c); return *this; }
#if WBWLIB_HAS_INT128
  /**
   * @brief 输出 128 位有符号整数（i128）。
   * @param x 待输出的整数
   */
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
  /**
   * @brief 打印 C 字符串（等价于 write）。
   * @param s 待输出的字符串
   * @return 返回 *this 以支持链式调用
   */
  FastIO& print(const char* s) { return write(s); }

  /**
   * @brief 打印 C 字符串并换行。
   * @param s 待输出的字符串
   * @return 返回 *this 以支持链式调用
   */
  FastIO& println(const char* s) { write(s); return write('\n'); }

  /**
   * @brief 输出一个空格。
   * @return 返回 *this 以支持链式调用
   */
  FastIO& sp() { return write(' '); }

  /**
   * @brief 输出换行。
   * @return 返回 *this 以支持链式调用
   */
  FastIO& endl() { return write('\n'); }

  /**
   * @brief 输出任意可写对象并换行。
   * @param x 待输出的对象（重载了 write）
   * @return 返回 *this 以支持链式调用
   */
  template<class T>
  FastIO& writeln(const T& x) { write(x); return write('\n'); }
};

} // namespace core
} // namespace wbwlib

#endif // WBWLIB_CORE_FASTIO_HPP