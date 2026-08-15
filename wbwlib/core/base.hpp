#ifndef WBWLIB_CORE_BASE_HPP
#define WBWLIB_CORE_BASE_HPP

/**
 * @file base.hpp
 * @brief wbwlib 基础：版本号、特性探测宏、类型别名、enable_if 工具、错误策略。
 *
 * 复杂度和兼容性说明见各子模块。本文件仅提供编译期基础设施，不包含算法。
 *
 * 特性探测：
 *   WBWLIB_HAS_CPP17 / WBWLIB_HAS_CPP20 ：根据 __cplusplus 自动开启，
 *   也可在编译命令行用 -DWBWLIB_FORCE_CPP20 手动覆盖。
 *
 * 错误策略：
 *   默认启用 assert；若定义 WBWLIB_THROW，则 wbw_error() 抛出异常。
 */

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>

#define WBWLIB_VERSION_MAJOR 1
#define WBWLIB_VERSION_MINOR 0
#define WBWLIB_VERSION_PATCH 0
#define WBWLIB_VERSION 10000

#ifndef WBWLIB_CONCAT_
#define WBWLIB_CONCAT_(a, b) a##b
#endif
#ifndef WBWLIB_CONCAT
#define WBWLIB_CONCAT(a, b) WBWLIB_CONCAT_(a, b)
#endif
// 字符串化
#ifndef WBWLIB_STR_
#define WBWLIB_STR_(x) #x
#endif
#ifndef WBWLIB_STR
#define WBWLIB_STR(x) WBWLIB_STR_(x)
#endif

// ---------- 特性探测 ----------
#if defined(WBWLIB_FORCE_CPP17)
#define WBWLIB_HAS_CPP17 1
#elif defined(__cplusplus) && __cplusplus >= 201703L
#define WBWLIB_HAS_CPP17 1
#else
#define WBWLIB_HAS_CPP17 0
#endif

#if defined(WBWLIB_FORCE_CPP20)
#define WBWLIB_HAS_CPP20 1
#elif defined(__cplusplus) && __cplusplus >= 202002L
#define WBWLIB_HAS_CPP20 1
#else
#define WBWLIB_HAS_CPP20 0
#endif

// 编译器内置 __int128 支持
#if defined(__SIZEOF_INT128__)
#define WBWLIB_HAS_INT128 1
#else
#define WBWLIB_HAS_INT128 0
#endif

namespace wbwlib {

// ---------- 错误处理 ----------
// 统一断言/抛异常策略。默认 assert；定义 WBWLIB_THROW 后抛 std::logic_error。
#ifdef WBWLIB_THROW
[[noreturn]] inline void wbw_error(const char* msg) {
  throw std::logic_error(msg);
}
#define WBWLIB_ASSERT(cond) \
  do { if (!(cond)) ::wbwlib::wbw_error(#cond); } while (0)
#else
[[noreturn]] inline void wbw_error(const char* msg) {
  assert(false && msg);
  std::abort();
}
#define WBWLIB_ASSERT(cond) assert(cond)
#endif

// ---------- 类型别名 ----------
using i8  = std::int8_t;   ///< 8 位有符号整数
using i16 = std::int16_t;  ///< 16 位有符号整数
using i32 = std::int32_t;  ///< 32 位有符号整数
using i64 = std::int64_t;  ///< 64 位有符号整数
using u8  = std::uint8_t;  ///< 8 位无符号整数
using u16 = std::uint16_t; ///< 16 位无符号整数
using u32 = std::uint32_t; ///< 32 位无符号整数
using u64 = std::uint64_t; ///< 64 位无符号整数
#if WBWLIB_HAS_INT128
using i128 = __int128_t;   ///< 128 位有符号整数
using u128 = __uint128_t;  ///< 128 位无符号整数
#endif

// ---------- 通用 trait 工具（C++14 版的 enable_if 便捷别名） ----------
template<bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template<bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

template<class T>
using remove_reference_t = typename std::remove_reference<T>::type;

template<class T>
using remove_cv_t = typename std::remove_cv<T>::type;

template<class T>
using decay_t = typename std::decay<T>::type;

// 判断 T 是否为 (有符号|无符号) 整数基本类型
template<class T> struct is_integral_base : std::integral_constant<bool, std::is_integral<T>::value> {};

// ---------- SFINAE 探测器 ----------
// 检测类型是否有成员函数 f（用于通用模板约束）
#define WBWLIB_DEF_MEMBER_DETECTOR(name, member)                                  \
  template<class T>                                                               \
  struct WBWLIB_CONCAT(has_member_, name) {                                       \
    template<class U> static auto test(int) -> decltype(std::declval<U>().member, std::true_type()); \
    template<class U> static auto test(...) -> std::false_type;                   \
    static const bool value = decltype(test<T>(0))::value;                        \
  };

} // namespace wbwlib

#endif // WBWLIB_CORE_BASE_HPP