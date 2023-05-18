#pragma once

#include <lemon/matching.h>
#include <lemon/adaptors.h>

#include <limits>

using namespace std;
using namespace lemon;

namespace ApxSMCP {


    vector<int>* get_odd_degree_nodes(int n, int** graph) {

        int* nodes_degree = new int[n];
        // init vector values
        for (int i = 0; i < n; i++)
            nodes_degree[i] = 0;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graph[i][j]) {
                    nodes_degree[i]++;
                    nodes_degree[j]++;
                }
            }
        }
        vector<int>* odd_vertices = new vector<int>;
        for (int i = 0; i < n; i++)
            if (nodes_degree[i] % 2)
                odd_vertices->push_back(i);
        
        return odd_vertices;
    }

    // init_graph_adaptor()

    // short_cutting()

    void short_cutting(int** sol) {

    }

    int** solve(int n, int** sn_sol, FullGraph& graph, FullGraph::EdgeMap<float>& cost) {

        vector<int>* odd_vertices = get_odd_degree_nodes(n, sn_sol);

        if (odd_vertices->size() == 0) {  
            return sn_sol;
        }

        FullGraph::NodeMap<bool> filter(graph, false);
        for (auto& i : *odd_vertices) {
            FullGraph::Node u = graph(i);
            filter.set(u, true);
        }
        FilterNodes<FullGraph> subgraph(graph, filter);
        cout << countNodes(subgraph) << endl;

        FullGraph::EdgeMap<float>* inverted_cost = new FullGraph::EdgeMap<float>(subgraph);

        for (auto& i : *odd_vertices) {
            for (auto& j : *odd_vertices) {
                FullGraph::Node u = graph(i);
                FullGraph::Node v = graph(j);
                if (i == j) {
                    (*inverted_cost)[graph.edge(u, v)] = numeric_limits<float>::min();
                    continue;
                }
                (*inverted_cost)[graph.edge(u, v)] = -vertices_distance((*odd_vertices)[i], (*odd_vertices)[j]);
            }
        }

        MaxWeightedPerfectMatching< FilterNodes<FullGraph>, FullGraph::EdgeMap<float> > perf_match(subgraph, *inverted_cost);

        // TODO split init and start of algorithm for eficiency
        perf_match.run();

        for (auto& i : *odd_vertices) {
            for (auto& j : *odd_vertices) {
                FullGraph::Node u = graph(i);
                FullGraph::Node v = graph(j);
                if (perf_match.matching(graph.edge(u, v))) {
                    sn_sol[i][j] = 1;
                }
            }
        }

        return sn_sol;
    }

}
