#ifndef WBWLIB_MATH_MODULAR_HPP
#define WBWLIB_MATH_MODULAR_HPP

/**
 * @file modular.hpp
 * @brief 模意义整数类 modint（固定模版）与动态模版。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp
 *
 * 特性：
 *  - 自动约分（加/减/乘后立即取模）；
 *  - 支持全部主流运算符，与原生整数混合运算；
 *  - 逆元用扩展欧几里得（模数为任意互质值均可），快速幂用倍增；
 *  - 固定模版为编译期常量，运算最快。
 *
 * @par 示例
 * @code{.cpp}
 *   using M = wbwlib::math::modint<998244353>;
 *   M a = 3, b = 5;
 *   M c = a * b + 9;         // 24
 *   M d = M(10).pow(5);      // 100000 ... mod
 *   动态模：wbwlib::math::modint_dyn::set_mod(1e9+7);
 * @endcode
 */

#include <iostream>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"

namespace wbwlib {
namespace math {

// ================= 固定模数 ModInt =================
/**
 * @brief 固定模数的模整数类：所有运算自动取模，支持与原生整数混合运算。
 * @tparam MOD 编译期模数（> 0）
 * @tparam Base 底层整数类型，默认 i64
 */
template<u64 MOD, typename Base = i64>
class modint {
  Base v_;                        ///< 取值在 [0, MOD) 内
  static_assert(MOD > 0, "modint: 模数必须为正");

  /// 将 x 归一到 [0, MOD) 区间
  static Base norm(Base x) {
    if (x >= (Base)MOD) x -= (Base)MOD;
    else if (x < 0) x += (Base)MOD;
    return x;
  }

public:
  using value_type = Base;

  /// 编译期模数
  static constexpr u64 mod_value() { return MOD; }

  /**
   * @brief 默认构造：值为 0。
   */
  modint() = default;

  /**
   * @brief 从 Base 整数构造（自动取模归一）。
   * @param v 初始值
   */
  modint(Base v) : v_(norm(v % (Base)MOD)) {}

  /**
   * @brief 从任意整数类型构造（自动取模归一）。
   * @tparam T 整数类型
   * @param v 初始值
   */
  template<class T, typename = enable_if_t<std::is_integral<T>::value>>
  modint(T v) : v_(norm((Base)(v % (Base)MOD))) {}

  /// 返回内部数值（[0, MOD) 内）
  Base val() const { return v_; }

  // ---------- 运算 ----------
  /**
   * @brief 加法：\f$(v_1 + v_2) \bmod MOD\f$。
   * @param o 加数
   * @return 两数之和（已取模）
   */
  modint operator+(const modint& o) const { modint r(*this); r.v_ += o.v_; r.v_ = norm(r.v_); return r; }

  /**
   * @brief 减法：\f$(v_1 - v_2) \bmod MOD\f$（保证结果非负）。
   * @param o 减数
   * @return 两数之差（已取模）
   */
  modint operator-(const modint& o) const { modint r(*this); r.v_ -= o.v_; r.v_ = norm(r.v_); return r; }

  /**
   * @brief 乘法：\f$(v_1 \cdot v_2) \bmod MOD\f$。
   * @param o 乘数
   * @return 两数之积（已取模）
   */
  modint operator*(const modint& o) const {
#if WBWLIB_HAS_INT128
    return modint((Base)((i128)v_ * o.v_ % (Base)MOD));
#else
    return modint(mul_mod(static_cast<u64>(v_), static_cast<u64>(o.v_), MOD));
#endif
  }
  /**
   * @brief 除法：乘以 o 的逆元（要求 \f$\gcd(o, MOD) = 1\f$）。
   * @param o 除数
   * @return 商（已取模）
   */
  modint operator/(const modint& o) const { return *this * o.inv(); }

  /**
   * @brief 加法赋值。
   * @param o 加数
   * @return 自身引用
   */
  modint& operator+=(const modint& o) { v_ = norm(v_ + o.v_); return *this; }

  /**
   * @brief 减法赋值。
   * @param o 减数
   * @return 自身引用
   */
  modint& operator-=(const modint& o) { v_ = norm(v_ - o.v_); return *this; }

  /**
   * @brief 乘法赋值。
   * @param o 乘数
   * @return 自身引用
   */
  modint& operator*=(const modint& o) { *this = *this * o; return *this; }

  /**
   * @brief 除法赋值。
   * @param o 除数
   * @return 自身引用
   */
  modint& operator/=(const modint& o) { *this = *this / o; return *this; }

  /**
   * @brief 一元正号：返回自身。
   * @return 自身副本
   */
  modint operator+() const { return *this; }

