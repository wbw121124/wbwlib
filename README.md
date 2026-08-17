# wbwlib

信息学竞赛（NOIP / CSP / NOI）C++ 头文件库：只含头文件，命名空间 `wbwlib`，中文注释，强类型模板，C++14 基线（自动启用 C++17/20 特性）。

## 使用

```cpp
#include <wbwlib.h>          // 全量聚合
// 或按需引入子头文件：
#include <wbwlib/wbwlib.hpp>
#include <wbwlib/string/string.hpp>   // 模块聚合头（8 个模块均有同名聚合头）
#include <wbwlib/datastruct/segtree.hpp>
#include <wbwlib/graph/shortest-path.hpp>
```

编译时把仓库根目录加入 `-I`，无需链接额外库：

```bash
g++ -O2 -std=c++14 -Wall -I D:/wbwlib main.cpp
# C++17：-std=c++17，C++20：需定义 WBWLIB_HAS_CPP20（或编译自动检测）
```

## 目录总览

| 目录 | 内容 |
|------|------|
| `core/` | 宏与特性探测、类型别名、快读快写、随机、抗卡哈希、通用工具（离散化、min/max） |
| `math/` | 数论（gcd/exgcd/CRT/线性筛/Miller-Rabin/Pollard-Rho）、ModInt、组合数学、矩阵、线性代数、FFT/NTT、多项式、BSGS、数论分块、原根、有理数、FWT |
| `datastruct/` | 树状数组、线段树（Lazy/Beats/主席树）、ST 表、FHQ Treap、Splay、左偏树、并查集（带权/可撤销）、莫队、分块、单调栈/队列、李超树、笛卡尔树 |
| `string/` | KMP、Z 函数、Manacher、Trie/01-Trie、AC 自动机、后缀数组、后缀自动机、回文自动机、字符串哈希、最小表示法 |
| `graph/` | 邻接表封装、最短路、最小生成树、拓扑、Tarjan 全家桶、欧拉路径、二分图、网络流、费用流、LCA、树 DP、树链剖分、虚树、LCT |
| `dp/` | 斜率优化 CHT、决策单调性分治 DP、数位 DP、状压辅助 |
| `geo/` | 点/向量、凸包、多边形、极角排序、半平面交、圆 |
| `misc/` | 压位大整数 |
| `tests/` | 冒烟测试与批量编译脚本 |

@dot 模块依赖总览
digraph wbwlib {
  rankdir=LR;
  node [shape=box, style="rounded,filled", fillcolor="#eef2ff", fontname="Microsoft YaHei"];
  "core" [label="core\n基础"];
  "math" [label="math\n数学"];
  "ds"   [label="datastruct\n数据结构"];
  "str"  [label="string\n字符串"];
  "graph"[label="graph\n图论"];
  "dp"   [label="dp\n动态规划"];
  "geo"  [label="geo\n计算几何"];
  "misc" [label="misc\n大整数"];
  "wbw"  [label="wbwlib.hpp\n聚合入口", fillcolor="#dbeafe"];
  "math" -> "core"; "ds" -> "core"; "str" -> "core"; "graph" -> "core"; "dp" -> "core"; "geo" -> "core"; "misc" -> "core";
  "graph" -> "ds"; "graph" -> "str";
  "dp" -> "ds"; "dp" -> "geo";
  "wbw" -> {"core" "math" "ds" "str" "graph" "dp" "geo" "misc"};
}
@enddot

## 惯例
- 全局函数风格：`wbw::math::gcd`、`wbw::ds::BIT`、`wbw::graph::dijkstra`。
- 默认错误处理用 `assert`；`#define WBWLIB_THROW` 可改为抛异常。
- 每个头文件顶部注释包含时间/空间复杂度、依赖与用法示例。

## 测试
```powershell
pwsh D:/wbwlib/tests/run-tests.ps1
```
以 `-std=c++14` 与 `-std=c++17` 编译运行全部断言。