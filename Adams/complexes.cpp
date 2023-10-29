#include "algebras/myio.h"
#include "groebner_res.h"
#include <fmt/core.h>
#include <regex>

/****************************************************
 *                   S0
 ***************************************************/
void Coh_S0(int1d& v_degs, Mod1d& rels, int t_max)
{
    v_degs = {0};
    rels.clear();
    for (int i = 0; (1 << i) <= t_max; ++i)
        rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
}

/****************************************************
 *                   tmf
 ***************************************************/
void Coh_A_over_An(int1d& v_degs, Mod1d& rels, int n, int t_max)
{
    v_degs = {0};
    rels.clear();
    for (int i = 0; i <= n && ((1 << i) <= t_max); ++i)
        rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
}

/****************************************************
 *                   ko
 ***************************************************/
void Coh_ko(int1d& v_degs, Mod1d& rels, int t_max)
{
    v_degs = {0};
    rels.clear();
    for (int i = 0; i < 2 && ((1 << i) <= t_max); ++i)
        rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
}

/****************************************************
 *                   X2
 ***************************************************/
void Coh_X2(int1d& v_degs, Mod1d& rels, int t_max)
{
    v_degs = {0};
    rels.clear();
    for (int j = 3; j <= 5; ++j)
        for (int i = 0; (((1 << j) - 1) << i) <= t_max; ++i)
            rels.push_back(MMod(MMilnor::P(i, i + j), 0));
}

/****************************************************
 *                    Ch
 *              Cofiber of h_n, n=0,1,2,3
 ***************************************************/
void Coh_Chn(int1d& v_degs, Mod1d& rels, int n, int t_max)
{
    v_degs = {0};
    rels.clear();
    for (int i = 0; (1 << i) <= t_max; ++i)
        if (i != n)
            rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
    for (int i = 0; (1 << i) + (1 << n) <= t_max; ++i)
        rels.push_back(MMilnor::P(i, i + 1) * MMod(MMilnor::P(n, n + 1), 0));
}

/****************************************************
 *                    tmf smash Ch
 *              Cofiber of h_n, n=0,1,2
 ***************************************************/
void Coh_tmf_Chn(int1d& v_degs, Mod1d& rels, int n, int t_max)
{
    v_degs = {0};
    rels.clear();
    for (int i = 0; (1 << i) <= t_max && i <= 2; ++i)
        if (i != n)
            rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
    for (int i = 0; (1 << i) + (1 << n) <= t_max && i <= 2; ++i)
        rels.push_back(MMilnor::P(i, i + 1) * MMod(MMilnor::P(n, n + 1), 0));
}

/****************************************************
 *                     M
 ***************************************************/
void Coh_M(int1d& v_degs, Mod1d& rels, int t_max)
{
    v_degs = {0};
    rels.clear();
    Mod cell = MMod({}, 0);
    int dim = 0;
    for (int j = 0; (1 << j) - 1 <= t_max; ++j) {
        for (int i = 0; (1 << i) + dim <= t_max; ++i)
            if (i != j)
                rels.push_back(MMilnor::P(i, i + 1) * cell);
        cell = MMilnor::P(j, j + 1) * cell;
        dim += 1 << j;
    }
}

/****************************************************
 *                   j
 ***************************************************/
void Coh_j(int1d& v_degs, Mod1d& rels, int t_max)
{
    auto& Sq = MMilnor::Sq;

    v_degs = {0, 7};
    rels.clear();
    for (int i = 0; i < 3; ++i)
        rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
    Mod rel;
    rel = Sq(8) * MMod(MMilnor(), 0) + Sq(1) * MMod(MMilnor(), 1);
    rels.push_back(std::move(rel));

    rel = Sq(7) * MMod(MMilnor(), 1);
    rels.push_back(std::move(rel));

    rel = Sq(4) * (Sq(6) * MMod(MMilnor(), 1)) + Sq(6) * (Sq(4) * MMod(MMilnor(), 1));
    rels.push_back(std::move(rel));
}

/****************************************************
 *                   j/2
 ***************************************************/
void Coh_j_C2(int1d& v_degs, Mod1d& rels, int t_max)
{
    auto& Sq = MMilnor::Sq;

    v_degs = {0, 7};
    rels.clear();

    Mod i7Sq1 = Sq(8) * MMod(MMilnor(), 0) + Sq(1) * MMod(MMilnor(), 1);

    Mod rel;
    rel = Sq(2) * MMod(MMilnor(), 0);
    rels.push_back(std::move(rel));

    rel = Sq(2) * (Sq(1) * MMod(MMilnor(), 0));
    rels.push_back(std::move(rel));

    rel = Sq(4) * MMod(MMilnor(), 0);
    rels.push_back(std::move(rel));

    rel = Sq(7) * MMod(MMilnor(), 1) + Sq(6) * i7Sq1;
    rels.push_back(std::move(rel));

    rel = Sq(4) * (Sq(6) * MMod(MMilnor(), 1)) + Sq(6) * (Sq(4) * MMod(MMilnor(), 1)) + Sq(3) * (Sq(6) * i7Sq1) + Sq(6) * (Sq(3) * i7Sq1) + Sq(4) * (Sq(5) * i7Sq1) + Sq(5) * (Sq(4) * i7Sq1);
    rels.push_back(std::move(rel));
}

/****************************************************
 *                   RPn
 ***************************************************/
constexpr inline int N_MAX_RP = 261;

int Period_RP(int n)
{
    MyException::Assert(n >= 0, "n >= 0 in phi(n)");
    int phi = (n / 8) * 4, residue = n % 8;
    if (residue <= 1)
        phi += residue;
    else if (residue <= 3)
        phi += 2;
    else if (residue <= 7)
        phi += 3;
    return phi <= 31 ? 1 << phi : -1;
}

