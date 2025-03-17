#include <IpSolverMem.h>
#include <numeric>
#include "catch.hpp"

using namespace std;
using namespace ips;

TEST_CASE("ip_mem case 0", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(3, 2);
    solver.SetVarBoundsWithNeg({1700L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1, 2, 8});
    solver.SetConstraintCoefs(1, {153900, 154000, 154600});
    std::string var_str = "x0, lb=1700, ub=5900, len=43\n"
        "x1, lb=3000, ub=7300, len=44\n"
        "x2, lb=0, ub=4300, len=44";
    std::string obj_str = "1 * x0 + 2 * x1 + 8 * x2";
    std::string cons_str = "153900 * x0 + 154000 * x1 + 154600 * x2";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(8900L, 1370390000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "2100,6800,0";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip_mem case 1 hit avg price", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(3, 2);
    solver.SetVarBoundsWithNeg({0L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1, 2, 8});
    solver.SetConstraintCoefs(1, {100, 200, 300});
    std::string cons_str = "100 * x0 + 200 * x1 + 300 * x2";
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(5000L, 1000000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "0,5000,0";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip_mem case 1", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(5, 2);
    solver.SetVarBoundsWithNeg({1100L, 100L, 1700L, 0L, 0L, 0L, 0L, 0L, 0L, 0L}, {21400L, 20600L, 22200L, 17100L, 14200L, 0L, 0L, 0L, 3400L, 6300L});
    solver.SetObjectiveCoefs({1, 6, 8, 11, 19});
    solver.SetConstraintCoefs(1, {276000, 276500, 276700, 277000, 277800});
    std::string var_str = 
        "x0, lb=1100, ub=21400, len=204\n"
        "x1, lb=100, ub=20600, len=206\n"
        "x2, lb=1700, ub=22200, len=206\n"
        "x3, lb=-3400, ub=17100, len=206\n"
        "x4, lb=-6300, ub=14200, len=206";
    std::string obj_str = "1 * x0 + 6 * x1 + 8 * x2 + 11 * x3 + 19 * x4";
    std::string cons_str = "276000 * x0 + 276500 * x1 + 276700 * x2 + 277000 * x3 + 277800 * x4";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(13500L, 3736620000L, 0UL);
    std::vector<int64_t> res_sol(solver.Solution(), solver.Solution() + 5);
    std::vector<int64_t> expected_sol = {1100L, 100L, 6300L, 5800L, 200L};
    REQUIRE(res == OPTIMAL);
    REQUIRE(res_sol == expected_sol);
}

TEST_CASE("ip_mem case 1 max count", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(5, 2);
    solver.SetVarBoundsWithNeg({1100L, 100L, 1700L, 0L, 0L, 0L, 0L, 0L, 0L, 0L}, {21400L, 20600L, 22200L, 17100L, 14200L, 0L, 0L, 0L, 3400L, 6300L});
    solver.SetObjectiveCoefs({1, 6, 8, 11, 19});
    solver.SetConstraintCoefs(1, {276000, 276500, 276700, 277000, 277800});
    auto res = solver.Solve(13500L, 3736620000L, 5UL);
    std::vector<int64_t> res_sol(solver.Solution(), solver.Solution() + 5);
    std::vector<int64_t> expected_sol = {1100L, 100L, 2000L, 11700L, -1400L};
    REQUIRE(res == FEASIBLE);
    REQUIRE(res_sol == expected_sol);
}

TEST_CASE("ip_mem case 2", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(2, 2);
    solver.SetVarBoundsWithNeg({30300L, 0L, 0L, 0L}, {82800L, 31100L, 0L, 21900L});
    solver.SetObjectiveCoefs({1, 2});
    solver.SetConstraintCoefs(1, {72600, 72700});
    std::string cons_str = "72600 * x0 + 72700 * x1";
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(60900L, 4421300000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "61300,-400";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip_mem case 3", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(2, 2);
    solver.SetVarBoundsWithNeg({65500L, 0L, 0L, 0L}, {118300L, 36000L, 0L, 17300L});
    solver.SetObjectiveCoefs({1, 2});
    solver.SetConstraintCoefs(1, {112400, 112500});
    auto res = solver.Solve(101000L, 11350960000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "115400,-14400";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip_mem case 4", "[ip_mem]") {
    IpSolverMem solver;
    solver.init(3, 2);
    solver.SetVarBoundsWithNeg({1600L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1, 2, 8});
    solver.SetConstraintCoefs(1, {153900, 154000, 154600});
    auto res = solver.Solve(8900L, 1370390000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "2100,6800,0";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}