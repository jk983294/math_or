#include <IpSolver.h>
#include <chrono>
#include <iostream>

using namespace ips;


int main() {
    IpSolver solver;
    solver.MakeIntVar(6);
    solver.SetVarBound(0, 1600, 5900);
    solver.SetVarBound(1, 3000, 7300);
    solver.SetVarBound(2, 0, 4300);
    solver.SetVarBound(3, 0, 0);
    solver.SetVarBound(4, 0, 0);
    solver.SetVarBound(5, 0, 0);
    solver.SetObjectiveCoef(0, 1);
    solver.SetObjectiveCoef(1, 1);
    solver.SetObjectiveCoef(2, 1);
    solver.SetObjectiveCoef(3, 1);
    solver.SetObjectiveCoef(4, 1);
    solver.SetObjectiveCoef(5, 1);
    solver.MakeRowConstraint(4);
    solver.SetConstraintCoefs(0, {1L, 1L, 1L, -1L, -1L, -1L});
    solver.SetConstraintCoefs(1, {153900L, 154000L, 154600L, -153900L, -154000L, -154600L});
    solver.SetConstraintCoef(2, 0, 1);
    solver.SetConstraintCoef(3, 3, -1);
    solver.SetConstraintBound(0, 8900, 8900);
    solver.SetConstraintBound(1, 1370390000L, 1370390000L);
    solver.SetConstraintBound(2, 1700, 5900);
    solver.SetConstraintBound(3, 0, 0);

    std::cout << "vars:\n" << solver.to_string_or_variables() << std::endl;
    std::cout << "obj: " << solver.to_string_or_objective() << std::endl;
    std::cout << "cons:\n" << solver.to_string_or_constraint() << std::endl;

    auto t0 = std::chrono::system_clock::now();
    ResultStatus result_status = solver.Solve();
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
        // printf("Problem solved in %ld iterations\n", solver->iterations());
        // printf("Problem solved in %ld branch-and-bound nodes\n", solver->nodes());
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        std::cout << "time: " << duration.count() << " us\n";
    }
    return 0;
}