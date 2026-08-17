#ifndef WBWLIB_MATH_MATH_HPP
#define WBWLIB_MATH_MATH_HPP

/**
 * @file math/math.hpp
 * @brief math 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/math/math.hpp>
 * @endcode
 */

#include "wbwlib/math/bsgs.hpp"
#include "wbwlib/math/combinatorics.hpp"
#include "wbwlib/math/floor-sum.hpp"
#include "wbwlib/math/fourier.hpp"
#include "wbwlib/math/fraction.hpp"
#include "wbwlib/math/fwt.hpp"
#include "wbwlib/math/linalg.hpp"
#include "wbwlib/math/matrix.hpp"
#include "wbwlib/math/modular.hpp"
#include "wbwlib/math/number-theory.hpp"
#include "wbwlib/math/polynomial.hpp"
#include "wbwlib/math/primes.hpp"
#include "wbwlib/math/primitive-root.hpp"

#endif // WBWLIB_MATH_MATH_HPP
