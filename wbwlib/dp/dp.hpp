#ifndef WBWLIB_DP_DP_HPP
#define WBWLIB_DP_DP_HPP

/**
 * @file dp/dp.hpp
 * @brief dp 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/dp/dp.hpp>
 * @endcode
 */

#include "wbwlib/dp/cht.hpp"
#include "wbwlib/dp/digit-dp.hpp"
#include "wbwlib/dp/divide-conquer-dp.hpp"
#include "wbwlib/dp/state-compress.hpp"

#endif // WBWLIB_DP_DP_HPP
