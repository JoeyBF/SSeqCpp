#include "algebras/benchmark.h"
#include "algebras/dbalg.h"
#include "algebras/utility.h"
#include "groebner_steenrod.h"

std::vector<int> bench::Counter::counts_ = {0, 0, 0, 0, 0};

void AdamsE2()
{
    using namespace steenrod;

    bench::Timer timer;

    int t_trunc = 70;
    Mod1d rels;
    for (int i = 0; i < 10; ++i) {
        rels.push_back(MMod(MMilnor::P(i, i + 1), 0));
        if (MMilnor::P(i, i + 1).deg() > t_trunc)
            break;
    }
    auto gb = GroebnerMRes::load("AdamsE2.db", t_trunc);
    size_t dim = AddRelsMRes(gb, rels, t_trunc);
    std::cout << "dim=" << dim << '\n';



    /*auto& data = gb.data();
    for (size_t s = 0; s < data.size(); ++s) {
        std::cout << "s=" << s << '\n';
        for (size_t i = 0; i < data[s].size(); ++i)
            std::cout << data[s][i].x1 << " = " << data[s][i].x2 << '\n';
    }*/
}

int main()
{
    // steenrod::generate_basis("AdamsE2.db", 250);
    AdamsE2();
    bench::Counter::print();
    return 0;
}