#ifndef WBWLIB_MATH_FRACTION_HPP
#define WBWLIB_MATH_FRACTION_HPP

/**
 * @file fraction.hpp
 * @brief 精确有理数（分数）类：自动约分，支持全部算术/比较/输出。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp
 *
 * 用法：
 *   wbwlib::math::Fraction a(1, 3), b = 2;
 *   Fraction c = a + b;            // 7/3
 *   c.print();                     // 7/3
 *   double d = a.value();          // 0.333...
 */

#include <cstdlib>
#include <ostream>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"

namespace wbwlib {
namespace math {

class Fraction {
  i64 num_, den_;   ///< 分母恒为正
 public:
  Fraction(i64 num = 0, i64 den = 1) : num_(num), den_(den) { norm(); }

  void norm() {          // 约分 + 分母转正
    if (den_ == 0) wbw_error("Fraction::norm 分母为零");
    if (den_ < 0) { num_ = -num_; den_ = -den_; }
    i64 g = gcd(std::llabs(num_), std::llabs(den_));
    if (g) { num_ /= g; den_ /= g; }
  }

  i64 num() const { return num_; }
  i64 den() const { return den_; }

  Fraction operator-() const { return Fraction(-num_, den_); }

  Fraction operator+(const Fraction& o) const {
    i64 g = gcd(den_, o.den_);
    i64 l = den_ / g * o.den_;               // 公分母
    return Fraction(num_ * (o.den_ / g) + o.num_ * (den_ / g), l);
  }
  Fraction operator-(const Fraction& o) const { return *this + (-o); }
  Fraction operator*(const Fraction& o) const { return Fraction(num_ * o.num_, den_ * o.den_); }
  Fraction operator/(const Fraction& o) const {
    if (o.num_ == 0) wbw_error("Fraction::operator/ 除零");
    return Fraction(num_ * o.den_, den_ * o.num_);   // norm 会修符号
  }

  Fraction& operator+=(const Fraction& o) { return *this = *this + o; }
  Fraction& operator-=(const Fraction& o) { return *this = *this - o; }
  Fraction& operator*=(const Fraction& o) { return *this = *this * o; }
  Fraction& operator/=(const Fraction& o) { return *this = *this / o; }

  bool operator==(const Fraction& o) const { return num_ == o.num_ && den_ == o.den_; }
  bool operator!=(const Fraction& o) const { return !(*this == o); }
  bool operator<(const Fraction& o) const { return num_ * o.den_ < o.num_ * den_; }
  bool operator>(const Fraction& o) const { return o < *this; }
  bool operator<=(const Fraction& o) const { return !(o < *this); }
  bool operator>=(const Fraction& o) const { return !(*this < o); }

  double value() const { return (double)num_ / den_; }

  friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
    return os << f.num_ << '/' << f.den_;
  }
};

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_FRACTION_HPP