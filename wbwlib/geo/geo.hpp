#ifndef WBWLIB_GEO_GEO_HPP
#define WBWLIB_GEO_GEO_HPP

/**
 * @file geo/geo.hpp
 * @brief geo 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/geo/geo.hpp>
 * @endcode
 */

#include "wbwlib/geo/circle.hpp"
#include "wbwlib/geo/convex-hull.hpp"
#include "wbwlib/geo/half-plane.hpp"
#include "wbwlib/geo/point.hpp"
#include "wbwlib/geo/polar.hpp"
#include "wbwlib/geo/polygon.hpp"

#endif // WBWLIB_GEO_GEO_HPP
