#ifndef WBWLIB_MATH_MODULAR_HPP
#define WBWLIB_MATH_MODULAR_HPP

/**
 * @file modular.hpp
 * @brief 模意义整数类 modint（固定模版）与动态模版。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp
 *
 * 特性：
 *  - 自动约分（加/减/乘后立即取模）；
 *  - 支持全部主流运算符，与原生整数混合运算；
 *  - 逆元用扩展欧几里得（模数为任意互质值均可），快速幂用倍增；
 *  - 固定模版为编译期常量，运算最快。
 *
 * 用法：
 *   using M = wbwlib::math::modint<998244353>;
 *   M a = 3, b = 5;
 *   M c = a * b + 9;         // 24
 *   M d = M(10).pow(5);      // 100000 ... mod
 *   动态模：wbwlib::math::modint_dyn::set_mod(1e9+7);
 */

#include <iostream>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"

namespace wbwlib {
namespace math {

// ================= 固定模数 ModInt =================
template<u64 MOD, typename Base = i64>
class modint {
  Base v_;                        ///< 取值在 [0, MOD) 内
  static_assert(MOD > 0, "modint: 模数必须为正");

  static Base norm(Base x) {
    if (x >= (Base)MOD) x -= (Base)MOD;
    else if (x < 0) x += (Base)MOD;
    return x;
  }

public:
  using value_type = Base;

  modint() = default;
  modint(Base v) : v_(norm(v % (Base)MOD)) {}

  template<class T, typename = enable_if_t<std::is_integral<T>::value>>
  modint(T v) : v_(norm((Base)(v % (Base)MOD))) {}

  Base val() const { return v_; }

  // ---------- 运算 ----------
  modint operator+(const modint& o) const { modint r(*this); r.v_ += o.v_; r.v_ = norm(r.v_); return r; }
  modint operator-(const modint& o) const { modint r(*this); r.v_ -= o.v_; r.v_ = norm(r.v_); return r; }
  modint operator*(const modint& o) const {
#if WBWLIB_HAS_INT128
    return modint((Base)((i128)v_ * o.v_ % (Base)MOD));
#else
    return modint(mul_mod(static_cast<u64>(v_), static_cast<u64>(o.v_), MOD));
#endif
  }
  modint operator/(const modint& o) const { return *this * o.inv(); }

  modint& operator+=(const modint& o) { v_ = norm(v_ + o.v_); return *this; }
  modint& operator-=(const modint& o) { v_ = norm(v_ - o.v_); return *this; }
  modint& operator*=(const modint& o) { *this = *this * o; return *this; }
  modint& operator/=(const modint& o) { *this = *this / o; return *this; }

  modint operator+() const { return *this; }
  modint operator-() const { modint r; r.v_ = norm(-v_); return r; }

  modint& operator++() { v_ = norm(v_ + 1); return *this; }
  modint& operator--() { v_ = norm(v_ - 1); return *this; }

  bool operator==(const modint& o) const { return v_ == o.v_; }
  bool operator!=(const modint& o) const { return v_ != o.v_; }

  // ---------- 幂 / 逆 ----------
  /// 快速幂 a^p mod MOD
  modint pow(i64 p) const {
    Base r = 1 % (Base)MOD, a = v_;
    while (p > 0) {
      if (p & 1) r = (Base)((i128)r * a % (Base)MOD);
      a = (Base)((i128)a * a % (Base)MOD);
      p >>= 1;
    }
    return modint(r);
  }

