#ifndef WBWLIB_WBWLIB_HPP
#define WBWLIB_WBWLIB_HPP

/**
 * @file wbwlib.hpp
 * @brief wbwlib 全量聚合头文件：按模块目录顺序引入全部头文件。
 *
 * @par 依赖
 * 仅 C++14 标准库；每个子头文件自带 include guard，可安全重复引入。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/wbwlib.hpp>   // 全量聚合
 *   #include <wbwlib.h>            // 仓库根目录下同名软链（可选）
 *   #include <wbwlib/string/string.hpp>   // 按模块聚合
 * @endcode
 *
 * 命名空间：wbwlib::core / wbwlib::math / wbwlib::ds / wbwlib::str /
 *            wbwlib::graph / wbwlib::dp / wbwlib::geo / wbwlib::misc / wbwlib::crypto
 *
 * 模块聚合头（按需引入）：
 *   wbwlib/core/core.hpp、wbwlib/math/math.hpp、wbwlib/datastruct/datastruct.hpp、
 *   wbwlib/string/string.hpp、wbwlib/graph/graph.hpp、wbwlib/dp/dp.hpp、
 *   wbwlib/geo/geo.hpp、wbwlib/misc/misc.hpp、wbwlib/crypto/crypto.hpp
 */

#include "wbwlib/core/core.hpp"
#include "wbwlib/math/math.hpp"
#include "wbwlib/datastruct/datastruct.hpp"
#include "wbwlib/string/string.hpp"
#include "wbwlib/graph/graph.hpp"
#include "wbwlib/dp/dp.hpp"
#include "wbwlib/geo/geo.hpp"
#include "wbwlib/misc/misc.hpp"
#include "wbwlib/crypto/crypto.hpp"

#endif // WBWLIB_WBWLIB_HPP