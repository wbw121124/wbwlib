#ifndef WBWLIB_CORE_CORE_HPP
#define WBWLIB_CORE_CORE_HPP

/**
 * @file core/core.hpp
 * @brief core 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/core/core.hpp>
 * @endcode
 */

#include "wbwlib/core/base.hpp"
#include "wbwlib/core/fastio.hpp"
#include "wbwlib/core/hash.hpp"
#include "wbwlib/core/random.hpp"
#include "wbwlib/core/utils.hpp"

#endif // WBWLIB_CORE_CORE_HPP