  /// 逆元（扩展欧几里得；要求 gcd(v_, MOD)==1，与是否质数无关）
  modint inv() const {
    if (v_ == 0) wbw_error("modint::inv 对 0 求逆非法");
    Base x, y;
    ext_gcd(v_, (Base)MOD, x, y);
    x = (x % (Base)MOD + (Base)MOD) % (Base)MOD;
    return modint(x);
  }
};

// ---------- 与原生整数的混合运算 ----------
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator+(const modint<MOD, Base>& a, T b) { return a + modint<MOD, Base>(b); }
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator+(T a, const modint<MOD, Base>& b) { return modint<MOD, Base>(a) + b; }
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator-(const modint<MOD, Base>& a, T b) { return a - modint<MOD, Base>(b); }
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator-(T a, const modint<MOD, Base>& b) { return modint<MOD, Base>(a) - b; }
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator*(const modint<MOD, Base>& a, T b) { return a * modint<MOD, Base>(b); }
template<u64 MOD, class Base, class T,
         typename = enable_if_t<std::is_integral<T>::value>>
inline modint<MOD, Base> operator*(T a, const modint<MOD, Base>& b) { return modint<MOD, Base>(a) * b; }

template<u64 MOD, class Base>
std::ostream& operator<<(std::ostream& os, const modint<MOD, Base>& m) { return os << m.val(); }

// ================= 动态模数 ModInt =================
/**
 * 模数可在运行时设置（set_mod）。内部用函数局部静态变量，多线程不安全（OI 无害）。
 * 运算与固定版一致；逆元同样依赖于被调用时的当前模数。
 */
class modint_dyn {
  static i64& m() { static i64 mod = 1000000007; return mod; }
  i64 v_;

  static i64 norm(i64 x) {
    i64 MOD = m();
    if (x >= MOD) x %= MOD;
    if (x < 0) x = (x % MOD + MOD) % MOD;
    return x;
  }

public:
  using value_type = i64;

  modint_dyn() = default;
  modint_dyn(i64 v) : v_(norm(v)) {}

  static void set_mod(i64 mod) { WBWLIB_ASSERT(mod > 0); m() = mod; }
  static i64 get_mod() { return m(); }

  i64 val() const { return v_; }
  explicit operator i64() const { return v_; }

  modint_dyn operator+(const modint_dyn& o) const { return modint_dyn(norm(v_ + o.v_)); }
  modint_dyn operator-(const modint_dyn& o) const { return modint_dyn(norm(v_ - o.v_)); }
  modint_dyn operator*(const modint_dyn& o) const {
#if WBWLIB_HAS_INT128
    return modint_dyn((i64)((i128)v_ * o.v_ % m()));
#else
    return modint_dyn(mul_mod((u64)v_, (u64)o.v_, (u64)m()));
#endif
  }
  modint_dyn operator/(const modint_dyn& o) const { return *this * o.inv(); }
  modint_dyn& operator+=(const modint_dyn& o) { v_ = norm(v_ + o.v_); return *this; }
  modint_dyn& operator-=(const modint_dyn& o) { v_ = norm(v_ - o.v_); return *this; }
  modint_dyn& operator*=(const modint_dyn& o) { v_ = norm((i64)((i128)v_ * o.v_ % m())); return *this; }
  modint_dyn& operator/=(const modint_dyn& o) { *this *= o.inv(); return *this; }
  modint_dyn operator-() const { return modint_dyn(norm(-v_)); }
  bool operator==(const modint_dyn& o) const { return v_ == o.v_; }
  bool operator!=(const modint_dyn& o) const { return v_ != o.v_; }

  modint_dyn pow(i64 p) const {
    i64 r = 1 % m(), a = v_;
    while (p > 0) {
      if (p & 1) r = (i64)((i128)r * a % m());
      a = (i64)((i128)a * a % m());
      p >>= 1;
    }
    return modint_dyn(r);
  }
  modint_dyn inv() const {
    if (v_ == 0) wbw_error("modint_dyn::inv 对 0 求逆非法");
    i64 x, y;
    ext_gcd(v_, m(), x, y);
    return modint_dyn(norm(x));
  }
};

inline std::ostream& operator<<(std::ostream& os, const modint_dyn& m) { return os << m.val(); }

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_MODULAR_HPP