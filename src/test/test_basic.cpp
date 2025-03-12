#include <IpSolver.h>
#include <numeric>
#include "catch.hpp"

using namespace std;
using namespace ips;

TEST_CASE("ip case 0", "[ip]") {
    IpSolver solver;
    solver.init(6, 2);
    solver.SetVarBounds({1700L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1L, 2L, 8L, 1L, 2L, 8L});

    solver.SetConstraintCoefs(0, {1L, 1L, 1L, -1L, -1L, -1L});
    solver.SetConstraintCoefs(1, {153900L, 154000L, 154600L, -153900L, -154000L, -154600L});
    solver.SetConstraintBounds({8900L, 1370390000L}, {8900L, 1370390000L});
    std::string var_str = "x0, lb=1700, ub=5900, neg=0, len=0\n"
        "x1, lb=3000, ub=7300, neg=0, len=0\n"
        "x2, lb=0, ub=4300, neg=0, len=0\n"
        "x3, lb=0, ub=0, neg=0, len=0\n"
        "x4, lb=0, ub=0, neg=0, len=0\n"
        "x5, lb=0, ub=0, neg=0, len=0";
    std::string obj_str = "1 * x0 + 2 * x1 + 8 * x2 + 1 * x3 + 2 * x4 + 8 * x5";
    std::string cons_str = "8900 <= 1 * x0 + 1 * x1 + 1 * x2 + -1 * x3 + -1 * x4 + -1 * x5 <= 8900\n"
        "1370390000 <= 153900 * x0 + 154000 * x1 + 154600 * x2 + -153900 * x3 + -154000 * x4 + -154600 * x5 <= 1370390000";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve();
    std::vector<int64_t> res_sol(solver.Solution(), solver.Solution() + 6);
    std::vector<int64_t> expected_sol = {2100L, 6800L, 0L, 0L, 0L, 0L};
    REQUIRE(res == OPTIMAL);
    REQUIRE(res_sol == expected_sol);
}

TEST_CASE("ip case 1", "[ip]") {
    IpSolver solver;
    solver.init(10, 2);
    solver.SetVarBounds({1100L, 100L, 1700L, 0L, 0L, 0L, 0L, 0L, 0L, 0L}, {21400L, 20600L, 22200L, 17100L, 14200L, 0L, 0L, 0L, 3400L, 6300L});
    solver.SetObjectiveCoefs({1L, 6L, 8L, 11L, 19L, 1L, 6L, 8L, 11L, 19L});

    solver.SetConstraintCoefs(0, {1L, 1L, 1L, 1L, 1L, -1L, -1L, -1L, -1L, -1L});
    solver.SetConstraintCoefs(1, {276000L, 276500L, 276700L, 277000L, 277800L, -276000L, -276500L, -276700L, -277000L, -277800L});
    solver.SetConstraintBounds({13500L, 3736620000L}, {13500L, 3736620000L});
    std::string var_str = 
        "x0, lb=1100, ub=21400, neg=0, len=0\n"
        "x1, lb=100, ub=20600, neg=0, len=0\n"
        "x2, lb=1700, ub=22200, neg=0, len=0\n"
        "x3, lb=0, ub=17100, neg=0, len=0\n"
        "x4, lb=0, ub=14200, neg=0, len=0\n"
        "x5, lb=0, ub=0, neg=0, len=0\n"
        "x6, lb=0, ub=0, neg=0, len=0\n"
        "x7, lb=0, ub=0, neg=0, len=0\n"
        "x8, lb=0, ub=3400, neg=0, len=0\n"
        "x9, lb=0, ub=6300, neg=0, len=0";
    std::string obj_str = "1 * x0 + 6 * x1 + 8 * x2 + 11 * x3 + 19 * x4 + 1 * x5 + 6 * x6 + 8 * x7 + 11 * x8 + 19 * x9";
    std::string cons_str = "13500 <= 1 * x0 + 1 * x1 + 1 * x2 + 1 * x3 + 1 * x4 + -1 * x5 + -1 * x6 + -1 * x7 + -1 * x8 + -1 * x9 <= 13500\n"
        "3736620000 <= 276000 * x0 + 276500 * x1 + 276700 * x2 + 277000 * x3 + 277800 * x4 + -276000 * x5 + -276500 * x6 + -276700 * x7 + -277000 * x8 + -277800 * x9 <= 3736620000";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve();
    std::vector<int64_t> res_sol(solver.Solution(), solver.Solution() + 10);
    std::vector<int64_t> expected_sol = {2100L, 6800L, 0L, 0L, 0L, 0L};
    REQUIRE(res == OPTIMAL);
    REQUIRE(res_sol == expected_sol);
}