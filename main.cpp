// steiner-multicycle-problem-3-apx.cpp : Defines the entry point for the application.
//

#include <iostream>

#include "gurobi_c++.h"
#include "src/relaxed_solver_surviv_net.h"

#include <vector>

#include <lemon/list_graph.h>

int test()
{
    std::vector<int> source2sink = { 2, 3 };
    std::vector<Vertex> vertices = {
        Vertex(0.0f, 0.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
        Vertex(0.0f, 1.0f),
    };

    GRBEnv env = new GRBEnv(true);
    env.set("LogFile", "mip1.log");
    env.start();

    // Create an empty model
    GRBModel model = GRBModel(env);

    RelaxedSurvivableNetworkSolver solver(model, 4, source2sink, vertices);

    solver.solve(model);

    return 0;
}

int main(int argc, char* argv[]) {
    test();
    return 0;
}