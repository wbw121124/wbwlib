#ifndef WBWLIB_STR_SUFFIX_AUTOMATON_HPP
#define WBWLIB_STR_SUFFIX_AUTOMATON_HPP

/**
 * @file suffix-automaton.hpp
 * @brief 后缀自动机 SAM。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：状态/转移数 O(2n)，构建线性。
 *
 * 用途：
 *   - 本质不同子串数：sum(len[i] - len[link[i]])
 *   - 每个子串出现次数：topo 求和 cnt
 *   - 多个模式匹配：从起点沿字符转移
 *
 * 用法：
 *   wbwlib::str::SAM sam;
 *   sam.extend(ch) for s；sam.build();
 *   sam.distinct_substr()； sam.occOfStart(i)...
 */

#include <string>
#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

class SAM {
  static const int ALPHA = 26;

 public:
  struct State {
    int len, link;
    int nxt[ALPHA];
    i64 cnt;          ///< 该状态（等价类）的 endpos 大小（非克隆初始 1）
    int cnt2;         ///< 用于排序的度数
  };
  std::vector<State> st_;
  int last_;

  SAM() { reset(); }

  void reset() {
    st_.clear();
    st_.push_back({0, -1, {}, 0, 0});
    last_ = 0;
  }

  void extend(int c) {
    int cur = (int)st_.size();
    st_.push_back({st_[last_].len + 1, 0, {}, 1, 0});
    int p = last_;
    while (p != -1 && st_[p].nxt[c] == 0) {
      st_[p].nxt[c] = cur;
      p = st_[p].link;
    }
    if (p == -1) {
      st_[cur].link = 0;
    } else {
      int q = st_[p].nxt[c];
      if (st_[p].len + 1 == st_[q].len) {
        st_[cur].link = q;
      } else {
        int clone = (int)st_.size();
        st_.push_back(st_[q]);
        st_[clone].len = st_[p].len + 1;
        st_[clone].cnt = 0;
        while (p != -1 && st_[p].nxt[c] == q) {
          st_[p].nxt[c] = clone;
          p = st_[p].link;
        }
        st_[q].link = st_[cur].link = clone;
      }
    }
    last_ = cur;
  }

  /// 直接吃一个字符串
  void build(const std::string& s) {
    for (char ch : s) extend(ch - 'a');
  }

  /// 按长度拓扑排序，累加 cnt（需在 extend 全部完成后调用一次）
  void build_cnt() {
    int sz = (int)st_.size();
    std::vector<int> order(sz);
    int mx = 0;
    for (int i = 0; i < sz; ++i) mx = std::max(mx, st_[i].len);
    std::vector<int> bucket(mx + 1, 0);
    for (int i = 0; i < sz; ++i) ++bucket[st_[i].len];
    for (int i = 1; i <= mx; ++i) bucket[i] += bucket[i - 1];
    for (int i = sz - 1; i >= 0; --i) order[--bucket[st_[i].len]] = i;
    // 从长到短累加 endpos
    for (int i = sz - 1; i > 0; --i) {
      int v = order[i];
      if (st_[v].link >= 0) st_[st_[v].link].cnt += st_[v].cnt;
    }
  }

  /// 本质不同子串数量（调用前需完整构建）
  i64 distinct_substr() const {
    i64 ans = 0;
    for (size_t i = 1; i < st_.size(); ++i)
      ans += st_[i].len - st_[st_[i].link].len;
    return ans;
  }

  /// 某子串的出现次数：从起点沿转移走完子串所在状态，返回 st_[state].cnt
  i64 count_occurrence(const std::string& pat) const {
    int u = 0;
    for (char ch : pat) {
      int c = ch - 'a';
      if (st_[u].nxt[c] == 0) return 0;
      u = st_[u].nxt[c];
    }
    return st_[u].cnt;
  }

  int size() const { return (int)st_.size(); }
  int last() const { return last_; }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_SUFFIX_AUTOMATON_HPP