  /**
   * @brief 一元负号：返回 \f$MOD - v\f$（v=0 时为 0）。
   * @return 相反数（已取模）
   */
  modint operator-() const { modint r; r.v_ = norm(-v_); return r; }

  /**
   * @brief 前置自增（加 1）。
   * @return 自身引用
   */
  modint& operator++() { v_ = norm(v_ + 1); return *this; }

  /**
   * @brief 前置自减（减 1）。
   * @return 自身引用
   */
  modint& operator--() { v_ = norm(v_ - 1); return *this; }

  /**
   * @brief 相等比较。
   * @param o 比较对象
   * @return 数值相等返回 true
   */
  bool operator==(const modint& o) const { return v_ == o.v_; }

  /**
   * @brief 不等比较。
   * @param o 比较对象
   * @return 数值不等返回 true
   */
  bool operator!=(const modint& o) const { return v_ != o.v_; }

  // ---------- 幂 / 逆 ----------
  /**
   * @brief 快速幂：计算 \f$a^p \bmod MOD\f$。
   * @param p 指数（非负）
   * @return \f$a^p \bmod MOD\f$
   */
  modint pow(i64 p) const {
    Base r = 1 % (Base)MOD, a = v_;
    while (p > 0) {
      if (p & 1) r = (Base)((i128)r * a % (Base)MOD);
      a = (Base)((i128)a * a % (Base)MOD);
      p >>= 1;
    }
    return modint(r);
  }

  /**
   * @brief 求逆元（扩展欧几里得；要求 \f$\gcd(v, MOD) = 1\f$，与 mod 是否质数无关）。
   * @return 满足 \f$v \cdot x \equiv 1 \pmod{MOD}\f$ 的 x；v 为 0 时报错
   */
  modint inv() const {
    if (v_ == 0) wbw_error("modint::inv 对 0 求逆非法");
    Base x, y;
    ext_gcd(v_, (Base)MOD, x, y);
    x = (x % (Base)MOD + (Base)MOD) % (Base)MOD;
    return modint(x);
  }
};

// ---------- 与原生整数的混合运算 ----------
/**
 * @brief modint 与整数的加法（整数在右侧）。
 * @tparam T 整数类型
 * @param a modint
 * @param b 整数
 * @return 和
 */
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator+(const modint<MOD, Base>& a, T b) { return a + modint<MOD, Base>(b); }
/**
 * @brief 整数与 modint 的加法（整数在左侧）。
 * @tparam T 整数类型
 * @param a 整数
 * @param b modint
 * @return 和
 */
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator+(T a, const modint<MOD, Base>& b) { return modint<MOD, Base>(a) + b; }
/**
 * @brief modint 与整数的减法（整数在右侧）。
 * @tparam T 整数类型
 * @param a modint
 * @param b 整数
 * @return 差
 */
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator-(const modint<MOD, Base>& a, T b) { return a - modint<MOD, Base>(b); }
/**
 * @brief 整数与 modint 的减法（整数在左侧）。
 * @tparam T 整数类型
 * @param a 整数
 * @param b modint
 * @return 差
 */
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator-(T a, const modint<MOD, Base>& b) { return modint<MOD, Base>(a) - b; }
/**
 * @brief modint 与整数的乘法（整数在右侧）。
 * @tparam T 整数类型
 * @param a modint
 * @param b 整数
 * @return 积
 */
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator*(const modint<MOD, Base>& a, T b) { return a * modint<MOD, Base>(b); }
/**
 * @brief 整数与 modint 的乘法（整数在左侧）。
 * @tparam T 整数类型
 * @param a 整数
 * @param b modint
 * @return 积
 */
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator*(T a, const modint<MOD, Base>& b) { return modint<MOD, Base>(a) * b; }

/**
 * @brief 输出 modint 的数值。
 * @param os 输出流
 * @param m modint 对象
 * @return 输出流引用
 */
template<u64 MOD, class Base>
std::ostream& operator<<(std::ostream& os, const modint<MOD, Base>& m) { return os << m.val(); }

// ================= 动态模数 ModInt =================
/**
 * @brief 动态模数的模整数类：模数可在运行时用 set_mod 设置。
 *
 * 内部用函数局部静态变量保存模数，多线程不安全（OI 无害）。运算与固定版一致；
 * 逆元同样依赖于被调用时的当前模数。
 */
class modint_dyn {
  /// 当前全局模数（函数局部静态，默认 1000000007）
  static i64& m() { static i64 mod = 1000000007; return mod; }
  i64 v_;

  /// 将 x 归一到 [0, 当前模数) 区间
  static i64 norm(i64 x) {
    i64 MOD = m();
    if (x >= MOD) x %= MOD;
    if (x < 0) x = (x % MOD + MOD) % MOD;
    return x;
  }

public:
  using value_type = i64;

