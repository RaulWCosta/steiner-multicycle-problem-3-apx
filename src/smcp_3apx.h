#pragma once

#include <lemon/max_matching.h>

namespace ApxSMCP {

    FullGraph* create_graph() {
        
        
    }


    void solve(int n, int** graph, float** dists) {
        vector<int> odd_degree_vertices = new vector<int>;

        for (int i = 0; i < n; i++) {
            int acc = 0;
            for (int j = 0; j < n; j++) {
                acc += graph[i][j];
                acc += graph[j][i];
            }
            if (acc % 2) {
                // odd degree
                odd_degree_vertices.push_back(i);
            }
        }

        if (odd_degree_vertices.size() > 0) {
            
            int n_odd_vert = odd_degree_vertices.size();

            FullGraph* graph = new FullGraph(n_odd_vert);
            FullGraph::EdgeMap<float> cost = new FullGraph::EdgeMap<float>(*graph);

            for (int i = 0; i < n_odd_vert; i++) {
                for (int j = 0; j < n_odd_vert; j++) {
                    FullGraph::Node u = (*graph)(i);
                    FullGraph::Node v = (*graph)(j);
                    cost[graph->edge(u, v)] = -dists[i][j];
                }
            }


            // MaxWeightedPerfectMatching(graph, cost);

        }

    }

}