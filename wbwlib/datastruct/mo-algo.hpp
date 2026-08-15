#ifndef WBWLIB_DS_MO_ALGO_HPP
#define WBWLIB_DS_MO_ALGO_HPP

/**
 * @file mo-algo.hpp
 * @brief 莫队算法：离线处理静态区间查询（可加分可删）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O((n+q)·sqrt(n)·转移代价)。
 *
 * 用法（区间颜色数）：
 *   int cnt[MAXV];  int cur = 0;
 *   auto add = [&](int idx){ if (++cnt[a[idx]] == 1) ++cur; };
 *   auto del = [&](int idx){ if (--cnt[a[idx]] == 0) --cur; };
 *   auto get = [&]{ return cur; };
 *   auto ans = wbwlib::ds::mo_many(l, r, add, del, get);  // 一次处理多个询问
 *
 * 单类封装：
 *   wbwlib::ds::MoSolver mo(n, q);
 *   mo.add_query(l, r, id);
 *   auto res = mo.solve(a, add, del, get);   // res[id] = 该询问的答案
 */

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/// 多询问莫队；l,r 为 1 基闭区间，add/del 接收元素下标 idx，get 返回当前答案
template<class AddF, class DelF, class GetF>
std::vector<typename std::decay<typename std::result_of<GetF()>::type>::type>
mo_queries(const std::vector<std::pair<int, int>>& qs,
           AddF add, DelF del, GetF get, int n) {
  using Ans = typename std::decay<typename std::result_of<GetF()>::type>::type;
  int q = (int)qs.size();
  int B = (int)std::max(1, (int)std::sqrt((double)n));
  std::vector<int> ord(q);
  for (int i = 0; i < q; ++i) ord[i] = i;
  std::sort(ord.begin(), ord.end(), [&](int x, int y) {
    int bx = qs[x].first / B, by = qs[y].first / B;
    if (bx != by) return bx < by;
    // 奇偶优化
    return (bx & 1) ? qs[x].second > qs[y].second
                    : qs[x].second < qs[y].second;
  });
  std::vector<Ans> res(q);
  int L = 1, R = 0;
  for (int id : ord) {
    int ql = qs[id].first, qr = qs[id].second;
    while (L > ql) add(--L);
    while (R < qr) add(++R);
    while (L < ql) del(L++);
    while (R > qr) del(R--);
    res[id] = get();
  }
  return res;
}

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_MO_ALGO_HPP