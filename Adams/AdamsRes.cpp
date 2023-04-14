#include "algebras/benchmark.h"
#include "algebras/myio.h"
#include "algebras/utility.h"
#include "groebner_res.h"
#include "main.h"

#include <cstring>
#include <sstream>

void ResolveV2(const Mod1d& rels, const int1d& v_degs, int t_trunc, int stem_trunc, const std::string& db_filename, const std::string& tablename)
{
    using namespace steenrod;

    auto gb = AdamsRes::load(db_filename, tablename, t_trunc, stem_trunc);
    Resolve(gb, rels, v_degs, t_trunc, stem_trunc, db_filename, tablename);

    std::cout << "gb.dim_Ext()=" << gb.dim_Ext() << '\n';
    std::cout << "gb.dim_Gb()=" << gb.dim_Gb() << '\n';
}

Mod1d GetS0Rels(int t_trunc)
{
    using namespace steenrod;
    Mod1d rels;
    for (int i = 0; (1 << i) <= t_trunc; ++i)
        rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
    return rels;
}

steenrod::Mod XTo(unsigned m)
{
    using namespace steenrod;

    unsigned k = 1;
    while ((1U << k) <= m + 1)
        ++k;
    --k;
    m -= (1U << k) - 1;
    Mod result = MMod(MMilnor(), k - 1);
    result = MMilnor::Sq(m) * result;
    return result;
}

Mod1d GetRPRels(int n, int t_trunc)
{
    using namespace steenrod;

    Mod1d rels;
    Mod tmp;
    /* Sq^{2^i} * x^m = (2^i, m - 2^i) x^{m + 2^i} */
    for (int i = 0; (1 << i) <= t_trunc; ++i) {
        int k = 1 << i;
        for (int m = 1; m + k <= t_trunc && m <= n; ++m) {
            Mod xm = XTo((unsigned)m);
            Mod rel = MMilnor::Sq(k) * xm;
            if (m + k <= n && k <= m && !(k & (m - k)))
                rel.iaddP(XTo(unsigned(m + k)), tmp);
            rels.push_back(std::move(rel));
        }
    }
    return rels;
}

int1d GetRP_v_degs(int n)
{
    int1d result;
    for (int i = 1; (1 << i) - 1 <= n; ++i)
        result.push_back((1 << i) - 1);
    return result;
}

int main_res_S0(int argc, char** argv, int index)
{
    int t_max = 100, stem_max = 261;
    std::string db_filename = "S0_Adams_res.db";
    std::string tablename = "S0_Adams_res";

    if (argc > index + 1 && strcmp(argv[size_t(index + 1)], "-h") == 0) {
        std::cout << "Calculate the minimal resolution of S0 for the Adams spectral sequence\n";
        std::cout << "Usage:\n  Adams res S0 [t_max] [stem_max] [db_filename] [tablename]\n\n";

        std::cout << "Default values:\n";
        std::cout << "  t_max = " << t_max << "\n";
        std::cout << "  stem_max = " << stem_max << "\n";
        std::cout << "  db_filename = " << db_filename << "\n";
        std::cout << "  tablename = " << tablename << "\n\n";

        std::cout << VERSION << std::endl;
        return 0;
    }
    if (myio::load_op_arg(argc, argv, ++index, "t_max", t_max))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "stem_max", stem_max))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "db_filename", db_filename))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "tablename", tablename))
        return index;

    ResolveV2(GetS0Rels(t_max), {0}, t_max, stem_max, db_filename, tablename);

#ifndef MYDEPLOY
    bench::Counter::print();
#endif
    return 0;
}

int main_res_RP(int argc, char** argv, int index)
{
    int t_max = 100, stem_max = 261;
    int n = -1;
    std::string db_filename = "RPn_Adams_res.db";
    std::string tablename = "RPn_Adams_res";

    if (argc > index + 1 && strcmp(argv[size_t(index + 1)], "-h") == 0) {
        std::cout << "Calculate the minimal resolution of S0 for the Adams spectral sequence\n";
        std::cout << "Usage:\n  Adams res RP [n] [t_max] [stem_max] [db_filename] [tablename]\n\n";

        std::cout << "Default values:\n";
        std::cout << "  n = inf\n";
        std::cout << "  t_max = " << t_max << "\n";
        std::cout << "  stem_max = " << stem_max << "\n";
        std::cout << "  db_filename = " << db_filename << "\n";
        std::cout << "  tablename = " << tablename << "\n\n";

        std::cout << VERSION << std::endl;
        return 0;
    }
    if (argc > index + 1 && strcmp(argv[size_t(index + 1)], "inf") == 0) {
        n = -1;
        ++index;
    }
    else if (myio::load_op_arg(argc, argv, ++index, "n", n))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "t_max", t_max))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "stem_max", stem_max))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "db_filename", db_filename))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "tablename", tablename))
        return index;
    std::string strN = n == -1 ? "inf" : std::to_string(n);
    if (n == -1)
        n = t_max;
    if (db_filename == "RPn_Adams_res.db")
        db_filename = "RP" + strN + "_Adams_res.db";
    if (tablename == "RPn_Adams_res")
        tablename = "RP" + strN + "_Adams_res";

    ResolveV2(GetRPRels(n, t_max), GetRP_v_degs(n), t_max, stem_max, db_filename, tablename);

