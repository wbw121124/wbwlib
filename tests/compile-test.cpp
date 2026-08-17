// wbwlib 冒烟测试：聚合编译 + 各模块核心功能断言
// 编译：g++ -O2 -std=c++14 -Wall -Wextra -I <仓库根> tests/compile-test.cpp
// 覆盖 plan.md「已知待办」中需运行时验证的项（Splay/FHQ、LiChaoTree、SA/SAM/PAM、
// 虚树、LCT、HLD、换根 DP、WeightedLCA、BigInt 等）。

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <functional>
#include <algorithm>
#include <random>
#include <iostream>

#include "wbwlib.h"

using namespace wbwlib;
using wbwlib::i64;
using wbwlib::u32;
using wbwlib::u64;

// ---- segment_tree 测试用的策略函数（须为外部链接） ----
static void st_max_pushup(i64& o, const i64& a, const i64& b) { o = (std::max)(a, b); }
static void st_max_addtag(i64& o, const i64& t, int, int) { o += t; }
static i64 st_max_merge(const i64& a, const i64& b) { return (std::max)(a, b); }
static i64 st_max_null() { return -(1LL << 60); }
static void st_as_addtag(i64& o, const i64& t, int l, int r) { o = t * i64(r - l + 1); }
static void st_as_compose(i64& o, const i64& t) { o = t; }

static int g_cnt = 0, g_fail = 0;

#define CHECK(cond)                                                        \
  do {                                                                     \
    ++g_cnt;                                                               \
    if (!(cond)) {                                                         \
      ++g_fail;                                                            \
      std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);          \
    }                                                                      \
  } while (0)

