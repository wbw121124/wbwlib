# wbwlib 建设计划

> 信息学竞赛 C++ 头文件库（header-only），命名空间 `wbwlib`，中文注释，全量考点 + 冷门扩展。

## 语言标准策略
- 严格基线 **C++14**；通过 `wbwlib/core/base.hpp` 的宏探测启用 C++17/20 特性。
- 泛型统一 `template<class T, typename = wbw::enable_if_t<...>>`，保证 C++14 可编译。
- `static_assert` 提供明确报错；常规路径零异常、零 RTTI。

## 目录结构
```
plan.md
README.md
wbwlib.h                     # 总入口
wbwlib/
  wbwlib.hpp                 # 聚合子头文件
  core/  base fastio random hash utils
  math/  number-theory primes modular combinatorics matrix linalg
         bsgs floor-sum primitive-root fraction fourier(FFT/NTT)
         polynomial fwt
  datastruct/ bit segtree segtree-beats persistent sparse-table
              fhq-treap splay leftist-heap dsu mo-algo sqrt-block
              monotonic li-chao-tree cartesian-tree
  string/ kmp z manacher trie ac-automaton suffix-array
          suffix-automaton palindromic-pam rolling-hash minimal-string
  graph/  adjacency shortest-path mst topo tarjan euler-path bipartite
          network-flow min-cost-flow lca tree-dp hld
          virtual-tree link-cut-tree
  dp/     cht divide-conquer-dp digit-dp state-compress
  geo/    point convex-hull polygon polar half-plane circle
  misc/   big-int
tests/
  compile-test.cpp
  run-tests.ps1
```

## 工程约定
- 每个 `.hpp` 自包含：`#pragma once` + include-what-you-use。
- 子命名空间：`wbwlib::math / wbwlib::ds / wbwlib::str / wbwlib::graph / wbwlib::dp / wbwlib::geo / wbwlib::core / wbwlib::misc`，根命名空间 `wbwlib`。
- 版本宏 `WBWLIB_VERSION`；错误策略默认 `assert`，可开 `WBWLIB_THROW`。
- 中文注释：文件头（复杂度/依赖/用法/示例）+ 关键函数注释。
- 全局函数风格，OI 惯例直接调用。

## 执行顺序
1. `plan.md` + `README.md` + `core/*`
2. `math/*`
3. `datastruct/*`
4. `string/*`
5. `graph/*`
6. `dp/*`、`geo/*`、`misc/big-int`
7. `wbwlib.h` 聚合 + `tests/` 冒烟测试编译运行并修复

## 验证
- 本机 mingw g++ 8.1.0；分别以 `-std=c++14` 与 `-std=c++17` 编译冒烟测试。
- C++20 分支仅宏保护，不在 g++8.1 本地测试。