void normalize_RP(int n1, int n2, int& n1_, int& n2_)  //// TODO: normalize CP and HP
{
    n1_ = n1;
    n2_ = n2;
    int T = Period_RP(n2 - n1);
    if (T >= 0) {
        if (n1 < 0 && T < -n1 * 2) {
            int shift = ((-n1) / T + 1) * T;
            n1 += shift;
            n2 += shift;
        }
        if (n1 > 0) {
            n1_ = 1 + ((n1 - 1) % T);
            n2_ = n2 - (n1 - n1_);
        }
    }
}

void Coh_P(int1d& v_degs, Mod1d& rels, Mod1d& cell_reduced, int1d& min_rels, int n1, int n2, int t_max, const std::string& over, int hopf)
{
    size_t i_max = 0;
    if (over == "S0")
        i_max = 1ULL << 30;
    else if (over == "tmf")
        i_max = 2;
    else {
        fmt::print("over={} not supported\n", over);
        throw MyException(0x87fe2a2b, "over not supported.");
    }

    int1d v_degs2;
    int offset = std::max(0, -n1);
    for (int n = n1; n <= n2 && n <= t_max; ++n)
        v_degs2.push_back((n + offset) << hopf);

    Mod1d rels2;
    Mod tmp;
    /* Sq^{k} * x^n = (k1, n - k1) x^{n + k1} when n >= 0 and k = (k1 << hopf)
     * Sq^{k} * x^n = (k1, -n - 1) x^{n + k1} when n < 0 and k = (k1 << hopf)
     */
    for (size_t i = 0; (1 << i) <= t_max && i <= i_max; ++i) {
        int k = 1 << i;
        int k1 = k >> hopf;
        for (int n = n1; n <= n2 && ((n << hopf) + k) <= t_max; ++n) {
            Mod rel = MMilnor::Sq((uint32_t)k) * MMod(MMilnor(), n - n1);
            if (k1 && n + k1 <= n2)
                if (n >= 0 ? (k1 <= n && !(k1 & (n - k1))) : !(k1 & (-n - 1)))
                    rel.iaddP(MMod(MMilnor(), n + k1 - n1), tmp);
            rels2.push_back(std::move(rel));
        }
    }

    Groebner gb(t_max, {}, v_degs2);
    min_rels.clear();
    gb.AddRels(rels2, t_max, min_rels);
    gb.MinimizeOrderedGensRels(cell_reduced, min_rels);

    v_degs = gb.v_degs();
    rels.clear();
    for (int i : min_rels)
        rels.push_back(gb.data()[i]);
}

void Coh_X2_RP(int1d& v_degs, Mod1d& rels, Mod1d& cell_reduced, int1d& min_rels, int n1, int n2, int t_max)
{
    int1d v_degs2;
    int offset = std::max(0, -n1);
    for (int n = n1; n <= n2 && n <= t_max; ++n)
        v_degs2.push_back(n + offset);

    Mod1d rels2;
    Mod tmp;
    /* Sq^k * x^n = (k, n - k) x^{n + k} when n >= 0
     * Sq^k * x^n = (k, -n - 1) x^{n + k} when n < 0
     */
    for (size_t i = 0; (1 << i) <= t_max; ++i) {
        int k = 1 << i;
        for (int n = n1; n <= n2 && n + k <= t_max; ++n) {
            Mod rel = MMilnor::Sq((uint32_t)k) * MMod(MMilnor(), n - n1);
            if (n + k <= n2)
                if (n >= 0 ? (k <= n && !(k & (n - k))) : !(k & (-n - 1)))
                    rel.iaddP(MMod(MMilnor(), n + k - n1), tmp);
            rels2.push_back(std::move(rel));
        }
    }
    Groebner gb2(t_max, {}, v_degs2);
    gb2.AddRels(rels2, t_max);

    Mod1d rels3;
    for (int j = 3; j <= 5; ++j) {
        for (int i = 0; (((1 << j) - 1) << i) <= t_max; ++i) {
            int k = ((1 << j) - 1) << i;
            for (int n = n1; n <= n2 && n + k <= t_max; ++n) {
                Mod rel = MMilnor::P(i, i + j) * MMod(MMilnor(), n - n1);
                if (n + k <= n2 && !gb2.Reduce(rel + MMod(MMilnor(), n + k - n1)))
                    rel.iaddP(MMod(MMilnor(), n + k - n1), tmp);
                rels3.push_back(std::move(rel));
            }
        }
    }
    Groebner gb3(t_max, {}, v_degs2);
    min_rels.clear();
    gb3.AddRels(rels3, t_max, min_rels);
    gb3.MinimizeOrderedGensRels(cell_reduced, min_rels);

    v_degs = gb3.v_degs();
    rels.clear();
    for (int i : min_rels)
        rels.push_back(gb3.data()[i]);
}

