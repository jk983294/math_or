#include <IpSolver.h>
#include <chrono>
#include <iostream>

using namespace ips;

int main() {
    IpSolver solver;
    solver.init(6, 2);
    // solver.SetVarBounds({1700L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    // solver.SetObjectiveCoefs({1L, 2L, 8L, 1L, 2L, 8L});
    // solver.SetConstraintCoefs(0, {1L, 1L, 1L, -1L, -1L, -1L});
    // solver.SetConstraintCoefs(1, {153900L, 154000L, 154600L, -153900L, -154000L, -154600L});
    // solver.SetConstraintBounds({8900L, 1370390000L}, {8900L, 1370390000L});
    // int64_t _vol_target = 8900L;
    // int64_t _amt_target = 1370390000L;

    solver.init(10, 2);
    solver.SetVarBounds({1100L, 100L, 1700L, 0L, 0L, 0L, 0L, 0L, 0L, 0L},
                        {21400L, 20600L, 22200L, 17100L, 14200L, 0L, 0L, 0L, 3400L, 6300L});
    solver.SetObjectiveCoefs({1L, 6L, 8L, 11L, 19L, 1L, 6L, 8L, 11L, 19L});
    solver.SetConstraintCoefs(0, {1L, 1L, 1L, 1L, 1L, -1L, -1L, -1L, -1L, -1L});
    solver.SetConstraintCoefs(1, {276000L, 276500L, 276700L, 277000L, 277800L, -276000L, -276500L, -276700L, -277000L, -277800L});
    solver.SetConstraintBounds({13500L, 3736620000L}, {13500L, 3736620000L});
    int64_t _vol_target = 13500L;
    int64_t _amt_target = 3736620000L;

    // solver.init(4, 2);
    // solver.SetVarBounds({30300L, 0L, 0L, 0L}, {82800L, 31100L, 0L, 21900L});
    // solver.SetObjectiveCoefs({1L, 2L, 1L, 2L});
    // solver.SetConstraintCoefs(0, {1L, 1L, -1L, -1L});
    // solver.SetConstraintCoefs(1, {72600L, 72700L, -72600L, -72700L});
    // solver.SetConstraintBounds({60900L, 4421300000L}, {60900L, 4421300000L});
    // int64_t _vol_target = 60900L;
    // int64_t _amt_target = 4421300000L;

    // solver.init(6, 2);
    // solver.SetVarBounds({1600L, 3000L, 0L, 0L, 0L, 0L}, {5900L, 7300L, 4300L, 0L, 0L, 0L});
    // solver.SetObjectiveCoefs({1L, 2L, 8L, 1L, 2L, 8L});
    // solver.SetConstraintCoefs(0, {1L, 1L, 1L, -1L, -1L, -1L});
    // solver.SetConstraintCoefs(1, {153900L, 154000L, 154600L, -153900L, -154000L, -154600L});
    // solver.SetConstraintBounds({8900L, 1370390000L}, {8900L, 1370390000L});
    // int64_t _vol_target = 8900L;
    // int64_t _amt_target = 1370390000L;

    std::cout << "vars:\n" << solver.to_string_or_variables() << std::endl;
    std::cout << "obj: " << solver.to_string_or_objective() << std::endl;
    std::cout << "cons:\n" << solver.to_string_or_constraint() << std::endl;

    auto t0 = std::chrono::system_clock::now();
    ResultStatus result_status = solver.Solve(_vol_target, _amt_target);
    auto t1 = std::chrono::system_clock::now();

    std::cout << "vars:\n" << solver.to_string_or_variables() << std::endl;
    std::cout << "obj: " << solver.to_string_or_objective() << std::endl;
    std::cout << "cons:\n" << solver.to_string_or_constraint() << std::endl;

    if (result_status == INFEASIBLE || result_status == NOT_SOLVED) {
        printf("infeasible\n");
    } else {
        printf("Objective value = %ld\n", solver.ObjValue());
        printf("x = %s\n", solver.to_string_or_solution().c_str());
        // printf("Problem solved in %ld ms\n", solver->wall_time());
        printf("Problem solved in %ld iterations\n", solver.iterations());
        // printf("Problem solved in %ld branch-and-bound nodes\n", solver->nodes());
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        std::cout << "time: " << duration.count() << " us\n";
    }
    return 0;
}