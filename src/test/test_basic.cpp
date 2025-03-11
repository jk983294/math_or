#include <IpSolver.h>
#include <numeric>
#include "catch.hpp"

using namespace std;
using namespace ips;

TEST_CASE("ip case0", "[ip]") {
    IpSolver solver;
    solver.init(6, 4);
    solver.SetVarBounds({1600L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    solver.SetObjectiveCoefs({1L, 1L, 1L, 1L, 1L, 1L});

    solver.SetConstraintCoefs(0, {1L, 1L, 1L, -1L, -1L, -1L});
    solver.SetConstraintCoefs(1, {153900L, 154000L, 154600L, -153900L, -154000L, -154600L});
    solver.SetConstraintCoef(2, 0, 1);
    solver.SetConstraintCoef(3, 3, -1);
    solver.SetConstraintBounds({8900L, 1370390000L, 1700L, 0L}, {8900L, 1370390000L, 5900L, 0L});
    std::string var_str = "x0, lb=1600, ub=5900, neg=0, len=0\n"
        "x1, lb=3000, ub=7300, neg=0, len=0\n"
        "x2, lb=0, ub=4300, neg=0, len=0\n"
        "x3, lb=0, ub=0, neg=0, len=0\n"
        "x4, lb=0, ub=0, neg=0, len=0\n"
        "x5, lb=0, ub=0, neg=0, len=0";
    std::string obj_str = "1 * x0 + 1 * x1 + 1 * x2 + 1 * x3 + 1 * x4 + 1 * x5";
    std::string cons_str = "8900 <= 1 * x0 + 1 * x1 + 1 * x2 + -1 * x3 + -1 * x4 + -1 * x5 <= 8900\n"
        "1370390000 <= 153900 * x0 + 154000 * x1 + 154600 * x2 + -153900 * x3 + -154000 * x4 + -154600 * x5 <= 1370390000\n"
        "1700 <= 1 * x0 + 0 * x1 + 0 * x2 + 0 * x3 + 0 * x4 + 0 * x5 <= 5900\n"
        "0 <= 0 * x0 + 0 * x1 + 0 * x2 + -1 * x3 + 0 * x4 + 0 * x5 <= 0";
    REQUIRE(solver.to_string_or_variables() == var_str);
    REQUIRE(solver.to_string_or_objective() == obj_str);
    REQUIRE(solver.to_string_or_constraint() == cons_str);
}