#pragma once

#include <lemon/matching.h>

FullGraph* _graph;

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

    int** solve(int n, int** sn_sol, vector<pair<float, float>>& vertices) {

        vector<int>* odd_vertices = get_odd_degree_nodes(n, sn_sol);

        if (odd_vertices.size() > 0) {
            
        }


        // F
        int** int_solution = new int* [n];
        for (int i = 0; i < n; i++)
            int_solution[i] = new int[n];

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                int_solution[i][j] = 0;

        bool flag_valid_solution = false;

        // cria modelo e adiciona restrições "base"
        GRBVar** edge_vars = NULL;
        edge_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            edge_vars[i] = new GRBVar[n];
        GRBModel* model = init_gurobi_model(n, edge_vars, vertices);

        // vector with vertices within an invalid cycle, i.e. there is some vertex in the cycle which
        //  is not connected to it's pair

        LPSolver lp_solver = LPSolver(n, model, edge_vars, source2sink);

        // enquanto modelo nao retorna solucao viavel
        while(true) {

            // rodar LP até solução viável do relaxado
            double** lp_solution = lp_solver.solve();

            print_matrix(n, lp_solution);
            print_matrix(n, int_solution);

            // adiciona valores >= 0.5 no int_solution
            update_int_solution(n, lp_solution, int_solution, edge_vars, *model);

            print_matrix(n, int_solution);

            flag_valid_solution = is_valid_int_solution(n, int_solution, source2sink);

            if (flag_valid_solution) {
                return int_solution;
            }

            for (int i = 0; i < n; i++)
                delete[] lp_solution[i];
            delete[] lp_solution;

        }

    }

}



// ListDigraph g;
// ListDigraph::Node x = g.addNode();
// ListDigraph::Node y = g.addNode();
// ListDigraph::Node z = g.addNode();
// ListDigraph::NodeMap<bool> filter(g, true);
// FilterNodes<ListDigraph> subgraph(g, filter);
// std::cout << countNodes(subgraph) << ", ";
// filter[x] = false;
// std::cout << countNodes(subgraph) << ", ";
// subgraph.enable(x);
// subgraph.disable(y);
// subgraph.status(z, !subgraph.status(z));
// std::cout << countNodes(subgraph) << std::endl;