#define CHECK_EQ(a, b)                                                     \
  do {                                                                     \
    ++g_cnt;                                                               \
    const auto va = (a);                                                   \
    const auto vb = static_cast<::wbwlib::decay_t<decltype(va)>>(b);       \
    if (!(va == vb)) {                                                     \
      ++g_fail;                                                            \
      std::printf("FAIL %s:%d  %s == %s\n", __FILE__, __LINE__, #a, #b);   \
    }                                                                      \
  } while (0)

#define CHECK_NEAR(a, b, eps)                                              \
  do {                                                                     \
    ++g_cnt;                                                               \
    long double va = (a), vb = (b);                                        \
    if (fabsl(va - vb) > (eps)) {                                          \
      ++g_fail;                                                            \
      std::printf("FAIL %s:%d  %s ~= %s\n", __FILE__, __LINE__, #a, #b);   \
    }                                                                      \
  } while (0)

// ================= core =================
static void test_core() {
  std::printf("  c1 utils\n"); std::fflush(stdout);
  // utils
  std::vector<i64> a = {3, 1, 4, 1};
  auto r = core::discretize(a);
  CHECK(r == (std::vector<i64>{1, 0, 2, 0}));
  auto df = core::discretize_full(a);
  CHECK(df.first == (std::vector<i64>{1, 0, 2, 0}));
  CHECK(df.second == (std::vector<i64>{1, 3, 4}));
  CHECK_EQ(core::wmin(3, 1, 4), 1);
  CHECK_EQ(core::wmax(3, 1, 4), 4);
  CHECK_EQ(core::clamp(7, 1, 5), 5);
  CHECK_EQ(core::clamp(-2, 1, 5), 1);
  std::printf("  c2 random\n"); std::fflush(stdout);

  // random：固定种子应与 std::mt19937_64 一致
  core::Rand rnd(42);
  std::mt19937_64 m(42);
  CHECK_EQ(rnd.next(), m());
  CHECK_EQ(rnd.below(10) < 10, true);
  CHECK(rnd.range(5, 5) == 5);
  std::printf("  c3 hash\n"); std::fflush(stdout);

  // hash：可用的 unordered_map 哈希器
  core::umap<i64, int> mp;
  mp[7] = 3;
  CHECK_EQ(mp[7], 3);
  core::uset<std::pair<int, int>, core::pair_hash> st;
  st.insert({1, 2});
  CHECK(st.count({1, 2}) == 1);
  std::printf("  c4 fastio\n"); std::fflush(stdout);

  // FastIO：文件重定向读写
  {
    std::FILE* f = std::fopen("io_tmp_in.txt", "w");
    std::fprintf(f, "123 -456 abc\n42\n");
    std::fclose(f);
    {
      core::FastIO io("io_tmp_in.txt", "io_tmp_out.txt");
      int x, y;
      std::string s;
      char c;
      i64 z;
      io.read(x);
      io.read(y);
      io.read(s);
      io.read(c);
      io.read(z);
      CHECK_EQ(x, 123);
      CHECK_EQ(y, -456);
      CHECK(s == "abc");
      CHECK_EQ(c, '\n');
      CHECK_EQ(z, 42);
      io.write(x).sp().writeln(y);
    }  // 析构时 flush
    std::FILE* f2 = std::fopen("io_tmp_out.txt", "r");
    char buf[64];
    char* got = std::fgets(buf, sizeof buf, f2);
    std::fclose(f2);
    CHECK(got != nullptr && std::string(buf) == "123 -456\n");
    std::remove("io_tmp_in.txt");
    std::remove("io_tmp_out.txt");
  }

  // Stopwatch 基本可用性
  core::Stopwatch sw;
  (void)sw.ms();
}

// ================= math =================
static void test_math() {
  using namespace math;
  std::printf("  m1 numthy\n"); std::fflush(stdout);
  // number-theory
  CHECK_EQ(gcd(12, 18), 6);
  CHECK_EQ(lcm(4, 6), 12);
  CHECK_EQ(qpow<i64>(2, 10, 1000), 24);
  CHECK_EQ(mul_mod<i64>(1000000007, 1000000009, 998244353), (i64)1000000007 * 1000000009 % 998244353);
  i64 x, y;
  CHECK_EQ(ext_gcd<i64>(3, 5, x, y), 1);
  CHECK_EQ(inv(3, 7), 5);
  CHECK_EQ(phi(12), 4);
  CHECK_EQ(crt<int>({2, 3}, {3, 5}, 15), 8);
  auto er = excrt({2, 3}, {3, 5});
  CHECK(er.valid);
  CHECK_EQ(er.x, 8);
  CHECK_EQ(sum_floor(10), 27);
  auto blk = floor_blocks(10);
  CHECK_EQ(blk[0].l, 1);
  CHECK_EQ(blk[0].r, 1);
  std::printf("  m2 primes\n"); std::fflush(stdout);

  // primes
  auto ps = sieve(20);
  CHECK_EQ((int)ps.size(), 8);
  CHECK_EQ(ps.back(), 19);
  auto ls = linear_sieve(10);
  CHECK_EQ(ls.phi[8], 4);
  CHECK_EQ(ls.mu[6], 1);
  CHECK(is_prime64(1000000007));
  CHECK(!is_prime64(1000000000));
  CHECK(is_prime64(2305843009213693951ULL));  // 2^61-1
  auto fs = factorize(84);
  CHECK(fs == (std::vector<u64>{2, 2, 3, 7}));
  auto fe = factor_exp(84);
  CHECK_EQ(fe[0].first, 2);
  CHECK_EQ(fe[0].second, 2);
  CHECK_EQ(fe[1].first, 3);
  std::printf("  m3 modular\n"); std::fflush(stdout);

  // modular
  using M = modint<998244353>;
  M p = 3, q = 5;
  CHECK_EQ((p * q + 9).val(), 24);
  CHECK_EQ(M(10).pow(5).val(), 100000);
  CHECK_EQ(M(2).inv().val(), 499122177);
  CHECK_EQ((M(100) - 200).val(), 998244253);
  modint_dyn::set_mod(1000000007);
  modint_dyn dm(2);
  CHECK_EQ((dm * 3).val(), 6);
  CHECK_EQ(modint_dyn(10).pow(9).val(), 1000000000);
  std::printf("  m4 comb\n"); std::fflush(stdout);

  // combinatorics
  using Mc = modint<1000000007>;
  Comb<Mc> cb(100);
  CHECK_EQ(cb.C(10, 3).val(), 120);
  CHECK_EQ(cb.P(10, 3).val(), 720);
  CHECK_EQ(catalan(5, cb).val(), 42);
  CHECK_EQ(stirling2<Mc>(5, 2).val(), 15);
  CHECK_EQ(derangement<Mc>(4).val(), 9);
  std::printf("  m5 matrix\n"); std::fflush(stdout);

  // matrix（斐波那契矩阵快速幂）
  Mat<Mc> A(2, 2);
  A(0, 0) = Mc(1); A(0, 1) = Mc(1);
  A(1, 0) = Mc(1); A(1, 1) = Mc(0);
  auto P = A.pow(10);
  CHECK_EQ(P(0, 0).val(), 89);
  CHECK_EQ(P(1, 0).val(), 55);
  std::vector<Mc> v = {Mc(1), Mc(1)};
  auto Pv = A * v;
  CHECK_EQ(Pv[0].val(), 2);
  std::printf("  m6 linalg\n"); std::fflush(stdout);

  // linalg
  using Mg = modint<998244353>;
  using Ml = modint<1000000007>;
  std::vector<std::vector<Mg>> G2 = {{Mg(1), Mg(2), Mg(3)}, {Mg(2), Mg(4), Mg(6)}};
  CHECK_EQ(gauss_elim(G2), 1);
  std::printf("    m6a\n"); std::fflush(stdout);
  std::vector<std::vector<Mg>> M3 = {{Mg(1), Mg(2)}, {Mg(3), Mg(4)}};
  CHECK_EQ(det(M3).val(), 998244351);  // -2 mod
  std::printf("    m6b\n"); std::fflush(stdout);
  std::vector<std::vector<Ml>> A2 = {{Ml(2), Ml(1)}, {Ml(1), Ml(1)}};
  std::vector<Ml> b2 = {Ml(5), Ml(3)}, x2;
  CHECK_EQ(solve_linear(A2, b2, x2), 0);
  std::printf("    m6c\n"); std::fflush(stdout);
  CHECK_EQ(x2[0].val(), 2);
  CHECK_EQ(x2[1].val(), 1);
  std::printf("  m7 basis\n"); std::fflush(stdout);

  // XorBasis
  XorBasis xb;
  xb.insert(3);
  xb.insert(5);
  CHECK_EQ(xb.max_xor(), 6);
  CHECK(xb.has(6));
  CHECK(!xb.has(7));
  CHECK_EQ(xb.rank(), 2);
  xb.build_kth();
  CHECK_EQ(xb.kth_min(1), 0);
  CHECK_EQ(xb.kth_min(4), 6);
  std::printf("  m8 misc\n"); std::fflush(stdout);

  // bsgs / floor-sum / primitive-root / fraction
  CHECK_EQ(bsgs(2, 9, 19), 8);
  CHECK_EQ(bsgs(3, 1, 7), 0);
  CHECK_EQ(bsgs(2, 3, 7), -1);
  CHECK_EQ(floor_sum(4, 5, 2, 1), 2);
  CHECK_EQ(primitive_root(7), 3);
  CHECK_EQ(count_primitive_roots(7), 2);
  Fraction fr(1, 3), fr2(2, 6);
  CHECK(fr == fr2);
  auto frc = fr + 2;
  CHECK_EQ(frc.num(), 7);
  CHECK_EQ(frc.den(), 3);
  CHECK(Fraction(1, 2) < Fraction(2, 3));
  CHECK_EQ((Fraction(1, 2) * Fraction(3, 4)).num(), 3);
  std::printf("  m9 fourier\n"); std::fflush(stdout);

  // fourier：FFT 与 NTT 卷积
  std::vector<double> fa = {1, 2}, fb = {3, 4};
  auto fc = convolution_fft(fa, fb);
  CHECK_EQ((int)fc.size(), 3);
  CHECK_NEAR(fc[0], 3.0, 1e-9);
  CHECK_NEAR(fc[1], 10.0, 1e-9);
  CHECK_NEAR(fc[2], 8.0, 1e-9);
  std::vector<M> ma = {M(1), M(2)}, mb = {M(3), M(4)};
  auto mc = convolution_ntt(ma, mb);
  CHECK_EQ(mc[0].val(), 3);
  CHECK_EQ(mc[1].val(), 10);
  CHECK_EQ(mc[2].val(), 8);
  std::printf("  ma fwt\n"); std::fflush(stdout);

  // fwt：or/and/xor 卷积（规模 2 手算）
  auto xc = convolution_xor(ma, mb);
  CHECK_EQ(xc[0].val(), 11);  // 1*3+2*4
  CHECK_EQ(xc[1].val(), 10);  // 1*4+2*3
  auto oc = convolution_or(ma, mb);
  CHECK_EQ(oc[0].val(), 3);
  CHECK_EQ(oc[1].val(), 18);
  auto ac = convolution_and(ma, mb);
  CHECK_EQ(ac[0].val(), 13);
  CHECK_EQ(ac[1].val(), 8);
  std::printf("  mb poly\n"); std::fflush(stdout);

  // polynomial：1/(1+x+2x^2) = 1 - x - x^2 + 3x^3 + ...
  using pm = math::poly_mod;
  std::vector<pm> f = {pm(1), pm(1), pm(2)};
  auto g = poly::invf(f, 6);
  std::printf("    p1\n"); std::fflush(stdout);
  CHECK_EQ(g[0].val(), 1);
  CHECK_EQ(g[1].val(), 998244352);
  CHECK_EQ(g[2].val(), 998244352);
  CHECK_EQ(g[3].val(), 3);
  std::printf("    p2\n"); std::fflush(stdout);
  auto lf = poly::lnf(std::vector<pm>{pm(1), pm(2)}, 5);  // ln(1+2x)
  CHECK_EQ(lf[0].val(), 0);
  CHECK_EQ(lf[1].val(), 2);
  CHECK_EQ(lf[2].val(), 998244351);  // -2
  std::printf("    p3\n"); std::fflush(stdout);
  auto ef = poly::expf(std::vector<pm>{pm(0), pm(1)}, 4);  // exp(x)
  CHECK_EQ(ef[0].val(), 1);
  CHECK_EQ(ef[1].val(), 1);
  CHECK_EQ(ef[2].val(), 499122177);  // 1/2
  std::printf("    p4\n"); std::fflush(stdout);
  std::vector<pm> qq, rr;
  poly::divmod(std::vector<pm>{pm(1), pm(2), pm(3)},
               std::vector<pm>{pm(1), pm(1)}, qq, rr);
  std::printf("    p5\n"); std::fflush(stdout);
  CHECK_EQ(qq[0].val(), 998244352);  // -1
  CHECK_EQ(qq[1].val(), 3);
  CHECK_EQ(rr[0].val(), 2);
}

// ================= datastruct =================
static void test_ds() {
  using namespace ds;
  // Fenwick
  Fenwick<i64> bit(5);
  bit.add(2, 10);
  bit.add(5, 3);
  CHECK_EQ(bit.sum(4), 10);
  CHECK_EQ(bit.range_sum(2, 5), 13);
  CHECK_EQ(bit.lower_bound(10), 2);
  // RangeBIT 区间加
  RangeBIT<i64> rbit(5);
  rbit.add(2, 4, 10);
  CHECK_EQ(rbit.sum(1), 0);
  CHECK_EQ(rbit.sum(3), 20);
  CHECK_EQ(rbit.range_sum(2, 5), 30);
  // Fenwick2D
  Fenwick2D<i64> b2(3, 3);
  b2.add(1, 1, 1);
  b2.add(2, 2, 2);
  CHECK_EQ(b2.range_sum(1, 1, 2, 2), 3);

  // SegTree
  using Op = std::function<i64(i64, i64)>;
  SegTree<i64, Op> st(5, [](i64 a, i64 b) { return a + b; }, 0);
  st.update(2, 10);
  st.update(4, 3);
  CHECK_EQ(st.query(1, 5), 13);
  st.set(2, 1);
  CHECK_EQ(st.query(1, 5), 4);
  // LazySeg
  LazySeg<i64> lz(5);
  lz.build(std::vector<i64>{0, 1, 2, 3, 4, 5});
  lz.add(2, 4, 10);          // {1,12,13,14,5}
  lz.assign(3, 5, 100);      // {1,12,100,100,100}
  CHECK_EQ(lz.sum(1, 5), 313);
  CHECK_EQ(lz.max(1, 5), 100);
  // SegBeats
  SegBeats<i64> sb(5);
  sb.build(std::vector<i64>{0, 3, 1, 4, 1, 5});
  sb.chmin(1, 5, 2);         // {2,1,2,1,2}
  sb.add(1, 3, 1);           // {3,2,3,1,2}
  CHECK_EQ(sb.sum(1, 5), 11);
  CHECK_EQ(sb.max(1, 5), 3);

  // PersistentSegTree：区间第 k 小（静态，1 基）
  PersistentSegTree<int> pst;
  pst.build(7);
  std::vector<int> arr = {1, 5, 2, 6, 3, 7, 4};
  for (int v : arr) pst.update_latest(v, 1);
  CHECK_EQ(pst.query_kth(pst.roots[1], pst.roots[5], 2), 3);   // 区间[2,5]={5,2,6,3} 第2小=3
  CHECK_EQ(pst.query_count(pst.roots[1], pst.roots[5], 2, 4), 2);  // 值域[2,4]内元素数

  // SparseTable（区间 min）
  std::vector<i64> sa = {0, 3, 1, 4, 1, 5};
  SparseTable<i64, std::function<i64(i64, i64)>> spt(sa,
      [](i64 x, i64 y) { return std::min(x, y); });
  CHECK_EQ(spt.query(2, 5), 1);

  // FHQTreap
  FHQTreap<int> tr;
  tr.insert(5); tr.insert(3); tr.insert(8); tr.insert(3);
  CHECK_EQ(tr.size(), 4);
  CHECK_EQ(tr.kth(1), 3);
  CHECK_EQ(tr.kth(4), 8);
  CHECK_EQ(tr.order_of_key(3), 0);
  CHECK_EQ(tr.order_of_key(4), 2);
  tr.erase(3);
  CHECK_EQ(tr.order_of_key(4), 1);
  CHECK(tr.has(3) && !tr.has(99));
  // ImplicitTreap
  ImplicitTreap<i64> seq(std::vector<i64>{1, 2, 3, 4});
  seq.reverse(2, 3);        // {1,3,2,4}
  seq.add(1, 2, 10);        // {11,13,2,4}
  CHECK_EQ(seq.sum(1, 4), 30);
  seq.insert(2, 100);       // {11,100,13,2,4}
  seq.erase(2);             // {11,13,2,4}
  CHECK(seq.to_vector() == (std::vector<i64>{11, 13, 2, 4}));
  CHECK_EQ(seq.get(3), 2);

  // Splay
  Splay<i64> sp(std::vector<i64>{1, 2, 3});
  sp.reverse(1, 3);         // {3,2,1}
  sp.add(1, 2, 5);          // {8,7,1}
  CHECK_EQ(sp.sum(1, 3), 16);
  CHECK_EQ(sp.get(1), 8);
  sp.insert(2, 9);          // {8,9,7,1}
  sp.erase(1);              // {9,7,1}
  CHECK(sp.to_vector() == (std::vector<i64>{9, 7, 1}));

  // LeftistHeap
  LeftistHeap<int> h1, h2;
  h1.push(3); h1.push(1);
  h2.push(5);
  h1.merge(h2);
  CHECK_EQ(h1.top(), 1);
  CHECK_EQ(h1.pop(), 1);
  CHECK_EQ(h1.pop(), 3);
  CHECK_EQ(h1.pop(), 5);
  CHECK(h1.empty());

  // DSU 家族
  DSU d(5);
  d.unite(1, 2);
  d.unite(2, 3);
  CHECK(d.same(1, 3));
  CHECK(!d.same(1, 4));
  DSUWave dw(5);
  dw.unite(1, 2);
  dw.unite(2, 3);
  CHECK_EQ(dw.size_of(1), 3);
  WeightedDSU wd(5);
  wd.unite(1, 2, 3);        // value(2)-value(1)=3
  wd.unite(2, 3, 2);        // value(3)-value(2)=2
  CHECK_EQ(wd.diff(1, 3), 5);
  CHECK(!wd.unite(1, 3, 4));
  RollbackDSU rd(4);
  int snap = rd.snapshot();
  rd.unite(1, 2);
  CHECK(rd.same(1, 2));
  rd.rollback_to(snap);
  CHECK(!rd.same(1, 2));

  // 莫队：区间颜色数
  std::vector<int> cval = {0, 1, 2, 1, 3, 1, 2, 3};
  int cnt[4] = {0, 0, 0, 0}, cur = 0;
  auto add = [&](int idx) { if (++cnt[cval[idx]] == 1) ++cur; };
  auto del = [&](int idx) { if (--cnt[cval[idx]] == 0) --cur; };
  auto get = [&]() { return cur; };
  std::vector<std::pair<int, int>> qs = {{1, 3}, {2, 6}, {1, 7}};
  auto ans = mo_queries(qs, add, del, get, 7);
  CHECK_EQ(ans[0], 2);
  CHECK_EQ(ans[1], 3);
  CHECK_EQ(ans[2], 3);

  // 分块
  SqrtDecomp<i64> sd(std::vector<i64>{0, 1, 2, 3, 4, 5});
  sd.add(2, 4, 10);         // {1,12,13,14,5}
  CHECK_EQ(sd.sum(1, 5), 45);
  CHECK_EQ(sd.max(2, 4), 14);

  // 单调栈/队列
std::vector<i64> ma = {0, 3, 1, 4, 1, 5, 2};   // 1 基，6 个数据
  std::printf("    d10\n"); std::fflush(stdout);
  CHECK(prev_smaller(ma) == (std::vector<int>{0, 0, 0, 2, 0, 4, 4}));
  CHECK(sliding_window_min(ma, 3) == (std::vector<i64>{1, 1, 1, 1}));
  CHECK(next_greater(ma) == (std::vector<int>{7, 3, 3, 5, 5, 7, 7, 7}));

  // LiChaoTree（默认求最大）
  LiChaoTree<i64, 1, 1000000> lct;
  lct.add_line(2, 3);
  lct.add_line(-1, 100);
  CHECK_EQ(lct.query(4), 96);
  CHECK_EQ(lct.query(1), 99);

  // 笛卡尔树（小根）
  std::vector<int> ca = {0, 3, 1, 2, 4};
  auto ct = cartesian_tree(ca);
  CHECK_EQ(ct.root, 2);
  CHECK_EQ(ct.l[2], 1);
  CHECK_EQ(ct.r[2], 3);
CHECK_EQ(ct.r[3], 4);
  CHECK_EQ(ct.fa[3], 2);

  // 泛型懒标记线段树（Tval/Ttag 策略模板）
  {
    segment_tree<> seg(5);                    // 区间加 + 区间和（全默认）
    std::vector<i64> sa = {0, 1, 2, 3, 4, 5}; // a[1..5]
    seg.build(sa);
    seg.range_apply(1, 3, 2);                 // [3,4,5,4,5]
    CHECK_EQ(seg.query(1, 5), 21);
    CHECK_EQ(seg.query(2, 4), 13);
    CHECK_EQ(seg.all(), 21);
    seg.range_apply(1, 5, 1);                 // [4,5,6,5,6]
    CHECK_EQ(seg.query(3, 3), 6);
    seg.set(5, 0);                            // [4,5,6,5,0]
    CHECK_EQ(seg.query(1, 5), 20);

    // 区间最大值 + 区间加
    segment_tree<i64, i64, st_max_pushup, st_max_addtag,
                 segdetail::def_hastag<i64>, st_max_merge, st_max_null> mx(5);
    std::vector<i64> ma2 = {0, 1, 2, 3, 4, 5};
    mx.build(ma2);
    mx.range_apply(2, 4, 5);                  // [1,7,8,9,5]
    CHECK_EQ(mx.query(1, 5), 9);
    CHECK_EQ(mx.query(2, 2), 7);

    // 区间赋值 + 区间和（compose 覆盖）
    segment_tree<i64, i64, segdetail::def_pushup<i64>, st_as_addtag,
                 segdetail::def_hastag<i64>, segdetail::def_merge<i64>,
                 segdetail::def_nullval<i64>, st_as_compose> as(5);
    as.build(sa);
    as.range_apply(1, 3, 7);                  // [7,7,7,4,5]
    as.range_apply(2, 4, 5);                  // [7,5,5,5,5]
    CHECK_EQ(as.query(1, 5), 27);
    CHECK_EQ(as.query(2, 5), 20);
  }
}

// ================= string =================
static void test_str() {
  using namespace str;
  // KMP
  auto pi = prefix_function("ababa");
  CHECK(pi == (std::vector<int>{0, 0, 1, 2, 3}));
  auto pos = kmp_search("aba", "ababa");
  CHECK(pos == (std::vector<int>{3, 5}));
  CHECK_EQ(min_cycle("abcabc"), 3);
  CHECK_EQ(min_cycle("aaaa"), 1);

  // Z
  auto z = z_function("ababa");
  CHECK_EQ(z[2], 3);
  CHECK_EQ(z[4], 1);

  // Manacher
  auto d = manacher("ababa");
  CHECK_EQ(d.first[2], 3);
  CHECK_EQ(d.second[0], 0);
  CHECK_EQ(count_distinct_palindromes("ababa"), 5);

  // Trie / 01Trie
  Trie<> tr;
  tr.insert("abc");
  tr.insert("abc");
  tr.insert("abd");
  CHECK_EQ(tr.count("abc"), 2);
  CHECK_EQ(tr.count_prefix("ab"), 3);
  CHECK(tr.search("abd"));
  Trie01<> t1;
  t1.insert(5);
  t1.insert(3);
  CHECK_EQ(t1.xor_max(8), 13);

  // AC 自动机："ushershe" 中 he×2 + she×1
  ACAutomaton<> ac;
  ac.insert("he");
  ac.insert("she");
  ac.build();
  CHECK_EQ(ac.search("ushershe"), 4);

  // 后缀数组
  SuffixArray SA("banana");
  CHECK_EQ(SA.size(), 6);
  CHECK_EQ(SA.sa[SA.rank[2]], 2);
CHECK_EQ(SA.lcp(2, 4), 2);   // 0 基："nana" 与 "na"
  CHECK_EQ(SA.lcp(1, 3), 3);   // 0 基："anana" 与 "ana"

  // 后缀自动机
  SAM<> sam;
  sam.build("ababa");
  sam.build_cnt();
  CHECK_EQ(sam.distinct_substr(), 9);
  CHECK_EQ(sam.count_occurrence("aba"), 2);

  // 回文自动机
  PAM<> pam;
  pam.build("ababa");
  CHECK_EQ(pam.distinct(), 5);
  CHECK_EQ(pam.len(0), -1);
  CHECK_EQ(pam.len(1), 0);

  // 字符串哈希
  StringHash<> hs("ababa");
  CHECK(hs.sub(1, 3) == hs.sub(3, 5));
  CHECK(hs.sub(1, 2) != hs.sub(2, 3));

  // 最小表示法
  CHECK_EQ(minimal_rotation("bca"), 2);
  CHECK_EQ(minimal_rotation("aaaa"), 0);
  CHECK_EQ(minimal_rotation("banana"), 5);
}

// ================= graph =================
static void test_graph() {
  using namespace graph;
  // 最短路
  WAdj<i64> g(4);
  g.add(1, 2, 2);
  g.add(1, 3, 5);
  g.add(2, 3, 1);
  g.add(2, 4, 7);
  g.add(3, 4, 2);
  auto dist = dijkstra(4, 1, g);
  CHECK_EQ(dist[4], 5);        // 1-2-3-4 = 2+1+2
  WAdj<i64> g2(3);
  g2.add(1, 2, 5);
  g2.add(1, 3, -2);
  g2.add(3, 2, 1);
  std::vector<i64> sd;
  CHECK(spfa(3, 1, g2, sd));
  CHECK_EQ(sd[2], -1);         // 1-3-2 = -1
  std::vector<std::vector<i64>> fl = {
      {0, 0, 0, 0},
      {0, 0, 2, 5},
      {0, 2, 0, 1},
      {0, 5, 1, 0}};
  floyd(fl);
  CHECK_EQ(fl[1][3], 3);
  WAdj<i64> g3(3);
  g3.add(1, 2, 0);
  g3.add(2, 3, 1);
  auto d01 = bfs01(3, 1, g3);
  CHECK_EQ(d01[3], 1);

  // MST
  std::vector<std::array<int, 3>> es = {{1, 2, 1}, {1, 3, 4}, {2, 3, 2}, {2, 4, 5}, {3, 4, 3}};
  auto mst = kruskal(4, es);
  CHECK(mst.ok);
  CHECK_EQ(mst.total, 6);
  CHECK_EQ(prim(4, g), 5);     // 1-2(2) 2-3(1) 3-4(2)

// 拓扑
  Adj tg(5);
  tg[1] = {2, 3};
  tg[2] = {4};
  tg[3] = {4};
  std::vector<int> order;
  CHECK(topo_sort(tg, order));
  CHECK_EQ((int)order.size(), 4);

// Tarjan
  Adj gs(4);
  gs[1] = {2};
  gs[2] = {1, 3};
  std::vector<int> scc_id;
  CHECK_EQ(tarjan_scc(gs, scc_id), 2);
Adj gc(6);
  gc[1] = {2, 3};
  gc[2] = {1, 4};
  gc[3] = {1, 5};
  gc[4] = {2};
  gc[5] = {3};
  auto cut = tarjan_cut(gc);
  CHECK_EQ(cut[1], 1);
Adj gb(4);
  gb[1] = {2};
  gb[2] = {1, 3};
  gb[3] = {2};
  std::vector<std::pair<int, int>> bri;
  tarjan_bridge(gb, bri);
  CHECK_EQ((int)bri.size(), 2);

// 欧拉路径（三角形）
  Adj ge(4);
  ge[1] = {2, 3};
  ge[2] = {1, 3};
  ge[3] = {1, 2};
  std::vector<int> epath;
  CHECK(euler_undirected(ge, epath));
  CHECK_EQ((int)epath.size(), 4);

  // 二分图
Adj bg(5);
  bg[1] = {2, 3};
  bg[2] = {1, 4};
  bg[3] = {1, 4};
  bg[4] = {2, 3};
  std::vector<int> color;
  CHECK(is_bipartite(bg, color));
  Adj adjL(3);
  adjL[1] = {1, 2};
  adjL[2] = {2};
  CHECK_EQ(max_matching(2, 2, adjL), 2);

  // 网络流
  Dinic<i64> din(4);
  din.add_edge(1, 2, 3);
  din.add_edge(1, 3, 2);
  din.add_edge(2, 3, 1);
  din.add_edge(2, 4, 2);
  din.add_edge(3, 4, 3);
  CHECK_EQ(din.maxflow(1, 4), 5);
  auto scut = din.mincut(1, 4);
  CHECK_EQ((int)scut.size(), 5);

  // 费用流
  MinCostFlow mcf(4);
  mcf.add_edge(1, 2, 2, 1);
  mcf.add_edge(2, 4, 2, 1);
  mcf.add_edge(1, 3, 2, 2);
  mcf.add_edge(3, 4, 2, 2);
  auto mcr = mcf.min_cost_flow(1, 4);
  CHECK_EQ(mcr.first, 4);
  CHECK_EQ(mcr.second, 12);

  // LCA 与树上距离
Adj tree(5);
  tree[1] = {2, 3};
  tree[2] = {1, 4};
  tree[3] = {1};
  tree[4] = {2};
  LCA lc(tree, 1);
  CHECK_EQ(lc.query(4, 3), 1);
  CHECK_EQ(lc.kth(4, 1), 2);
  CHECK(lc.is_ancestor(1, 4));
  WAdj<i64> wt(4);
  wt.add_bidir(1, 2, 3);
  wt.add_bidir(1, 3, 5);
  wt.add_bidir(2, 4, 2);
  WeightedLCA<i64> wl(wt, 1);
  CHECK_EQ(wl.dist(4, 3), 10);
  CHECK_EQ(wl.get_lca(4, 3), 1);

  // 树的直径 / 中心 / 换根 DP
  auto dia = tree_diameter(tree);
  CHECK_EQ(std::get<0>(dia), 3);
  auto centers = tree_centers(tree);
  CHECK((centers[0] == 1 || centers[0] == 2));
auto merge = [](i64 a, i64 b) { return a + b; };
  auto push = [](i64 up, int, int) { return up + 1; };
  auto f = reroot_dp<i64>(4, tree, 1, (i64)0, merge, push);
  CHECK_EQ(f[1], 0);   // push 每次 +1 → f[u] = 距根深度
  CHECK_EQ(f[2], 1);
  CHECK_EQ(f[3], 1);
  CHECK_EQ(f[4], 2);

  // 树链剖分
  HLD hld(tree, 1);
  CHECK_EQ(hld.lca(4, 3), 1);
  i64 pts = 0;
  hld.path(4, 3, [&](int l, int r) { pts += r - l + 1; });
  CHECK_EQ(pts, 4);            // 路径 4-2-1-3 共 4 个点
  i64 sub = 0;
  hld.subtree(2, [&](int l, int r) { sub = r - l + 1; });
  CHECK_EQ(sub, 2);            // 子树 2 含 {2,4}

  // 虚树
  VirtualTree vt(lc);
  vt.build(std::vector<int>{2, 3, 4});
  CHECK_EQ((int)vt.nodes.size(), 4);   // 2,3,4 与 LCA(3,4)=1
  CHECK_EQ((int)vt.edges.size(), 3);

  // Link-Cut Tree
  LinkCutTree<i64> lct(5);
  lct.link(1, 2);
  lct.link(2, 3);
  lct.link(3, 4);
  lct.link(4, 5);
  lct.path_add(1, 5, 3);
  CHECK_EQ(lct.path_query(1, 5), 15);
  lct.set(3, 100);
  CHECK_EQ(lct.path_query(2, 4), 106);  // 3+100+3
  lct.cut(3, 4);
  CHECK_EQ(lct.path_query(1, 3), 106);  // 断开后 1-2-3 仍连通
  CHECK_EQ(lct.path_query(4, 5), 6);    // 4-5 仍连通
  CHECK_EQ(lct.findroot(2), lct.findroot(3));
  CHECK(lct.findroot(3) != lct.findroot(4));
}

// ================= dp / geo / misc =================
static void test_dp() {
  using namespace dp;
  MonoCHT cht;
  cht.add(0, 5);
  cht.add(1, 3);
  cht.add(2, 0);
  CHECK_EQ(cht.query(0), 0);
  CHECK_EQ(cht.query(1), 2);
  CHECK_EQ(cht.query(2), 4);
  LineContainer lc;
  lc.add_line(1, 0);
  lc.add_line(-1, 10);
  lc.add_line(2, -5);
  CHECK_EQ(lc.query(0), 10);
  CHECK_EQ(lc.query(3), 7);
  CHECK_EQ(lc.query(10), 15);
  lc.clear();
  lc.add_min_mode(1, 0);
  lc.add_min_mode(2, -1);
  lc.add_min_mode(-1, 5);
  CHECK_EQ(lc.query_min_mode(0), -1);
  CHECK_EQ(lc.query_min_mode(3), 2);
  CHECK_EQ(lc.query_min_mode(10), -5);

  // 决策单调性分治：n=5 分 2 段，代价=段长 → 最小 2
  auto cost = [](int l, int r) -> i64 { return r - l + 1; };
  auto dcdp = divide_conquer_dp(5, 2, cost);
  CHECK_EQ(dcdp[5], 5);

  // 数位 DP
  CHECK_EQ(sum_of_digits_leq("10"), 46);
  CHECK_EQ(count_mod_leq("10", 3), 4);
  CHECK_EQ(count_no_62_leq("62"), 62);

  // 状压
  CHECK_EQ(popcount(7u), 3);
  CHECK_EQ(lowbit(6u), 1);
  std::vector<int> subs;
  for_each_subset(7u, [&](u32 s) { subs.push_back((int)s); });
  CHECK_EQ((int)subs.size(), 7);   // 非空（含自身，不含 0）
  std::vector<std::vector<i64>> d(3, std::vector<i64>(3));
  d[0][1] = 1; d[1][0] = 1;
  d[1][2] = 2; d[2][1] = 2;
  d[0][2] = 3; d[2][0] = 3;
  CHECK_EQ(tsp(d), 6);
}

static void test_geo() {
  using namespace geo;
  using Pt = Point<i64>;
  Pt p1(0, 0), p2(2, 0), p3(1, 2), p4(1, 1);
  CHECK_EQ(orient(p1, p2, p3), 4);
  CHECK(orient(p1, p2, p3) > 0);
  CHECK(collinear(Pt(0, 0), Pt(1, 1), Pt(2, 2)));
  CHECK_EQ(dist2(p1, p2), 4);

  std::vector<Pt> pts = {p1, p2, p3, p4};
  auto hull = convex_hull(pts);
  CHECK_EQ((int)hull.size(), 3);
  CHECK(point_in_convex(hull, p4));
  CHECK(!point_in_convex(hull, Pt(3, 3)));

  std::vector<Pt> poly = {p1, p2, p3};
  CHECK_EQ(signed_area2(poly), 4);
  CHECK_EQ(point_in_polygon(poly, p4), 1);
  CHECK_EQ(point_in_polygon(poly, Pt(0, 0)), 2);
  CHECK(is_convex_polygon(poly));
  CHECK_NEAR(area(poly), 2.0, 1e-12);
  CHECK_NEAR(perimeter(poly), 2.0 + std::sqrt(5.0) + std::sqrt(5.0), 1e-9);

// 极角排序（半平面序：角度升序，跨 ±π）
  std::vector<PointD> vps = {PointD(1, 0), PointD(0, 1), PointD(-1, 0), PointD(0, -1)};
  polar_sort(vps);
  for (int i = 1; i < 4; ++i)
    CHECK(normalize_angle(angle_of(vps[i - 1])) <= normalize_angle(angle_of(vps[i])));
  CHECK_NEAR(normalize_angle(-1.0L), 2.0L * acosl(-1.0L) - 1.0L, 1e-12);

  // 半平面交：三角形区域（逆时针三边左侧）
  std::vector<HalfPlane> hps;
  hps.push_back(HalfPlane(PointD(-1, 0), PointD(1, 0)));
  hps.push_back(HalfPlane(PointD(1, 0), PointD(0, 1)));
  hps.push_back(HalfPlane(PointD(0, 1), PointD(-1, 0)));
  auto hp = half_plane_intersection(hps);
  CHECK_EQ((int)hp.size(), 3);
  std::vector<HalfPlane> empty_hp;
  empty_hp.push_back(HalfPlane(PointD(0, 0), PointD(1, 0)));
  empty_hp.push_back(HalfPlane(PointD(0, 0), PointD(-1, 0)));
  CHECK_EQ((int)half_plane_intersection(empty_hp).size(), 0);

  // 圆
  auto cc = circle_from_three_points(PointD(0, 0), PointD(2, 0), PointD(0, 2));
  CHECK_NEAR(cc.c.x, 1.0L, 1e-9);
  CHECK_NEAR(cc.c.y, 1.0L, 1e-9);
  CHECK_NEAR(cc.r, std::sqrt(2.0L), 1e-9);
  auto mc = min_enclosing_circle(
      {PointD(0, 0), PointD(2, 0), PointD(0, 2), PointD(1, 1)});
  CHECK_NEAR(mc.r, std::sqrt(2.0L), 1e-9);
  CHECK(cc.contains(PointD(1, 1)));
  Circle c1(PointD(0, 0), 1.0L), c2(PointD(2, 0), 1.0L);
  CHECK_EQ(circle_relation(c1, c2), 1);   // 外切
}

static void test_misc() {
  using BI = misc::BigInt;
  BI a("12345678901234567890");
  CHECK(a.to_string() == "12345678901234567890");
  BI b(12345);
  BI c = a * b;
  CHECK((c / b).to_string() == a.to_string());
  CHECK((c % a).to_string() == "0");
  CHECK((c / a).to_string() == b.to_string());
  CHECK((BI(100) / BI(7)) == BI(14));
  CHECK((BI(100) % BI(7)) == BI(2));
  CHECK((BI(100) / 7).to_string() == "14");
  CHECK_EQ(BI(100) % 7, 2);
  CHECK(BI(7).pow(10).to_string() == "282475249");
  CHECK((-BI(5)).to_string() == "-5");
  CHECK((BI(-5) * BI(3)) == BI(-15));
  CHECK((BI(-5) + BI(8)) == BI(3));
  CHECK(BI(-2).pow(3).to_string() == "-8");
  CHECK(BI(-2).pow(4).to_string() == "16");
  CHECK(BI(-5) < BI(3));
  CHECK(BI("123456789012345678901234567890") > BI("12345678901234567890123456789"));
  CHECK((BI("100000000000000000000") / BI("10000000000")).to_string() == "10000000000");
  CHECK(BI("100000000000000000000") / 10000000000 == BI("10000000000"));
  CHECK_EQ(BI("-14").mod_small(5), -4);
  CHECK(BI("-0") == BI(0));
  std::ostringstream oss;
  oss << BI("12345");
  CHECK(oss.str() == "12345");
}

// 小节名（main 的 argv[1] 用逗号分隔，只跑指定小节；空则全部）
static const char* k_sections[] = {"core", "math", "ds", "str", "graph", "dp", "geo", "misc"};
static bool g_want[8];

static bool section_enabled(const char* name, const std::string& filter) {
  if (filter.empty() || filter == "all") return true;
  size_t pos = 0;
  while (true) {
    size_t comma = filter.find(',', pos);
    std::string item = filter.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
    if (item == name) return true;
    if (comma == std::string::npos) break;
    pos = comma + 1;
  }
  return false;
}

int main(int argc, char** argv) {
  std::string filter = argc > 1 ? argv[1] : "";
  for (int i = 0; i < 8; ++i)
    g_want[i] = section_enabled(k_sections[i], filter);
  if (!filter.empty() && filter != "all") {
    for (int i = 0; i < 8; ++i)
      if (g_want[i]) std::printf("+ %s\n", k_sections[i]);
  }
  std::printf("start core\n"); std::fflush(stdout);
  if (g_want[0]) test_core();
  std::printf("start math\n"); std::fflush(stdout);
  if (g_want[1]) test_math();
  std::printf("start ds\n"); std::fflush(stdout);
  if (g_want[2]) test_ds();
  std::printf("start str\n"); std::fflush(stdout);
  if (g_want[3]) test_str();
  std::printf("start graph\n"); std::fflush(stdout);
  if (g_want[4]) test_graph();
  std::printf("start dp\n"); std::fflush(stdout);
  if (g_want[5]) test_dp();
  std::printf("start geo\n"); std::fflush(stdout);
  if (g_want[6]) test_geo();
  std::printf("start misc\n"); std::fflush(stdout);
  if (g_want[7]) test_misc();
  std::printf("%d checks, %d failures\n", g_cnt, g_fail);
  return g_fail == 0 ? 0 : 1;
}