void Coh_Fphi(int1d& v_degs, Mod1d& rels, Mod1d& cell_reduced, int1d& min_rels, int t_max)
{
    int1d v_degs2 = {0};
    for (int n = 1; n <= N_MAX_RP && n <= t_max; ++n)
        v_degs2.push_back(n + 1);

    Mod1d rels2;
    Mod tmp;
    /* Sq^k * x^n = (k, n - k) x^{n + k} */
    for (size_t i = 0; (1 << i) <= t_max; ++i) {
        size_t k = (size_t)1 << i;
        for (int n = 1; n <= N_MAX_RP && n + k <= t_max; ++n) {
            Mod rel = MMilnor::Sq((uint32_t)k) * MMod(MMilnor(), n);
            if (k <= n && !(k & (n - k)) && n + k <= N_MAX_RP)
                rel.iaddP(MMod(MMilnor(), n + k), tmp);
            rels2.push_back(std::move(rel));
        }
    }
    /* Sq^k * x^{-1} = (k, 1) x^{k-1} */
    for (size_t i = 0; (1 << i) <= t_max; ++i) {
        size_t k = (size_t)1 << i;

        Mod rel = MMilnor::Sq((uint32_t)k) * MMod(MMilnor(), 0);
        if (k > 1)
            rel.iaddP(MMod(MMilnor(), k - 1), tmp);
        rels2.push_back(std::move(rel));
    }

    Groebner gb(t_max, {}, v_degs2);
    min_rels.clear();
    gb.AddRels(rels2, t_max, min_rels);
    gb.MinimizeOrderedGensRels(cell_reduced, min_rels);

    v_degs = gb.v_degs();
    rels.clear();
    for (int i : min_rels)
        rels.push_back(gb.data()[i]);
}

void Coh_Fphi_n(int1d& v_degs, Mod1d& rels, Mod1d& cell_reduced, int1d& min_rels, int n_max, int t_max)
{
    int1d v_degs2 = {0};
    for (int n = 1; n <= n_max && n <= t_max; ++n)
        v_degs2.push_back(n + 1);

    Mod1d rels2;
    Mod tmp;
    /* Sq^k * x^n = (k, n - k) x^{n + k} */
    for (size_t i = 0; (1 << i) <= t_max; ++i) {
        size_t k = (size_t)1 << i;
        for (int n = 1; n <= n_max && n + k <= t_max; ++n) {
            Mod rel = MMilnor::Sq((uint32_t)k) * MMod(MMilnor(), n);
            if (k <= n && !(k & (n - k)) && n + k <= n_max)
                rel.iaddP(MMod(MMilnor(), n + k), tmp);
            rels2.push_back(std::move(rel));
        }
    }
    /* Sq^k * x^{-1} = (k, 1) x^{k-1} */
    for (size_t i = 0; (1 << i) <= t_max; ++i) {
        size_t k = (size_t)1 << i;

        Mod rel = MMilnor::Sq((uint32_t)k) * MMod(MMilnor(), 0);
        if (k > 1 && k - 1 <= n_max)
            rel.iaddP(MMod(MMilnor(), k - 1), tmp);
        rels2.push_back(std::move(rel));
    }

    Groebner gb(t_max, {}, v_degs2);
    min_rels.clear();
    gb.AddRels(rels2, t_max, min_rels);
    gb.MinimizeOrderedGensRels(cell_reduced, min_rels);

    v_degs = gb.v_degs();
    rels.clear();
    for (int i : min_rels)
        rels.push_back(gb.data()[i]);
}

int CohFromJsonV2(int1d& v_degs, Mod1d& rels, Mod1d& cell_reduced, int1d& min_rels, int t_max, const std::string& name)
{
    if (!myio::FileExists("Adams.json"))
        return -1;
    auto js = myio::load_json("Adams.json");
    try {
        auto& cws = js.at("CW_complexes");
        if (!cws.contains(name)) {
            return -1;
        }
        auto& cw_json = cws.at(name);
        if (!cw_json.contains("operations")) {
            fmt::print("missing key: operations\n");
            return -2;
        }
        int1d gen_degs;
        std::map<int, int> cells;
        std::map<int, int> indices;
        int index = 0;
        for (auto& c : cw_json.at("cells")) {
            int d = c.get<int>();
            gen_degs.push_back(d);
            ++cells[d];
            if (!ut::has(indices, d))
                indices[d] = index;
            ++index;
        }
        int1d cells_gen;
        for (auto& c : cw_json.at("cells_gen")) {
            if (c.is_number()) {
                int d = c.get<int>();
                cells_gen.push_back(indices.at(d));
            }
            else {
                int1d arr = c.get<std::vector<int>>();
                if (arr.size() < 2) {
                    fmt::print("Invalid cells_gen\n");
                    return -3;
                }
                int d = arr[0];
                for (size_t i = 1; i < arr.size(); ++i)
                    cells_gen.push_back(indices.at(d) + arr[i]);
            }
        }
        std::map<int, std::map<int, int1d>> ops;
        for (auto& op : cw_json.at("operations")) {
            int c0, v0;
            if (op[0].is_number()) {
                c0 = op[0].get<int>();
                v0 = indices.at(c0);
            }
            else {
                int1d arr = op[0].get<std::vector<int>>();
                c0 = arr[0];
                MyException::Assert(arr[1] < cells.at(c0), "arr[1] < cells.at(c0)");
                v0 = indices.at(c0) + arr[1];
            }
            int c1;
            int1d v1s;
            if (op[1].is_number()) {
                c1 = op[1].get<int>();
                v1s.push_back(indices.at(c1));
            }
            else {
                int1d arr = op[1].get<std::vector<int>>();
                c1 = arr[0];
                for (size_t i = 1; i < arr.size(); ++i) {
                    MyException::Assert(arr[i] < cells.at(c1), "arr[i] < cells.at(c1)");
                    v1s.push_back(indices.at(c1) + arr[i]);
                }
            }

            int n = c1 - c0;
            MyException::Assert(!(n & (n - 1)), "n is a power of 2");
            for (int v1 : v1s)
                ops[v0][n].push_back(v1);
        }

        Mod1d rels2;
        Mod tmp;
        for (size_t i = 0; i < gen_degs.size(); ++i) {
            for (int j = 0; (1 << j) + gen_degs[i] <= t_max; ++j) {
                Mod rel = MMilnor::P(j, j + 1) * MMod(MMilnor(), i);
                if (ut::has(ops, (int)i) && ut::has(ops.at((int)i), (int)(1 << j)))
                    for (int v1 : ops.at((int)i).at(int(1 << j)))
                        rel.iaddP(MMod(MMilnor(), v1), tmp);
                rels2.push_back(std::move(rel));
            }
        }

        Groebner gb(t_max, {}, gen_degs);
        min_rels.clear();
        gb.AddRels(rels2, t_max, min_rels);
        gb.MinimizeOrderedGensRels(cell_reduced, min_rels);

        int1d cells_gen_v2;
        for (size_t i = 0; i < cell_reduced.size(); ++i)
            if (cell_reduced[i].data.size() == 1 && cell_reduced[i].GetLead().deg_m() == 0)
                cells_gen_v2.push_back((int)i);
        MyException::Assert(cells_gen == cells_gen_v2, "cells_gen == cells_gen_v2");

        v_degs = gb.v_degs();
        ut::RemoveIf(v_degs, [t_max](int i) { return i > t_max; });
        rels.clear();
        for (int i : min_rels)
            rels.push_back(gb.data()[i]);
    }
    catch (nlohmann::detail::exception& e) {
        fmt::print("Error({:#x}) - {}\n", e.id, e.what());
        throw e;
    }
    return 0;
}

