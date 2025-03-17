#include <IpSolver.h>
#include <numeric>
#include "catch.hpp"

using namespace std;
using namespace ips;

TEST_CASE("ip case 0", "[ip]") {
    IpSolver solver;
    solver.init(6, 2);
    solver.SetVarBounds({1700L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1, 2, 8, 1, 2, 8});

    solver.SetConstraintCoefs(0, {1, 1, 1, -1, -1, -1});
    solver.SetConstraintCoefs(1, {153900, 154000, 154600, -153900, -154000, -154600});
    solver.SetConstraintBounds({8900L, 1370390000L}, {8900L, 1370390000L});
    std::string var_str = "x0, lb=1700, ub=5900, len=43\n"
        "x1, lb=3000, ub=7300, len=44\n"
        "x2, lb=0, ub=4300, len=44\n"
        "x3, lb=0, ub=0, len=1\n"
        "x4, lb=0, ub=0, len=1\n"
        "x5, lb=0, ub=0, len=1";
    std::string obj_str = "1 * x0 + 2 * x1 + 8 * x2 + 1 * x3 + 2 * x4 + 8 * x5";
    std::string cons_str = "1 * x0 + 1 * x1 + 1 * x2 + -1 * x3 + -1 * x4 + -1 * x5 = 8900\n"
        "153900 * x0 + 154000 * x1 + 154600 * x2 + -153900 * x3 + -154000 * x4 + -154600 * x5 = 1370390000";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(8900L, 1370390000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "2100,6800,0,0,0,0";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip case 1 hit avg price", "[ip]") {
    IpSolver solver;
    solver.init(6, 2);
    solver.SetVarBounds({0L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1, 2, 8, 1, 2, 8});

    solver.SetConstraintCoefs(0, {1, 1, 1, -1, -1, -1});
    solver.SetConstraintCoefs(1, {100, 200, 300, -100, -200, -300});
    solver.SetConstraintBounds({5000L, 1000000L}, {5000L, 1000000L});
    std::string cons_str = "1 * x0 + 1 * x1 + 1 * x2 + -1 * x3 + -1 * x4 + -1 * x5 = 5000\n"
        "100 * x0 + 200 * x1 + 300 * x2 + -100 * x3 + -200 * x4 + -300 * x5 = 1000000";
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(5000L, 1000000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "0,5000,0,0,0,0";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip case 1", "[ip]") {
    IpSolver solver;
    solver.init(10, 2);
    solver.SetVarBounds({1100L, 100L, 1700L, 0L, 0L, 0L, 0L, 0L, 0L, 0L}, {21400L, 20600L, 22200L, 17100L, 14200L, 0L, 0L, 0L, 3400L, 6300L});
    solver.SetObjectiveCoefs({1, 6, 8, 11, 19, 1, 6, 8, 11, 19});

    solver.SetConstraintCoefs(0, {1, 1, 1, 1, 1, -1, -1, -1, -1, -1});
    solver.SetConstraintCoefs(1, {276000, 276500, 276700, 277000, 277800, -276000, -276500, -276700, -277000, -277800});
    solver.SetConstraintBounds({13500L, 3736620000L}, {13500L, 3736620000L});
    std::string var_str = 
        "x0, lb=1100, ub=21400, len=204\n"
        "x1, lb=100, ub=20600, len=206\n"
        "x2, lb=1700, ub=22200, len=206\n"
        "x3, lb=0, ub=17100, len=172\n"
        "x4, lb=0, ub=14200, len=143\n"
        "x5, lb=0, ub=0, len=1\n"
        "x6, lb=0, ub=0, len=1\n"
        "x7, lb=0, ub=0, len=1\n"
        "x8, lb=0, ub=3400, len=35\n"
        "x9, lb=0, ub=6300, len=64";
    std::string obj_str = "1 * x0 + 6 * x1 + 8 * x2 + 11 * x3 + 19 * x4 + 1 * x5 + 6 * x6 + 8 * x7 + 11 * x8 + 19 * x9";
    std::string cons_str = "1 * x0 + 1 * x1 + 1 * x2 + 1 * x3 + 1 * x4 + -1 * x5 + -1 * x6 + -1 * x7 + -1 * x8 + -1 * x9 = 13500\n"
        "276000 * x0 + 276500 * x1 + 276700 * x2 + 277000 * x3 + 277800 * x4 + -276000 * x5 + -276500 * x6 + -276700 * x7 + -277000 * x8 + -277800 * x9 = 3736620000";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(13500L, 3736620000L, 0UL);
    std::vector<int64_t> res_sol(solver.Solution(), solver.Solution() + 10);
    std::vector<int64_t> expected_sol = {1100L, 100L, 6300L, 5800L, 200L, 0L, 0L, 0L, 0L, 0L};
    REQUIRE(res == OPTIMAL);
    REQUIRE(res_sol == expected_sol);
}

TEST_CASE("ip case 1 max count", "[ip]") {
    IpSolver solver;
    solver.init(10, 2);
    solver.SetVarBounds({1100L, 100L, 1700L, 0L, 0L, 0L, 0L, 0L, 0L, 0L}, {21400L, 20600L, 22200L, 17100L, 14200L, 0L, 0L, 0L, 3400L, 6300L});
    solver.SetObjectiveCoefs({1, 6, 8, 11, 19, 1, 6, 8, 11, 19});

    solver.SetConstraintCoefs(0, {1, 1, 1, 1, 1, -1, -1, -1, -1, -1});
    solver.SetConstraintCoefs(1, {276000, 276500, 276700, 277000, 277800, -276000, -276500, -276700, -277000, -277800});
    solver.SetConstraintBounds({13500L, 3736620000L}, {13500L, 3736620000L});
    auto res = solver.Solve(13500L, 3736620000L, 5UL);
    std::vector<int64_t> res_sol(solver.Solution(), solver.Solution() + 10);
    std::vector<int64_t> expected_sol = {1100L, 100L, 2000L, 11700L, 0L, 0L, 0L, 0L, 0L, 1400L};
    REQUIRE(res == FEASIBLE);
    REQUIRE(res_sol == expected_sol);
}

TEST_CASE("ip case 2", "[ip]") {
    IpSolver solver;
    solver.init(4, 2);
    solver.SetVarBounds({30300L, 0L, 0L, 0L}, {82800L, 31100L, 0L, 21900L});
    solver.SetObjectiveCoefs({1, 2, 1, 2});

    solver.SetConstraintCoefs(0, {1, 1, -1, -1});
    solver.SetConstraintCoefs(1, {72600, 72700, -72600, -72700});
    solver.SetConstraintBounds({60900L, 4421300000L}, {60900L, 4421300000L});
    std::string cons_str = "1 * x0 + 1 * x1 + -1 * x2 + -1 * x3 = 60900\n"
        "72600 * x0 + 72700 * x1 + -72600 * x2 + -72700 * x3 = 4421300000";
    REQUIRE(solver.to_string_or_constraint() == cons_str);
    auto res = solver.Solve(60900L, 4421300000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "61300,0,0,400";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip case 3", "[ip]") {
    IpSolver solver;
    solver.init(4, 2);
    solver.SetVarBounds({65500L, 0L, 0L, 0L}, {118300L, 36000L, 0L, 17300L});
    solver.SetObjectiveCoefs({1, 2, 1, 2});

    solver.SetConstraintCoefs(0, {1, 1, -1, -1});
    solver.SetConstraintCoefs(1, {112400, 112500, -112400, -112500});
    solver.SetConstraintBounds({101000L, 11350960000L}, {101000L, 11350960000L});
    auto res = solver.Solve(101000L, 11350960000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "115400,0,0,14400";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}

TEST_CASE("ip case 4", "[ip]") {
    IpSolver solver;
    solver.init(6, 2);
    solver.SetVarBounds({1600L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1, 2, 8, 1, 2, 8});

    solver.SetConstraintCoefs(0, {1, 1, 1, -1, -1, -1});
    solver.SetConstraintCoefs(1, {153900, 154000, 154600, -153900, -154000, -154600});
    solver.SetConstraintBounds({8900L, 1370390000L}, {8900L, 1370390000L});
    auto res = solver.Solve(8900L, 1370390000L, 0UL);
    REQUIRE(res == OPTIMAL);
    std::string sol_str = "2100,6800,0,0,0,0";
    REQUIRE(solver.to_string_or_solution() == sol_str);
}