  /// 当前模数
  static u64 mod_value() { return (u64)m(); }

  /**
   * @brief 默认构造：值为 0。
   */
  modint_dyn() = default;

  /**
   * @brief 从整数构造（自动取模归一）。
   * @param v 初始值
   */
  modint_dyn(i64 v) : v_(norm(v)) {}

  /**
   * @brief 设置全局模数（须为正）。
   * @param mod 新模数
   */
  static void set_mod(i64 mod) { WBWLIB_ASSERT(mod > 0); m() = mod; }

  /// 返回当前模数
  static i64 get_mod() { return m(); }

  /// 返回内部数值（[0, 当前模数) 内）
  i64 val() const { return v_; }

  /**
   * @brief 显式转换为 i64。
   * @return 内部数值
   */
  explicit operator i64() const { return v_; }

  /**
   * @brief 加法。
   * @param o 加数
   * @return 和（已取模）
   */
  modint_dyn operator+(const modint_dyn& o) const { return modint_dyn(norm(v_ + o.v_)); }

  /**
   * @brief 减法。
   * @param o 减数
   * @return 差（已取模）
   */
  modint_dyn operator-(const modint_dyn& o) const { return modint_dyn(norm(v_ - o.v_)); }

  /**
   * @brief 乘法。
   * @param o 乘数
   * @return 积（已取模）
   */
  modint_dyn operator*(const modint_dyn& o) const {
#if WBWLIB_HAS_INT128
    return modint_dyn((i64)((i128)v_ * o.v_ % m()));
#else
    return modint_dyn(mul_mod((u64)v_, (u64)o.v_, (u64)m()));
#endif
  }
  /**
   * @brief 除法：乘以 o 的逆元（要求与当前模数互质）。
   * @param o 除数
   * @return 商（已取模）
   */
  modint_dyn operator/(const modint_dyn& o) const { return *this * o.inv(); }

  /**
   * @brief 加法赋值。
   * @param o 加数
   * @return 自身引用
   */
  modint_dyn& operator+=(const modint_dyn& o) { v_ = norm(v_ + o.v_); return *this; }

  /**
   * @brief 减法赋值。
   * @param o 减数
   * @return 自身引用
   */
  modint_dyn& operator-=(const modint_dyn& o) { v_ = norm(v_ - o.v_); return *this; }

  /**
   * @brief 乘法赋值。
   * @param o 乘数
   * @return 自身引用
   */
  modint_dyn& operator*=(const modint_dyn& o) { v_ = norm((i64)((i128)v_ * o.v_ % m())); return *this; }

  /**
   * @brief 除法赋值。
   * @param o 除数
   * @return 自身引用
   */
  modint_dyn& operator/=(const modint_dyn& o) { *this *= o.inv(); return *this; }

  /**
   * @brief 一元负号。
   * @return 相反数（已取模）
   */
  modint_dyn operator-() const { return modint_dyn(norm(-v_)); }

  /**
   * @brief 相等比较。
   * @param o 比较对象
   * @return 数值相等返回 true
   */
  bool operator==(const modint_dyn& o) const { return v_ == o.v_; }

  /**
   * @brief 不等比较。
   * @param o 比较对象
   * @return 数值不等返回 true
   */
  bool operator!=(const modint_dyn& o) const { return v_ != o.v_; }

  /**
   * @brief 快速幂：计算 \f$a^p\f$ 对当前模数取模。
   * @param p 指数（非负）
   * @return \f$a^p \bmod m\f$
   */
  modint_dyn pow(i64 p) const {
    i64 r = 1 % m(), a = v_;
    while (p > 0) {
      if (p & 1) r = (i64)((i128)r * a % m());
      a = (i64)((i128)a * a % m());
      p >>= 1;
    }
    return modint_dyn(r);
  }
  /**
   * @brief 求逆元（扩展欧几里得；要求与当前模数互质）。
   * @return 满足 \f$v \cdot x \equiv 1\f$（当前模数）的 x；v 为 0 时报错
   */
  modint_dyn inv() const {
    if (v_ == 0) wbw_error("modint_dyn::inv 对 0 求逆非法");
    i64 x, y;
    ext_gcd(v_, m(), x, y);
    return modint_dyn(norm(x));
  }
};

/**
 * @brief 输出 modint_dyn 的数值。
 * @param os 输出流
 * @param m modint_dyn 对象
 * @return 输出流引用
 */
inline std::ostream& operator<<(std::ostream& os, const modint_dyn& m) { return os << m.val(); }

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_MODULAR_HPP