// // steiner-multicycle-problem-3-apx.cpp : Defines the entry point for the application.
// //

// #include <iostream>

// #include "gurobi_c++.h"
// #include "src/relaxed_solver_surviv_net.h"

// #include <vector>
// #include <utility>
// #include <tuple>

// #include <lemon/lmath.h>
// #include <lemon/core.h>
// #include <lemon/graph_to_eps.h>
// #include <lemon/concepts/graph_components.h>
// #include <lemon/full_graph.h>

// // int test()
// // {
// //     std::vector<int> source2sink = { 2, 3 };
// //     std::vector<Vertex> vertices = {
// //         Vertex(0.0f, 0.0f),
// //         Vertex(1.0f, 0.0f),
// //         Vertex(1.0f, 1.0f),
// //         Vertex(0.0f, 1.0f),
// //     };

// //     GRBEnv env = new GRBEnv(true);
// //     env.set("LogFile", "mip1.log");
// //     env.start();

// //     // Create an empty model
// //     GRBModel model = GRBModel(env);

// //     RelaxedSurvivableNetworkSolver solver(model, 4, source2sink, vertices);

// //     solver.solve(model);

// //     return 0;
// // }

// float vertices_distance(const std::pair<float, float>& a, const std::pair<float, float>& b) {
//     float dx = a.first - b.first;
//     float dy = a.second - b.second;

//     return sqrt(dx * dx + dy * dy);
// }


// std::tuple<lemon::FullDigraph*, lemon::FullDigraph::ArcMap<float>*> create_complete_graph(int n, std::vector<std::pair<float, float>>& vertices) {
//     lemon::FullDigraph* g = new lemon::FullDigraph(n);

//     lemon::FullDigraph::ArcMap<float>* cost = new lemon::FullDigraph::ArcMap<float>(*g);


//     for (int i = 0; i < n; i++) {
//         for (int j = i + 1; j < n; j++) {
//             lemon::FullDigraph::Node u = (*g)(i);
//             lemon::FullDigraph::Node v = (*g)(j);
//             float dist = vertices_distance(vertices[i], vertices[j]);
//             (*cost)[g->arc(u, v)] = dist;
//             (*cost)[g->arc(v, u)] = dist;
//         }
//     }

//     for (int i = 0; i < n; i++) {
//         lemon::FullDigraph::Node u = (*g)(i);
//         (*cost)[g->arc(u, u)] = 0.0f;
//     }

//     return {g, cost};
// }

// int main(int argc, char* argv[]) {
//     // test();
//     std::vector<std::pair<float, float>> vertices = {
//         std::pair<float, float>(0.0, 0.0),
//         std::pair<float, float>(1.0, 0.0),
//         std::pair<float, float>(1.0, 1.0),
//         std::pair<float, float>(0.0, 1.0),
//     };

//     std::vector<int> source2sink = { 2, 3 };

//     int n = 4;

//     lemon::FullDigraph* g = nullptr;
//     lemon::FullDigraph::ArcMap<float>* cost = nullptr;

//     std::tie(g, cost) = create_complete_graph(n, vertices);

//     for (int i = 0; i < n; i++) {
//         for (int j = 0; j < n; j++) {
//             lemon::FullDigraph::Node u = (*g)(i);
//             lemon::FullDigraph::Node v = (*g)(j);
//             std::cout << "i = " << i << ", j = " << j << " --- "  << (*cost)[g->arc(u, v)];
//             std::cout << std::endl;
//         }
//     }

//     // GRBEnv env = new GRBEnv(true);
//     // env.set("LogFile", "mip1.log");
//     // env.start();

//     // Create an empty model
//     // GRBModel model = GRBModel(env);

//     RelaxedSurvivableNetworkSolver solver(4, source2sink, g, cost);

// // GRBModel &model, int n, const std::vector<int> &source_to_sink,
// //         lemon::FullDigraph *graph, lemon::FullDigraph::ArcMap<float> *cost

//     std::cout <<"foio";
//     return 0;
// }


#include <vector>
#include <utility>

#include "src/new_2apx.h"

using namespace std;

int main(int argc, char* argv[]) {
    int n = 4;

    vector<int> source2sink = {2, 3};
    vector<pair<float, float>> vertices = {
        pair<float, float>(0.0, 0.0),
        pair<float, float>(1.0, 0.0),
        pair<float, float>(1.0, 1.0),
        pair<float, float>(0.0, 1.0),
    };

    solve(n, source2sink, vertices);
}