int CohFromJson(int1d& v_degs, Mod1d& rels, int t_max, const std::string& name)
{
    Mod1d cell_reduced;
    int1d min_rels;
    return CohFromJsonV2(v_degs, rels, cell_reduced, min_rels, t_max, name);
}

/****************************************************
                     # Maps
 ***************************************************/
bool IsFP(const std::string& cw, const std::string& field)
{
    std::regex is_RP_regex("^" + field + "P(?:m|)\\d+_(?:m|)\\d+$"); /* match example: RP1_4, RPm10_m4 */
    std::smatch match_RP;
    std::regex_search(cw, match_RP, is_RP_regex);
    return match_RP[0].matched;
}

bool IsFP(const std::string& cw)
{
    std::regex is_RP_regex("^(?:R|C|H)P(?:m|)\\d+_(?:m|)\\d+$"); /* match example: RP1_4, CPm10_m4 */
    std::smatch match_RP;
    std::regex_search(cw, match_RP, is_RP_regex);
    return match_RP[0].matched;
}

int IsFphi_n(const std::string& cw)
{
    std::regex regex("^Fphi(\\d+)$"); /* match example: Fphi4 */
    std::smatch match;
    std::regex_search(cw, match, regex);
    return match[0].matched ? std::stoi(match[1].str()) : 0;
}

void ParseFP(const std::string& cw, int& hopf, int& n1, int& n2)
{
    std::regex words_regex("^(R|C|H)P(m|)(\\d+)_(m|)(\\d+)$"); /* match example: CP1_4 */
    std::smatch match_P;
    std::regex_search(cw, match_P, words_regex);

    std::string field = match_P[1].str();
    hopf = field == "R" ? 0 : (field == "C" ? 1 : 2);
    n1 = std::stoi(match_P[3].str());
    n2 = std::stoi(match_P[5].str());
    if (!match_P[2].str().empty())
        n1 = -n1;
    if (!match_P[4].str().empty())
        n2 = -n2;
}

