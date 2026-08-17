#ifndef WBWLIB_MATH_FRACTION_HPP
#define WBWLIB_MATH_FRACTION_HPP

/**
 * @file fraction.hpp
 * @brief 精确有理数（分数）类：自动约分，支持全部算术/比较/输出。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/math/number-theory.hpp
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::math::Fraction a(1, 3), b = 2;
 *   Fraction c = a + b;            // 7/3
 *   c.print();                     // 7/3
 *   double d = a.value();          // 0.333...
 * @endcode
 */

#include <cstdlib>
#include <ostream>
#include "wbwlib/core/base.hpp"
#include "wbwlib/math/number-theory.hpp"

namespace wbwlib {
namespace math {

/**
 * @brief 精确有理数（分数）类：自动约分，支持全部算术/比较/输出。
 */
class Fraction {
  i64 num_, den_;   ///< 分母恒为正
 public:
  /**
   * @brief 构造分数 num/den（自动约分并保证分母为正）。
   * @param num 分子（默认 0）
   * @param den 分母（默认 1，不能为 0）
   */
  Fraction(i64 num = 0, i64 den = 1) : num_(num), den_(den) { norm(); }

  /**
   * @brief 约分并确保分母为正（分母为 0 时报错）。
   */
  void norm() {          // 约分 + 分母转正
    if (den_ == 0) wbw_error("Fraction::norm 分母为零");
    if (den_ < 0) { num_ = -num_; den_ = -den_; }
    i64 g = gcd(std::llabs(num_), std::llabs(den_));
    if (g) { num_ /= g; den_ /= g; }
  }

  /// 返回分子
  i64 num() const { return num_; }

  /// 返回分母（恒为正）
  i64 den() const { return den_; }

  /**
   * @brief 一元负号。
   * @return \f$-\frac{num}{den}\f$
   */
  Fraction operator-() const { return Fraction(-num_, den_); }

  /**
   * @brief 分数加法：\f$\frac{a}{b} + \frac{c}{d} = \frac{ad + bc}{bd}\f$（自动约分）。
   * @param o 加数
   * @return 和
   */
  Fraction operator+(const Fraction& o) const {
    i64 g = gcd(den_, o.den_);
    i64 l = den_ / g * o.den_;               // 公分母
    return Fraction(num_ * (o.den_ / g) + o.num_ * (den_ / g), l);
  }

  /**
   * @brief 分数减法。
   * @param o 减数
   * @return 差
   */
  Fraction operator-(const Fraction& o) const { return *this + (-o); }

  /**
   * @brief 分数乘法：\f$\frac{a}{b} \cdot \frac{c}{d} = \frac{ac}{bd}\f$（自动约分）。
   * @param o 乘数
   * @return 积
   */
  Fraction operator*(const Fraction& o) const { return Fraction(num_ * o.num_, den_ * o.den_); }

  /**
   * @brief 分数除法（除数为 0 时报错）。
   * @param o 除数
   * @return 商
   */
  Fraction operator/(const Fraction& o) const {
    if (o.num_ == 0) wbw_error("Fraction::operator/ 除零");
    return Fraction(num_ * o.den_, den_ * o.num_);   // norm 会修符号
  }

  /**
   * @brief 加法赋值。
   * @param o 加数
   * @return 自身引用
   */
  Fraction& operator+=(const Fraction& o) { return *this = *this + o; }

  /**
   * @brief 减法赋值。
   * @param o 减数
   * @return 自身引用
   */
  Fraction& operator-=(const Fraction& o) { return *this = *this - o; }

  /**
   * @brief 乘法赋值。
   * @param o 乘数
   * @return 自身引用
   */
  Fraction& operator*=(const Fraction& o) { return *this = *this * o; }

  /**
   * @brief 除法赋值。
   * @param o 除数
   * @return 自身引用
   */
  Fraction& operator/=(const Fraction& o) { return *this = *this / o; }

  /**
   * @brief 相等比较（约分后分子分母分别相等）。
   * @param o 比较对象
   * @return 相等返回 true
   */
  bool operator==(const Fraction& o) const { return num_ == o.num_ && den_ == o.den_; }

  /**
   * @brief 不等比较。
   * @param o 比较对象
   * @return 不等返回 true
   */
  bool operator!=(const Fraction& o) const { return !(*this == o); }

  /**
   * @brief 小于比较（交叉相乘）。
   * @param o 比较对象
   * @return \f$a/b < c/d\f$
   */
  bool operator<(const Fraction& o) const { return num_ * o.den_ < o.num_ * den_; }

  /**
   * @brief 大于比较。
   * @param o 比较对象
   * @return \f$a/b > c/d\f$
   */
  bool operator>(const Fraction& o) const { return o < *this; }

  /**
   * @brief 小于等于比较。
   * @param o 比较对象
   * @return \f$a/b \le c/d\f$
   */
  bool operator<=(const Fraction& o) const { return !(o < *this); }

  /**
   * @brief 大于等于比较。
   * @param o 比较对象
   * @return \f$a/b \ge c/d\f$
   */
  bool operator>=(const Fraction& o) const { return !(*this < o); }

  /**
   * @brief 转浮点值。
   * @return \f$num / den\f$（double）
   */
  double value() const { return (double)num_ / den_; }

  /**
   * @brief 以 "num/den" 形式输出。
   * @param os 输出流
   * @param f 分数
   * @return 输出流引用
   */
  friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
    return os << f.num_ << '/' << f.den_;
  }
};

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_FRACTION_HPP