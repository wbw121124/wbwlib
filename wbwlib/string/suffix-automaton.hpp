#ifndef WBWLIB_STR_SUFFIX_AUTOMATON_HPP
#define WBWLIB_STR_SUFFIX_AUTOMATON_HPP

/**
 * @file suffix-automaton.hpp
 * @brief 后缀自动机 SAM（模板化字符类型 / 字母表 / 字符映射）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/string/char-map.hpp
 *
 * @par 复杂度
 * 状态/转移数 O(2n)，构建线性。
 *
 * 用途：
 *   - 本质不同子串数：sum(len[i] - len[link[i]])
 *   - 每个子串出现次数：topo 求和 cnt
 *   - 多个模式匹配：从起点沿字符转移
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::SAM<> sam;                  // 默认：小写字母
 *   sam.build("aab"); sam.build_cnt();
 *   sam.distinct_substr();                   // 本质不同子串数
 *
 *   // 自定义字符集：如数字串（映射见 char-map.hpp 示例）
 *   struct DigitMap { static int id(char c){ return c-'0'; } static bool valid(char c){ return c>='0'&&c<='9'; } };
 *   wbwlib::str::SAM<char, 10, DigitMap> dsam;
 * @endcode
 */

#include <string>
#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/string/char-map.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief 后缀自动机 SAM：线性构建，支持本质不同子串数、子串出现次数等查询。
 * @tparam Char    字符类型（默认 char）
 * @tparam ALPHA   字母表大小（默认 26）
 * @tparam CharMap 字符映射策略（默认小写字母 LowerCharMap）
 */
template<class Char = char, int ALPHA = 26, class CharMap = LowerCharMap>
class SAM {

 public:
  /**
   * @brief SAM 状态（等价类），len 为状态内最长串长度，link 为后缀链接。
   */
  struct State {
    int len, link;
    int nxt[ALPHA];
    i64 cnt;          ///< 该状态（等价类）的 endpos 大小（非克隆初始 1）
    int cnt2;         ///< 用于排序的度数
  };
  std::vector<State> st_;
  int last_;

  /// 构造空 SAM（等价于 reset）
  SAM() { reset(); }

  /// 清空所有状态并重置到初始状态（只含根）
  void reset() {
    st_.clear();
    st_.push_back({0, -1, {}, 0, 0});
    last_ = 0;
  }

  /**
   * @brief 插入一个字符（0 基编码），必要时克隆状态并更新后缀链接。
   * @param c 字符编码（0..25）
   */
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

  /**
   * @brief 直接按字符串构建：逐个字符 extend。
   * @param s 输入字符串（字符集须被 CharMap::valid 覆盖）
   */
  void build(const std::basic_string<Char>& s) {
    for (Char ch : s) extend(CharMap::id(ch));
  }

  /**
   * @brief 按 len 桶排序拓扑序，沿后缀链接从长到短累加 endpos 计数（需在全部 extend 完成后调用一次）。
   */
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

  /**
   * @brief 本质不同子串数量，即 \f$\sum_i (len[i] - len[link[i]])\f$。
   * @return 本质不同子串个数（调用前需完整构建）
   */
  i64 distinct_substr() const {
    i64 ans = 0;
    for (size_t i = 1; i < st_.size(); ++i)
      ans += st_[i].len - st_[st_[i].link].len;
    return ans;
  }

  /**
   * @brief 某子串在整串中的出现次数（沿转移走完子串所在状态后取 cnt）。
   * @param pat 待查询子串
   * @return 出现次数（不存在返回 0；需先调用 build_cnt）
   */
  i64 count_occurrence(const std::basic_string<Char>& pat) const {
    int u = 0;
    for (Char ch : pat) {
      int c = CharMap::id(ch);
      if (st_[u].nxt[c] == 0) return 0;
      u = st_[u].nxt[c];
    }
    return st_[u].cnt;
  }

  /// 返回状态总数（含初始状态）
  int size() const { return (int)st_.size(); }
  /// 返回最后一个字符所在状态
  int last() const { return last_; }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_SUFFIX_AUTOMATON_HPP