void SetCohMapFP2FP(const std::string& cw1, const std::string& cw2, std::string& from, std::string& to, Mod1d& images, int& sus, int& fil)
{
    int hopf1, hopf2, n1, n2, m1, m2;
    ParseFP(cw1, hopf1, n1, n2);
    ParseFP(cw2, hopf2, m1, m2);
    MyException::Assert(n1 <= n2 && m1 <= m2, "n1 < n2 && m1 < m2");
    MyException::Assert(hopf1 == hopf2 || hopf1 + 1 == hopf2, "hopf1 == hopf2 || hopf1 + 1 == hopf2");
    int n1_ = n1, n2_ = n2;
    int m1_ = m1, m2_ = m2;
    std::string field1 = hopf1 == 0 ? "R" : (hopf1 == 1 ? "C" : "H");
    std::string field2 = hopf2 == 0 ? "R" : (hopf2 == 1 ? "C" : "H");

    /*# Normalize cw1 */

    if (hopf1 == 0)
        normalize_RP(n1, n2, n1_, n2_);
    if (n2 - n1 == 1) {
        from = hopf1 == 0 ? "C2" : (hopf1 == 1 ? "Ceta" : "Cnu");
        n1_ = 0;
        n2_ = 1;
    }
    else if (n2 - n1 == 0) {
        from = "S0";
        n1_ = 0;
        n2_ = 0;
    }
    else
        from = fmt::format("{}P{}_{}", field1, n1_, n2_);
    if (from != cw1)
        fmt::print("We use {} instead of {} because of James periodicity\n", from, cw1);

    /*# Normalize cw2 */

    if (hopf2 == 0)
        normalize_RP(m1, m2, m1_, m2_);
    if (m2 - m1 == 1) {
        to = hopf2 == 0 ? "C2" : (hopf2 == 1 ? "Ceta" : "Cnu");
        m1_ = 0;
        m2_ = 1;
    }
    else if (m2 - m1 == 0) {
        to = "S0";
        m1_ = 0;
        m2_ = 0;
    }
    else
        to = fmt::format("{}P{}_{}", field2, m1_, m2_);
    if (to != cw2)
        fmt::print("We use {} instead of {} because of James periodicity\n", to, cw2);

    if (hopf1 == hopf2) {
        int hopf = hopf1;
        /*# Compute map */

        if (n1 == m1) { /*## subcomplex */
            MyException::Assert(n2 < m2, "n2 < m2");

            int1d v_degs_n, tmp_ind1d;
            Mod1d tmp, tmp1;
            Coh_P(v_degs_n, tmp, tmp1, tmp_ind1d, n1, n2, std::min(n2 << hopf, 256), "S0", hopf);
            int1d v_degs_m;
            Coh_P(v_degs_m, tmp, tmp1, tmp_ind1d, m1, m2, std::min(m2 << hopf, 256), "S0", hopf);

            images.clear();
            for (size_t i = 0; i < v_degs_n.size(); ++i)
                images.push_back(MMod(MMilnor(), i));
            for (size_t i = v_degs_n.size(); i < v_degs_m.size(); ++i)
                images.push_back({});
            sus = (n1_ - m1_) << hopf;
            return;
        }
        else if (n2 == m2) { /*## quotient complex */
            MyException::Assert(n1 < m1, "n1 < m1");

            int1d v_degs_n, tmp_ind1d;
            Mod1d cell_reduced_n, tmp, tmp1;
            Coh_P(v_degs_n, tmp, cell_reduced_n, tmp_ind1d, n1, n2, std::min(n2 << hopf, 256), "S0", hopf);
            int1d v_degs_m;
            Coh_P(v_degs_m, tmp, tmp1, tmp_ind1d, m1, m2, std::min(m2 << hopf, 256), "S0", hopf);

            images.clear();
            for (size_t i = 0; i < v_degs_m.size(); ++i)
                images.push_back(cell_reduced_n[size_t((v_degs_m[i] << hopf) - n1)]);
            sus = (n2_ - m2_) << hopf;
            return;
        }
        else if (n1 == m2 + 1) { /*## attaching map */
            int1d v_degs_n, tmp_ind1d;
            Mod1d cell_reduced_n, tmp, tmp1;
            Coh_P(v_degs_n, tmp, cell_reduced_n, tmp_ind1d, n1, n2, std::min(n2 << hopf, 256), "S0", hopf);
            int1d v_degs_m;
            Mod1d rels_m;
            Coh_P(v_degs_m, rels_m, tmp1, tmp_ind1d, m1, m2, std::min(n2 << hopf, 256), "S0", hopf);
            int1d v_degs_l;
            Mod1d rels_l;
            Coh_P(v_degs_l, rels_l, tmp1, tmp_ind1d, m1, n2, std::min(n2 << hopf, 256), "S0", hopf);
            Groebner gb(n2, rels_l, v_degs_l);

            images.clear();
            for (size_t i = 0; i < rels_m.size(); ++i) {
                if (gb.Reduce(rels_m[i])) {
                    int cell = ((rels_m[i].GetLead().deg_m() + v_degs_m[rels_m[i].GetLead().v()]) << hopf) + std::min(m1, 0);
                    images.push_back(cell_reduced_n[size_t(cell - n1)]);
                }
                else {
                    images.push_back({});
                }
            }
            sus = (n1_ - m2_) << hopf;
            fil = 1;
            return;
        }
        else {
            fmt::print("The map between FP is not supported\n");
            throw MyException(0x31dd0ad6, "The map between FP is not supported");
        }
    }
    else if (hopf2 == hopf1 + 1) {
        MyException::Assert(n1 == 2 * m1 - 1 && n2 == 2 * m2, "n1 == 2 * m1 - 1 && n2 == 2 * m2");

        int1d v_degs_n, tmp_ind1d;
        Mod1d cell_reduced_n, tmp, tmp1;
        Coh_P(v_degs_n, tmp, cell_reduced_n, tmp_ind1d, n1, n2, std::min(n2 << hopf1, 256), "S0", hopf1);
        int1d v_degs_m;
        Coh_P(v_degs_m, tmp, tmp1, tmp_ind1d, m1, m2, std::min(m2 << hopf2, 256), "S0", hopf2);

        images.clear();
        for (size_t i = 0; i < v_degs_m.size(); ++i)
            images.push_back(cell_reduced_n[size_t((v_degs_m[i] << hopf1) - n1)]);
        sus = (n2_ - 2 * m2_) << hopf1;
        return;
    }
}

/* CW_hn1_hn2 --> Chn2 */
void SetCohMapQ3cell_2cell(Mod1d& images, int& sus, int n1, int n2)
{
    images = {MMod(MMilnor::P(n1, n1 + 1), 0)};
    sus = 1 << n1;
    return;
}

/* Chn1 smash Chn2 --> Chn2 */
void SetCohMapQSmash_2cell(Mod1d& images, int& sus, int n1, int n2)
{
    images = {MMod(MMilnor::P(n1, n1 + 1), 0)};
    sus = 1 << n1;
    return;
}

int Coh(int1d& v_degs, Mod1d& rels, int d_max, const std::string& cw)
{
    std::smatch match;
    if (cw == "S0")
        Coh_S0(v_degs, rels, d_max);
    else if (cw == "X2")
        Coh_X2(v_degs, rels, d_max);
    else if (cw == "tmf")
        Coh_A_over_An(v_degs, rels, 2, d_max);
    else if (cw == "A_A3")
        Coh_A_over_An(v_degs, rels, 3, d_max);
    else if (cw == "A_A4")
        Coh_A_over_An(v_degs, rels, 4, d_max);
    else if (cw == "A_A5")
        Coh_A_over_An(v_degs, rels, 5, d_max);
    else if (cw == "ko")
        Coh_ko(v_degs, rels, d_max);
    else if (cw == "j")
        Coh_j(v_degs, rels, d_max);
    else if (cw == "j_C2")
        Coh_j_C2(v_degs, rels, d_max);
    else if (cw == "tmf_C2")
        Coh_tmf_Chn(v_degs, rels, 0, d_max);
    else if (cw == "tmf_Ceta")
        Coh_tmf_Chn(v_degs, rels, 1, d_max);
    else if (cw == "tmf_Cnu")
        Coh_tmf_Chn(v_degs, rels, 2, d_max);
    else if (cw == "Fphi") {
        Mod1d tmp;
        int1d tmp_int1d;
        Coh_Fphi(v_degs, rels, tmp, tmp_int1d, d_max);
    }
    else if (std::regex_search(cw, match, std::regex("^Fphi(\\d+)$")); match[0].matched) { /* Fphi_n */
        Mod1d tmp;
        int1d tmp_int1d;
        int n = std::stoi(match[1].str());
        Coh_Fphi_n(v_degs, rels, tmp, tmp_int1d, n, d_max);
    }
    else if (cw == "M") {
        Coh_M(v_degs, rels, d_max);
    }
    else if (int error = CohFromJson(v_degs, rels, d_max, cw)) {
        fmt::print("Error({}) - Unsupported arugment cw={}\n", error, cw);
        return -1;
    }
    return 0;
}

