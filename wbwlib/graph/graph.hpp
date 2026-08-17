#ifndef WBWLIB_GRAPH_GRAPH_HPP
#define WBWLIB_GRAPH_GRAPH_HPP

/**
 * @file graph/graph.hpp
 * @brief graph 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/graph/graph.hpp>
 * @endcode
 */

#include "wbwlib/graph/adjacency.hpp"
#include "wbwlib/graph/bipartite.hpp"
#include "wbwlib/graph/euler-path.hpp"
#include "wbwlib/graph/hld.hpp"
#include "wbwlib/graph/lca.hpp"
#include "wbwlib/graph/link-cut-tree.hpp"
#include "wbwlib/graph/min-cost-flow.hpp"
#include "wbwlib/graph/mst.hpp"
#include "wbwlib/graph/network-flow.hpp"
#include "wbwlib/graph/shortest-path.hpp"
#include "wbwlib/graph/tarjan.hpp"
#include "wbwlib/graph/topo.hpp"
#include "wbwlib/graph/tree-dp.hpp"
#include "wbwlib/graph/virtual-tree.hpp"

#endif // WBWLIB_GRAPH_GRAPH_HPP
