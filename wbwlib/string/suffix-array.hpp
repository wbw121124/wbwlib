#ifndef WBWLIB_STR_SUFFIX_ARRAY_HPP
#define WBWLIB_STR_SUFFIX_ARRAY_HPP

/**
 * @file suffix-array.hpp
 * @brief 后缀数组（倍增 + 计数排序，含身扩展：LCP、第 k 大）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(n log n) 时间，O(n) 空间。
 *
 * 结果（0 基字符串 s，长度 n）：
 *   sa[1..n]    —— 第 i 小的后缀的起始位置（0 基）；（sa[0] 恒为 0，未使用）
 *   rank[i]     —— 起始位置 i 的后缀排名（1 基）
 *   height[1..n]—— height[i] = LCP(sa[i-1], sa[i])，height[1] = 0
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::SuffixArray SA("banana");
 *   for (int i = 1; i <= SA.size(); ++i) std::cout << SA.sa[i];
 *   int lcp = SA.lcp(x, y);     // 位置 x 与 y 两后缀的 LCP 长度
 * @endcode
 */

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief 后缀数组：倍增 + 计数排序构造，附 rank、height（LCP）数组。
 *
 * 结果（0 基字符串 s，长度 n）：
 *   sa[1..n]    第 i 小的后缀的起始位置（0 基），sa[0] 恒为 0 未使用；
 *   rank[i]     起始位置 i 的后缀排名（1 基）；
 *   height[1..n]  height[i] = LCP(sa[i-1], sa[i])，height[1] = 0。
 */
class SuffixArray {
  int n_;
  std::vector<int> cnt_;

 public:
  std::vector<int> sa, rank, height;

  /**
   * @brief 由字符串 s 直接构造后缀数组。
   * @param s 输入字符串（0 基）
   */
  explicit SuffixArray(const std::string& s) { build(s); }

  /**
   * @brief 构建后缀数组：倍增 + 计数排序，最后用 Kasai 求 height。
   * @param s 输入字符串（0 基）
   */
  void build(const std::string& s) {
    int n = (int)s.size();
    n_ = n;
    int m = n + 1;                     // 含哨兵，共 m 个后缀对象
    std::vector<int> a(m);
    for (int i = 0; i < n; ++i) a[i] = (int)(unsigned char)s[i] + 1;
    a[n] = 0;                          // 哨兵最小

    std::vector<int> rk(m), init_ord(m);
    // ---- 初始按第一字符计数排序 ----
    int K = 256;
    for (int i = 0; i < m; ++i) init_ord[i] = i;
    cnt_.assign(K + 1, 0);
    for (int x : a) ++cnt_[x];
    for (int i = 1; i <= K; ++i) cnt_[i] += cnt_[i - 1];
    for (int i = m - 1; i >= 0; --i) init_ord[--cnt_[a[i]]] = i;
    // 初始排名（字符类）
    int cls = 0;
    rk[init_ord[0]] = 0;
    for (int i = 1; i < m; ++i) {
      if (a[init_ord[i]] == a[init_ord[i - 1]]) rk[init_ord[i]] = cls;
      else rk[init_ord[i]] = ++cls;
    }
    // ---- 倍增 ----
    for (int k = 1; k < m && cls < m - 1; k <<= 1) {
      const int K2 = cls;
      std::vector<int> ord(m);
      for (int i = 0; i < m; ++i) ord[i] = i;
      // 先按第二关键字（i+k 处排名，越界视为 -1 排最前）稳定排序
      std::vector<int> key2(m);
      for (int i = 0; i < m; ++i) key2[i] = (i + k < m) ? rk[i + k] + 1 : 0;
      cnt_.assign(K2 + 2, 0);
      for (int x : key2) ++cnt_[x];
      for (int i = 1; i <= K2 + 1; ++i) cnt_[i] += cnt_[i - 1];
      for (int i = m - 1; i >= 0; --i) ord[--cnt_[key2[i]]] = i;
      // 再按第一关键字 rk[i] 稳定排序
      cnt_.assign(cls + 2, 0);
      for (int i = 0; i < m; ++i) ++cnt_[rk[i]];
      for (int i = 1; i <= cls + 1; ++i) cnt_[i] += cnt_[i - 1];
      std::vector<int> tmp(m);
      for (int i = m - 1; i >= 0; --i) tmp[--cnt_[rk[ord[i]]]] = ord[i];
      ord.swap(tmp);
      // 重新分配类
      std::vector<int> nrk(m);
      nrk[ord[0]] = 0;
      int cnew = 0;
      for (int i = 1; i < m; ++i) {
        int x = ord[i - 1], y = ord[i];
        if (rk[x] == rk[y] &&
            ((x + k < m ? rk[x + k] : -1) == (y + k < m ? rk[y + k] : -1)))
          nrk[y] = cnew;
        else nrk[y] = ++cnew;
      }
      cls = cnew;
      rk.swap(nrk);
    }
    // init_ord 已无意义，最终顺序存于 rk 对应的排序后数组；
    // 重新生成最终顺序 ord（含哨兵）：
    std::vector<int> ord2(m);
    for (int i = 0; i < m; ++i) ord2[i] = i;
    {
      // 按 rk 排序即最终后缀顺序（哨兵排最前）
      cnt_.assign(cls + 2, 0);
      for (int i = 0; i < m; ++i) ++cnt_[rk[i]];
      for (int i = 1; i <= cls + 1; ++i) cnt_[i] += cnt_[i - 1];
      std::vector<int> tmp(m);
      for (int i = m - 1; i >= 0; --i) tmp[--cnt_[rk[ord2[i]]]] = ord2[i];
      ord2.swap(tmp);
    }
    // ord2[0] 为哨兵后缀
    sa.assign(n + 1, 0);
    rank.assign(n + 1, 0);
    for (int p = 1; p <= n; ++p) {
      int pos = ord2[p];
      sa[p] = pos;
      rank[pos] = p;
    }
    // ---- Kasai 求 height ----
    height.assign(n + 1, 0);
    int h = 0;
    for (int i = 0; i < n; ++i) {
      if (rank[i] == 1) continue;
      int j = sa[rank[i] - 1];
      while (i + h < n && j + h < n && s[i + h] == s[j + h]) ++h;
      height[rank[i]] = h;
      if (h) --h;
    }
  }

  /// 返回字符串长度 n
  int size() const { return n_; }

  /**
   * @brief 位置 i 与 j 两后缀的 LCP 长度（朴素区间 height 最小值，O(n)；需要 O(1) 请配 ST 表）。
   * @param i 后缀起始位置（0 基）
   * @param j 后缀起始位置（0 基）
   * @return 最长公共前缀长度
   */
  int lcp(int i, int j) const {
    if (i == j) return n_ - i;
    int ri = rank[i], rj = rank[j];
    if (ri > rj) std::swap(ri, rj);
    // 区间 height 最小值：[ri+1, rj]（朴素 O(n)；需要 O(1) 请配 ST 表）
    int res = 0x7fffffff;
    for (int p = ri + 1; p <= rj; ++p) res = std::min(res, height[p]);
    return res;
  }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_SUFFIX_ARRAY_HPP