int MapCohFromJson(const std::string& name, std::string& from, std::string& to, Mod1d& images, int& sus, int& fil)
{
    auto js = myio::load_json("Adams.json");
    try {
        auto& cws = js.at("CW_complexes");
        auto& maps = js.at("maps");
        if (!maps.contains(name))
            return -1;
        auto& map_json = maps.at(name);
        from = map_json.at("from").get<std::string>();
        to = map_json.at("to").get<std::string>();
        sus = map_json.at("sus").get<int>();
        images = {};
        if (map_json.contains("images")) {
            fil = 0;
            int1d cells_gen_to;
            for (auto& c : cws.at(to).at("cells_gen"))
                cells_gen_to.push_back(c.get<int>());
            int1d cells_gen_from;
            for (auto& c : cws.at(from).at("cells_gen"))
                cells_gen_from.push_back(c.get<int>());
            auto& images_json = map_json.at("images");
            if (images_json.size() != cells_gen_to.size())
                return -2;
            for (size_t i = 0; i < images_json.size(); ++i) {
                int1d im_array;
                for (auto& im_i : images_json[i])
                    im_array.push_back(im_i.get<int>());
                if (im_array.empty())
                    images.push_back({});
                else {
                    int cell = im_array.back();
                    if (sus != cell - cells_gen_to[i])
                        return -3;
                    int v = ut::IndexOf(cells_gen_from, im_array[0]);
                    if (v == -1)
                        return -4;
                    Mod image = MMod({}, v);
                    for (size_t j = 1; j < im_array.size(); ++j) {
                        int n = im_array[j] - im_array[j - 1];
                        if ((n & (n - 1)) != 0)
                            return -5;
                        image = MMilnor::Sq(n) * image;
                    }
                    images.push_back(std::move(image));
                }
            }
        }
        else if (map_json.contains("total")) {
            fil = 1;
            std::string total = map_json.at("total");
            const int t_max = 256;
            int1d v_degs_to;
            Mod1d rels_to;
            Mod1d cell_reduced_to;
            int1d min_rels_to;
            CohFromJsonV2(v_degs_to, rels_to, cell_reduced_to, min_rels_to, t_max, to);
            Groebner gb_to(t_max, {}, v_degs_to);
            gb_to.AddRels(rels_to, t_max, min_rels_to);

            int1d v_degs_from;
            Mod1d rels_from;
            Mod1d cells_reduced_from;
            int1d min_rels_from;
            CohFromJsonV2(v_degs_from, rels_from, cells_reduced_from, min_rels_from, t_max, from);

            int1d v_degs;
            Mod1d rels;
            Coh(v_degs, rels, t_max, total);
            Groebner gb(t_max, {}, v_degs);
            gb.AddRels(rels, t_max);

            int1d cells_from;
            for (auto& c : cws.at(from).at("cells"))
                cells_from.push_back(c.get<int>());

            for (int i : min_rels_to) {
                auto& rel = gb_to.data()[i];
                if (gb.Reduce(rel)) {  ////
                    int rel_deg = rel.GetLead().deg_m() + v_degs_to[rel.GetLead().v()];
                    int index = ut::IndexOf(cells_from, rel_deg + sus - 1); ////
                    if (index == -1)
                        return -6;
                    if (index >= (int)cells_reduced_from.size())
                        return -7;
                    images.push_back(cells_reduced_from[index]);
                }
                else
                    images.push_back({});
            }
        }
        else
            return -8;
    }
    catch (nlohmann::detail::exception& e) {
        fmt::print("Error({:#x}) - {}\n", e.id, e.what());
        throw e;
    }
    return 0;
}

