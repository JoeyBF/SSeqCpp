#include "main.h"
#include "algebras/benchmark.h"
#include "algebras/myio.h"
#include "algebras/utility.h"
#include <iostream>

#ifndef MYDEPLOY
std::vector<int> bench::Counter::counts_ = {0, 0, 0};
#endif

int main_generators_to_csv(const std::string& db_filename, const std::string& tablename);

int main(int argc, char** argv)
{
    std::string cmd;

    int index = 0;
    if (argc > index + 1 && strcmp(argv[size_t(index + 1)], "-h") == 0) {
        std::cout << "Usage:\n  Adams <cmd> [-h] ...\n\n";

        std::cout << "<cmd> can be one of the following:\n";

        std::cout << "  res: Compute an A-resolution\n";
        std::cout << "  2cell: Generate the basis of Ext for a cell complex\n";
        std::cout << "  prod: Compute the multiplications\n";
        std::cout << "  export: Extract info of Ext from the resolution data\n";
        std::cout << "  plot: Generate an html file (feature not supported yet)\n";

        std::cout << VERSION << std::endl;
        return 0;
    }
    if (myio::load_arg(argc, argv, ++index, "cmd", cmd))
        return index;

    bench::Timer timer;

    if (cmd == "res")
        return main_res(argc, argv, index);
    else if (cmd == "2cell")
        return main_2cell(argc, argv, index);
    else if (cmd == "prod_hi")
        return main_prod_hi(argc, argv, index);
    else if (cmd == "prod")
        return main_prod(argc, argv, index);
    else if (cmd == "export")
        return main_export(argc, argv, index);
    else if (cmd == "res_csv")
        return main_generators_to_csv("E:/S0_Adams_res_d261.db", "S0_Adams_res");
    else
        std::cerr << "Invalid cmd: " << cmd << '\n';
}