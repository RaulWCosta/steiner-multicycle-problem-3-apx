#pragma once

#include <lemon/matching.h>
#include <lemon/adaptors.h>
#include <lemon/list_graph.h>
#include <tuple>

#include <limits>

#include "src/utils.h"

using namespace std;
using namespace lemon;

namespace ApxSMCP {


    void get_odd_degree_nodes(int n, int** sol, vector<int>* odd_vertices) {

        vector<int> degrees = get_vertices_degrees(n, sol);

        int* nodes_degree = new int[n];
        // init vector values
        for (int i = 0; i < n; i++)
            if (degrees[i] % 2)
                odd_vertices->push_back(i);

    }

    void get_neighboors(int n, int vert, int** sol, vector<int>* neighboors) {
        neighboors->clear();

        for (int i = 0; i < n; i++) {
            if (i == vert)
                continue;

            if (sol[i][vert])
                neighboors->push_back(i);
        }
    }

    tuple<int, int> get_best_shortcut(
        int n,
        int vert,
        vector<int>& neighbors,
        int** sol,
        FullGraph& graph,
        FullGraph::EdgeMap<float>& cost
    ) {

        float best_dist = 0;
        int best_vert_a = -1;
        int best_vert_b = -1;
        FullGraph::Node node_vert = graph(vert);

        for (auto& i : neighbors) {
            for (auto& j : neighbors) {
                if (i == j || sol[i][j])
                    continue;

                FullGraph::Node node_i = graph(i);
                FullGraph::Node node_j = graph(j);

                float shortcut_gain = cost[graph.edge(node_i, node_vert)];
                shortcut_gain += cost[graph.edge(node_vert, node_j)];
                shortcut_gain -= cost[graph.edge(node_i, node_j)];


                if (shortcut_gain > best_dist) {
                    best_dist = shortcut_gain;
                    best_vert_a = i;
                    best_vert_b = j;
                }
            }
        }

        return make_tuple(best_vert_a, best_vert_b);
    }

    int** short_cutting(int n, int** sol, FullGraph& graph, FullGraph::EdgeMap<float>& cost) {
        vector<int> vertices_to_short;
        int vert, a, b;
        vector<int> neighbors;

        vector<int> vertices_degrees = get_vertices_degrees(n, sol);
        for (int i = 0; i < n; i++)
            if (vertices_degrees[i] > 2)
                vertices_to_short.push_back(i);

        while(vertices_to_short.size()) {

            vert = vertices_to_short[vertices_to_short.size() - 1];
            vertices_to_short.pop_back();

            get_neighboors(n, vert, sol, &neighbors);

            tie(a, b) = get_best_shortcut(n, vert, neighbors, sol, graph, cost);

            if (a == -1 || b == -1)
                continue;

            // update solution
            sol[vert][a] -= 1;
            sol[a][vert] -= 1;
            sol[vert][b] -= 1;
            sol[b][vert] -= 1;
            sol[a][b] += 1;
            sol[b][a] += 1;

            vertices_degrees[vert] -= 2;

            if (vertices_degrees[vert] > 2)
                vertices_to_short.push_back(vert);

        }

        return sol;
    }

    int** solve(int n, int** sn_sol, FullGraph& graph, FullGraph::EdgeMap<float>& cost) {

        vector<int> odd_vertices;
        get_odd_degree_nodes(n, sn_sol, &odd_vertices);

        if (odd_vertices.size() > 0) {

            FullGraph::NodeMap<bool> filter(graph, false);
            for (auto& i : odd_vertices) {
                FullGraph::Node u = graph(i);
                filter.set(u, true);
            }
            FilterNodes<FullGraph> subgraph(graph, filter);
            cout << countNodes(subgraph) << endl;

            FilterNodes<FullGraph>::EdgeMap<float>* inverted_cost = new FilterNodes<FullGraph>::EdgeMap<float>(subgraph);

            for (auto& i : odd_vertices) {
                for (auto& j : odd_vertices) {
                    FullGraph::Node u = graph(i);
                    FullGraph::Node v = graph(j);
                    if (i == j) {
                        continue;
                    }
                    (*inverted_cost)[graph.edge(u, v)] = -cost[graph.edge(u, v)];
                }
            }

            MaxWeightedPerfectMatching< FilterNodes<FullGraph>, FullGraph::EdgeMap<float> > perf_match(subgraph, *inverted_cost);

            // TODO split init and start of algorithm for eficiency
            perf_match.run();

            for (auto& i : odd_vertices) {
                for (auto& j : odd_vertices) {
                    FullGraph::Node u = graph(i);
                    FullGraph::Node v = graph(j);
                    if (perf_match.matching(graph.edge(u, v))) {
                        sn_sol[i][j] += 1;
                    }
                }
            }
        }

        // print_matrix(n, sn_sol);
        cout << "before shortcutting = " << get_sol_val(n, sn_sol, graph, cost) << endl;
        sn_sol = short_cutting(n, sn_sol, graph, cost); // TODO fix
        cout << "after shortcutting = " << get_sol_val(n, sn_sol, graph, cost) << endl;
        // print_matrix(n, sn_sol);

        return sn_sol;
    }

}