#ifndef MYDEPLOY
    bench::Counter::print();
#endif
    return 0;
}

int main_res_X2(int argc, char** argv, int index)
{
    int t_max = 100, stem_max = 261;
    std::string db_filename = "X2_Adams_res.db";
    std::string tablename = "X2_Adams_res";

    if (argc > index + 1 && strcmp(argv[size_t(index + 1)], "-h") == 0) {
        std::cout << "Calculate the minimal resolution of S0 for the Adams spectral sequence\n";
        std::cout << "Usage:\n  Adams res X2 [t_max] [stem_max] [db_filename] [tablename]\n\n";

        std::cout << "Default values:\n";
        std::cout << "  t_max = " << t_max << "\n";
        std::cout << "  stem_max = " << stem_max << "\n";
        std::cout << "  db_filename = " << db_filename << "\n";
        std::cout << "  tablename = " << tablename << "\n\n";

        std::cout << VERSION << std::endl;
        return 0;
    }
    if (myio::load_op_arg(argc, argv, ++index, "t_max", t_max))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "stem_max", stem_max))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "db_filename", db_filename))
        return index;
    if (myio::load_op_arg(argc, argv, ++index, "tablename", tablename))
        return index;

    using namespace steenrod;
    int1d v_degs;
    std::vector<uint64_t> bcs;
    std::unordered_map<uint64_t, uint64_t> bc2v;
    for (int t = 0; t <= t_max; ++t) {
        for (int c = 0; c <= t / 3; ++c) {
            int b = t - 3 * c;
            v_degs.push_back(t);
            auto bc = ut::Bind((uint64_t)b, (uint64_t)c);
            bcs.push_back(bc);
            bc2v[bc] = bcs.size() - 1;
        }
    }
    Mod1d rels;
    Mod tmp;

    for (size_t i = 0; (1 << i) <= t_max; ++i) {
        size_t a = (size_t)1 << i;
        for (size_t j = 0; j < bcs.size(); ++j) {
            int d = v_degs[j];
            if (a + d > t_max)
                continue;
            uint64_t b, c;
            ut::UnBind(bcs[j], b, c);
            Mod rel = MMilnor::Sq((uint32_t)a) * MMod(MMilnor(), j);
            for (size_t n = 0; n <= (a + d) / 3; ++n) {
                size_t m = a + d - 3 * n;
                if (a + 2 * c >= 2 * n && b + c >= n && !((a + 2 * c - 2 * n) & (b + c - n)) && c <= n && !(c & (n - c))) {
                    auto mn = ut::Bind((uint64_t)m, (uint64_t)n);
                    auto v_mn = bc2v.at(mn);
                    rel.iaddP(MMod(MMilnor(), v_mn), tmp);
                }
                // std::cout << "rel=" << rel << '\n';
            }
            rels.push_back(std::move(rel));
        }
    }

    Groebner gb(t_max, {}, v_degs);
    gb.AddRels(rels, t_max);
    gb.MinimizeOrderedGens();

    ResolveV2(gb.data(), gb.v_degs(), t_max, stem_max, db_filename, tablename);

#ifndef MYDEPLOY
    bench::Counter::print();
#endif
    return 0;
}

int main_res(int argc, char** argv, int index)
{
    std::string cmd = "S0";

    if (argc > index + 1 && strcmp(argv[size_t(index + 1)], "-h") == 0) {
        std::cout << "Calculate the minimal resolution for the Adams spectral sequence\n";
        std::cout << "Usage:\n  Adams res <cmd> [...]\n\n";

        std::cout << "<cmd> can be one of the following:\n";
        std::cout << "  S0: The sphere\n";
        std::cout << "  RP: Real projective spaces\n\n";
        std::cout << "  X2: connected cover X<2>\n\n";

        std::cout << VERSION << std::endl;
        return 0;
    }
    if (myio::load_arg(argc, argv, ++index, "complex", cmd))
        return index;

    if (cmd == "S0")
        return main_res_S0(argc, argv, index);
    else if (cmd == "RP")
        return main_res_RP(argc, argv, index);
    else if (cmd == "X2")
        return main_res_X2(argc, argv, index);
    else {
        std::cerr << "Invalid cmd: " << cmd << '\n';
        return 0;
    }

#ifndef MYDEPLOY
    bench::Counter::print();
#endif
    return 0;
}