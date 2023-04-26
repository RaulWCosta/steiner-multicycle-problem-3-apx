// steiner-multicycle-problem-3-apx.cpp : Defines the entry point for the application.
//

#include <iostream>

#include "gurobi_c++.h"
#include "src/relaxed_solver_surviv_net.h"

#include <vector>
#include <utility>
#include <tuple>

#include <lemon/lmath.h>
#include <lemon/core.h>
#include <lemon/graph_to_eps.h>
#include <lemon/full_graph.h>
#include <lemon/concepts/graph_components.h>

// int test()
// {
//     std::vector<int> source2sink = { 2, 3 };
//     std::vector<Vertex> vertices = {
//         Vertex(0.0f, 0.0f),
//         Vertex(1.0f, 0.0f),
//         Vertex(1.0f, 1.0f),
//         Vertex(0.0f, 1.0f),
//     };

//     GRBEnv env = new GRBEnv(true);
//     env.set("LogFile", "mip1.log");
//     env.start();

//     // Create an empty model
//     GRBModel model = GRBModel(env);

//     RelaxedSurvivableNetworkSolver solver(model, 4, source2sink, vertices);

//     solver.solve(model);

//     return 0;
// }

float vertices_distance(const std::pair<float, float>& a, const std::pair<float, float>& b) {
    float dx = a.first - b.first;
    float dy = a.second - b.second;

    return sqrt(dx * dx + dy * dy);
}

std::tuple<lemon::FullGraph*, lemon::FullGraph::EdgeMap<float>*> create_complete_graph(int n, std::vector<std::pair<float, float>>& vertices) {
    lemon::FullGraph* g = new lemon::FullGraph(n);

    lemon::FullGraph::EdgeMap<float>* cost = new lemon::FullGraph::EdgeMap<float>(*g);


    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            lemon::FullGraphBase::Node u = (*g)(i);
            lemon::FullGraphBase::Node v = (*g)(j);
            float dist = vertices_distance(vertices[i], vertices[j]);
            (*cost)[g->edge(u, v)] = dist;
        }
    }

    return {g, cost};
}

int main(int argc, char* argv[]) {
    // test();
    std::vector<std::pair<float, float>> vertices = {
        std::pair<float, float>(0.0, 0.0),
        std::pair<float, float>(1.0, 0.0),
        std::pair<float, float>(1.0, 1.0),
        std::pair<float, float>(0.0, 1.0),
    };

    int n = 4;

    lemon::FullGraph* g = nullptr;
    lemon::FullGraph::EdgeMap<float>* cost = nullptr;

    std::tie(g, cost) = create_complete_graph(n, vertices);

    // for (int i = 0; i < g->edges().size(); i++)
    //     std::cout << "edge" << std::endl;


    // for( EdgeIt i(*g); i != lemon::INVALID; ++i ) {
    //     std::cout << " " << g->edgeFromId(g->id(i)).;
    // }

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            lemon::FullGraphBase::Node u = (*g)(i);
            lemon::FullGraphBase::Node v = (*g)(j);
            std::cout << "i = " << i << ", j = " << j << " --- "  << (*cost)[g->edge(u, v)];
            std::cout << std::endl;
        }
    }

    std::cout <<"foio";
    return 0;
}