void SetCohMap(const std::string& cw1, const std::string& cw2, std::string& from, std::string& to, Mod1d& images, int& sus, int& fil)
{
    from = cw1;
    to = cw2;
    sus = 0;
    fil = 0;
    if (cw1 == "S0") {
        if (cw2 == "tmf" || cw2 == "ko" || cw2 == "X2" || cw2 == "A_A3" || cw2 == "A_A4" || cw2 == "A_A5") {
            images = {MMod(MMilnor(), 0)};
            return;
        }
    }
    if (cw2 == "S0") {
        {
            const std::vector<std::string> Cs = {"C2", "Ceta", "Cnu", "Csigma", "C2h4", "C2h5", "C2h6"};
            int i = ut::IndexOf(Cs, cw1);
            if (i != -1) {
                images = {MMod(MMilnor::P(i, i + 1), 0)};
                sus = 1 << i;
                return;
            }
        }
        if (cw1 == "RP1_256") {
            images = {{}};
            for (int i = 1; i <= 8; ++i)
                images.push_back(MMod(MMilnor(), i - 1));
            fil = 1;
            return;
        }
        if (cw1 == "CP1_128") {
            images = {{}, {}};
            for (int i = 2; i <= 8; ++i)
                images.push_back(MMod(MMilnor(), i - 2));
            fil = 1;
            sus = -1;
            return;
        }
        if (cw1 == "HP1_64") {
            images = {{}, {}, {}};
            for (int i = 3; i <= 8; ++i)
                images.push_back(MMod(MMilnor(), i - 3));
            fil = 1;
            sus = -3;
            return;
        }
    }
    if (cw2 == "tmf") {
        const std::vector<std::string> Cs = {"tmf_C2", "tmf_Ceta", "tmf_Cnu"};
        int i = ut::IndexOf(Cs, cw1);
        if (i != -1) {
            images = {MMod(MMilnor::P(i, i + 1), 0)};
            sus = 1 << i;
            return;
        }
    }
    if ((cw1 == "C2" && cw2 == "tmf_C2") || (cw1 == "Ceta" && cw2 == "tmf_Ceta") || (cw1 == "Cnu" && cw2 == "tmf_Cnu")) {
        images = {MMod(MMilnor(), 0)};
        return;
    }
    if (cw1 == "CW_sigma_nu_eta_2" && cw2 == "tmf") {
        images = {MMod(MMilnor(), 0)};
        return;
    }
    if (cw1 == "RP1_256" && cw2 == "tmf_RP1_256") {
        images = {};
        int1d v_degs;
        int1d v_degs_n, tmp_ind1d;
        const int t_max = 256;
        Mod1d cell_reduced, tmp;
        Coh_P(v_degs, tmp, cell_reduced, tmp_ind1d, 1, t_max, t_max, "S0", 0);
        for (int i = 0; i < 2; ++i)
            images.push_back(MMod({}, i));
        for (int i = 2; 7 + 8 * (i - 2) <= t_max; ++i)
            images.push_back(cell_reduced[size_t(6 + 8 * (i - 2))]);
        return;
    }
    if (cw1 == "C2") {
        if (cw2 == "C2h4" || cw2 == "C2h5" || cw2 == "C2h6" || cw2 == "CW_2_eta" || cw2 == "C2_Ceta") {
            images = {MMod(MMilnor(), 0)};
            return;
        }
        if (cw2 == "j_C2") {
            images = {MMod(MMilnor(), 0), {}};
            return;
        }
        if (cw2 == "Q_DC2h4") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 4)
                    images.push_back(MMod(MMilnor::P(0, 1), 0));
                else
                    images.push_back({});
            }
            sus = -14;
            fil = 1;
            to = "S0";
            return;
        }
        if (cw2 == "Q_DC2h5") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 5)
                    images.push_back(MMod(MMilnor::P(0, 1), 0));
                else
                    images.push_back({});
            }
            sus = -30;
            fil = 1;
            to = "S0";
            return;
        }
        if (cw2 == "Q_DC2h6") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 6)
                    images.push_back(MMod(MMilnor::P(0, 1), 0));
                else
                    images.push_back({});
            }
            sus = -62;
            fil = 1;
            to = "S0";
            return;
        }
    }
    if (cw1 == "Ceta") {
        if (cw2 == "CW_eta_nu" || cw2 == "CW_eta_2" || cw2 == "C2_Ceta" || cw2 == "Ceta_Cnu" || cw2 == "Joker") {
            images = {MMod(MMilnor(), 0)};
            return;
        }
        if (cw2 == "Q_CW_2_eta") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 0)
                    images.push_back(MMod({}, 0));
                else
                    images.push_back({});
            }
            sus = 0;
            fil = 1;
            to = "S0";
            return;
        }
        if (cw2 == "Q_CW_nu_eta") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 2)
                    images.push_back(MMod({}, 0));
                else
                    images.push_back({});
            }
            sus = -3;
            fil = 1;
            to = "S0";
            return;
        }
    }
    if (cw1 == "Cnu") {
        if (cw2 == "CW_nu_sigma" || cw2 == "CW_nu_eta" || cw2 == "Ceta_Cnu" || cw2 == "Cnu_Csigma") {
            images = {MMod(MMilnor(), 0)};
            return;
        }
        if (cw2 == "Q_CW_eta_nu") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 1)
                    images.push_back(MMod({}, 0));
                else
                    images.push_back({});
            }
            sus = -1;
            fil = 1;
            to = "S0";
            return;
        }
        if (cw2 == "Q_CW_sigma_nu") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 3)
                    images.push_back(MMod({}, 0));
                else
                    images.push_back({});
            }
            sus = -7;
            fil = 1;
            to = "S0";
            return;
        }
    }
    if (cw1 == "Csigma") {
        if (cw2 == "CW_sigma_nu" || cw2 == "Cnu_Csigma") {
            images = {MMod(MMilnor(), 0)};
            return;
        }
        if (cw2 == "Q_CW_nu_sigma") {
            images = {};
            for (int i = 0; i <= 8; ++i) {
                if (i == 2)
                    images.push_back(MMod({}, 0));
                else
                    images.push_back({});
            }
            sus = -3;
            fil = 1;
            to = "S0";
            return;
        }
    }
    if (cw2 == "C2") {
        if (cw1 == "DC2h4" || cw1 == "DC2h5" || cw1 == "DC2h6") {
            images = {MMod(MMilnor(), 1)};
            sus = cw1 == "DC2h4" ? 15 : (cw1 == "DC2h5" ? 31 : 63);
            return;
        }
        if (cw1 == "CW_eta_2") {
            SetCohMapQ3cell_2cell(images, sus, 1, 0);
            return;
        }
        if (cw1 == "C2_Ceta") {
            SetCohMapQSmash_2cell(images, sus, 1, 0);
            return;
        }
    }
    if (cw2 == "Ceta") {
        if (cw1 == "CW_2_eta") {
            SetCohMapQ3cell_2cell(images, sus, 0, 1);
            return;
        }
        if (cw1 == "CW_nu_eta") {
            SetCohMapQ3cell_2cell(images, sus, 2, 1);
            return;
        }
        if (cw1 == "C2_Ceta") {
            SetCohMapQSmash_2cell(images, sus, 0, 1);
            return;
        }
        if (cw1 == "Ceta_Cnu") {
            SetCohMapQSmash_2cell(images, sus, 2, 1);
            return;
        }
    }
    if (cw2 == "Cnu") {
        if (cw1 == "CW_eta_nu") {
            SetCohMapQ3cell_2cell(images, sus, 1, 2);
            return;
        }
        if (cw1 == "CW_sigma_nu") {
            SetCohMapQ3cell_2cell(images, sus, 3, 2);
            return;
        }
        if (cw1 == "Ceta_Cnu") {
            SetCohMapQSmash_2cell(images, sus, 1, 2);
            return;
        }
        if (cw1 == "Cnu_Csigma") {
            SetCohMapQSmash_2cell(images, sus, 3, 2);
            return;
        }
    }
    if (cw2 == "Csigma") {
        if (cw1 == "CW_nu_sigma") {
            SetCohMapQ3cell_2cell(images, sus, 2, 3);
            return;
        }
        if (cw1 == "Cnu_Csigma") {
            SetCohMapQSmash_2cell(images, sus, 2, 3);
            return;
        }
    }
    if (cw1 == "j" && cw2 == "j_C2") {
        images = {MMod(MMilnor(), 0), MMod(MMilnor(), 1)};
        return;
    }
    if (cw1 == "j_C2" && cw2 == "j") {
        Mod i7Sq1 = MMilnor::Sq(8) * MMod(MMilnor(), 0) + MMilnor::Sq(1) * MMod(MMilnor(), 1);
        images = {MMod(MMilnor::P(0, 1), 0), i7Sq1};
        sus = 1;
        return;
    }

    /* (2, eta, nu) */
    if (cw1 == "CW_2_eta" && (cw2 == "Joker" || cw2 == "CW_2_eta_nu")) {
        images = {MMod(MMilnor(), 0)};
        return;
    }
    if (cw1 == "CW_nu_eta" && cw2 == "CW_nu_eta_2") {
        images = {MMod(MMilnor(), 0)};
        return;
    }
    if (cw1 == "CW_2_eta_nu" && cw2 == "CW_eta_nu") {
        images = {MMod(MMilnor::P(0, 1), 0)};
        sus = 1;
        return;
    }
    if (cw1 == "CW_nu_eta_2" && cw2 == "CW_eta_2") {
        images = {MMod(MMilnor::P(2, 3), 0)};
        sus = 4;
        return;
    }
    if (cw1 == "CW_eta_nu" && cw2 == "Q_CW_2_eta_nu") {
        images = {};
        for (int i = 0; i <= 8; ++i) {
            if (i == 0)
                images.push_back(MMod({}, 0));
            else
                images.push_back({});
        }
        sus = 0;
        fil = 1;
        to = "S0";
        return;
    }
    if (cw1 == "CW_eta_2" && cw2 == "Q_CW_nu_eta_2") {
        images = {};
        for (int i = 0; i <= 8; ++i) {
            if (i == 2)
                images.push_back(MMod({}, 0));
            else
                images.push_back({});
        }
        sus = -3;
        fil = 1;
        to = "S0";
        return;
    }

    if (cw1 == "CW_eta_2" && cw2 == "Fphi4") {
        images = {MMod(MMilnor(), 0)};
        return;
    }
    if (cw1 == "Joker") {
        if (cw2 == "CW_eta_2") {
            images = {MMod(MMilnor::P(0, 1), 0)};
            sus = 1;
            return;
        }
        if (cw2 == "Ceta") {
            images = {MMod(MMilnor::P(1, 2), 0)};
            sus = 2;
            return;
        }
    }
    if ((cw1 == "C2h4" && cw2 == "Csigmasq_0") || (cw1 == "C2h5" && cw2 == "Ctheta4_0") || (cw1 == "C2h6" && cw2 == "Ctheta5_0")) {
        to = "S0";
        images = {MMod(MMilnor::P(0, 1), 0)};
        sus = 1;
        return;
    }
    if (IsFP(cw1) && IsFP(cw2)) {
        SetCohMapFP2FP(cw1, cw2, from, to, images, sus, fil);
        return;
    }
    if (int n1 = IsFphi_n(cw1), n2 = IsFphi_n(cw2); 0 < n1 && n1 < n2) {
        images = {MMod(MMilnor(), 0)};
        return;
    }
    if (cw1 == "Fphi" && cw2 == "RP1_256") {
        images = {};
        for (int i = 1; i <= 8; ++i)
            images.push_back(MMod(MMilnor::P(i, i + 1), 0));
        sus = 1;
        return;
    }
    if (int n = IsFphi_n(cw1)) {
        if (cw2 == fmt::format("RP1_{}", n)) {
            images = {};
            for (int i = 1; (1 << i) - 1 <= n; ++i)
                images.push_back(MMod(MMilnor::P(i, i + 1), 0));
            sus = 1;
            return;
        }
    }
    std::string name = fmt::format("{}_to_{}", cw1, cw2);
    if (int error = MapCohFromJson(name, from, to, images, sus, fil)) {
        fmt::print("Error({}) - map not supported.\n", error);
        throw MyException(0x8636b4b2, "map not supported.");
    }
}
