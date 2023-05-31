#pragma once

#include <lemon/matching.h>
#include <lemon/adaptors.h>
#include <lemon/list_graph.h>

#include <limits>

#include "src/utils.h"

using namespace std;
using namespace lemon;

namespace ApxSMCP {


    void get_odd_degree_nodes(int n, int** graph, vector<int>* odd_vertices) {

        int* nodes_degree = new int[n];
        // init vector values
        for (int i = 0; i < n; i++)
            nodes_degree[i] = 0;

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {

                nodes_degree[i] += graph[i][j];
                nodes_degree[j] += graph[i][j];

            }
        }

        for (int i = 0; i < n; i++)
            if (nodes_degree[i] % 2)
                odd_vertices->push_back(i);

    }

    // init_graph_adaptor()

    // short_cutting()

    int** short_cutting(int n, int** sol) {
        
        vector<int> vertices_degrees;
        vertices_degrees.reserve(n);


        
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
            // TODO ../../../allInst/m10Q10s555.tsp.ccpdp tem vertice de grau impar
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
        print_matrix(n, sn_sol);
        sn_sol = short_cutting(n, sn_sol);

        return sn_sol;
    }

}
