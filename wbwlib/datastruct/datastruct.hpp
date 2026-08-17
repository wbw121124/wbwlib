#ifndef WBWLIB_DATASTRUCT_DATASTRUCT_HPP
#define WBWLIB_DATASTRUCT_DATASTRUCT_HPP

/**
 * @file datastruct/datastruct.hpp
 * @brief datastruct 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/datastruct/datastruct.hpp>
 * @endcode
 */

#include "wbwlib/datastruct/bit.hpp"
#include "wbwlib/datastruct/cartesian-tree.hpp"
#include "wbwlib/datastruct/dsu.hpp"
#include "wbwlib/datastruct/fhq-treap.hpp"
#include "wbwlib/datastruct/leftist-heap.hpp"
#include "wbwlib/datastruct/li-chao-tree.hpp"
#include "wbwlib/datastruct/mo-algo.hpp"
#include "wbwlib/datastruct/monotonic.hpp"
#include "wbwlib/datastruct/persistent-segtree.hpp"
#include "wbwlib/datastruct/segment_tree.hpp"
#include "wbwlib/datastruct/segtree-beats.hpp"
#include "wbwlib/datastruct/segtree.hpp"
#include "wbwlib/datastruct/sparse-table.hpp"
#include "wbwlib/datastruct/splay.hpp"
#include "wbwlib/datastruct/sqrt-block.hpp"

#endif // WBWLIB_DATASTRUCT_DATASTRUCT_HPP
