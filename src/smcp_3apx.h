#pragma once

#include <lemon/matching.h>
#include <lemon/adaptors.h>
#include <lemon/list_graph.h>
#include <tuple>
#include <unordered_set>

#include <limits>

#include "src/surv_net_2apx.h"
#include "src/utils.h"

using namespace std;
using namespace lemon;

namespace ApxSMCP {


    void get_odd_degree_nodes(int n, int** sol, vector<int>* odd_degree_nodes) {

        vector<int> degrees = get_vertices_degrees(n, sol);

        int* nodes_degree = new int[n];
        // init vector values
        for (int i = 0; i < n; i++)
            if (degrees[i] % 2)
                odd_degree_nodes->push_back(i);

    }

    void calculate_sol_euclidean_path(int n, int init_vert, int** sol_cp, vector<int>* euclidean_path) {
        vector<int>* stack = new vector<int>;
        stack->push_back(init_vert);
        while (stack->size()) {
            int curr_vert = (*stack)[stack->size() - 1];
            stack->pop_back();

            // bool added_neighboor = false;
            for (int i = 0; i < n; i++) {
                if (i == curr_vert)
                    continue;
                if (sol_cp[curr_vert][i]) {
                    // added_neighboor = true;
                    stack->push_back(i);
                    sol_cp[curr_vert][i]--;
                    sol_cp[i][curr_vert]--;
                }
            }
            // if (!added_neighboor) {
            //     break;
            // }

            euclidean_path->push_back(curr_vert);
        }
        delete stack;
    }

    void shortcut_euclidian_path_in_solution(int n, vector<int>& euclidean_path, int** sol) {
        vector<bool> visited_in_path(n, false);
        // shortcut euclidean path
        int a, b;

        int curr_vert = euclidean_path[0];
        visited_in_path[curr_vert] = true;

        for (int i = 1; i < euclidean_path.size() - 1; i++) {
            curr_vert = euclidean_path[i];
            if (visited_in_path[curr_vert]) {
                // execute shortcut
                a = euclidean_path[i-1];
                b = euclidean_path[i+1];
                if (a == b || b == curr_vert || curr_vert == a)
                    continue;

                if (!sol[a][curr_vert] || !sol[b][curr_vert])
                    continue; // this is an error case

                sol[a][curr_vert]--;
                sol[curr_vert][a]--;
                sol[b][curr_vert]--;
                sol[curr_vert][b]--;
                sol[b][a]++;
                sol[a][b]++;


                euclidean_path[i] = euclidean_path[i-1];
            }
            visited_in_path[curr_vert] = true;
        }
    }

    int** short_cutting(int n, int** sol) {

        int **sol_cp = new int* [n];
        for (int i = 0; i < n; i++)
            sol_cp[i] = new int[n];


        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                sol_cp[i][j] = sol[i][j];
            }
        }

        // get any initial vertex
        int curr_vert = 0;

        vector<int>* euclidean_path = new vector<int>;

        vector<bool> visited(n, false);
        visited[curr_vert] = true;

        // for each component
        while (curr_vert != -1) {

            euclidean_path->clear();

            calculate_sol_euclidean_path(n, curr_vert, sol_cp, euclidean_path);

            // cout << "copy!" << endl;
            // print_matrix(n, sol_cp);

            shortcut_euclidian_path_in_solution(n, *euclidean_path, sol);

            // update visited node to search next componenet
            for (int i = 0; i < euclidean_path->size(); i++)
                visited[(*euclidean_path)[i]] = true;

            curr_vert = -1;
            for (int i = 0; i < n; i++) {
                if (!visited[i]) {
                    curr_vert = i;
                    break;
                }
            }
        }


        delete euclidean_path;
        for (int i = 0; i < n; i++)
            delete[] sol_cp[i];
        delete[] sol_cp;

        return sol;
    }

    int** solve(int n, float** edges_weights, int** sn_sol) {

        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                sn_sol[i][j] = 0;

        ListGraph* graph = new ListGraph();
        graph->reserveNode(n);
        graph->reserveEdge((n * n) >> 1);

        SurvivableNetwork::init_graph(n, graph);
        ListGraph::EdgeMap<float>* cost = new ListGraph::EdgeMap<float>(*graph);
        SurvivableNetwork::init_cost_map(n, *graph, edges_weights, cost);

        SurvivableNetwork::solve(n, graph, cost, sn_sol);

        vector<int> odd_degree_nodes;
        get_odd_degree_nodes(n, sn_sol, &odd_degree_nodes);

        if (odd_degree_nodes.size() > 0) {

            ListGraph::NodeMap<bool> filter(*graph, false);
            for (auto& i : odd_degree_nodes) {
                ListGraph::Node u = graph->nodeFromId(i);
                filter.set(u, true);
            }
            FilterNodes<ListGraph> subgraph(*graph, filter);
            cout << countNodes(subgraph) << endl;

            FilterNodes<ListGraph>::EdgeMap<float>* inverted_cost = new FilterNodes<ListGraph>::EdgeMap<float>(subgraph);


            for (FilterNodes<ListGraph>::EdgeIt e(subgraph); e != INVALID; ++e) {
                (*inverted_cost)[e] = -(*cost)[e];
            }

            MaxWeightedPerfectMatching< FilterNodes<ListGraph>, ListGraph::EdgeMap<float> > perf_match(subgraph, *inverted_cost);

            perf_match.run();

            for (FilterNodes<ListGraph>::EdgeIt e(subgraph); e != INVALID; ++e) {
                if (perf_match.matching(e)) {
                    int left = subgraph.id(subgraph.u(e));
                    int right = subgraph.id(subgraph.v(e));

                    sn_sol[left][right] += 1;
                    sn_sol[right][left] += 1;
                }
            }

        }

        // print_matrix(n, sn_sol);
        verify_solution("file", n, sn_sol);

        cout << "before shortcutting = " << get_sol_val(n, sn_sol, edges_weights) << endl;
        print_matrix(n, sn_sol);
        sn_sol = short_cutting(n, sn_sol); // TODO fix/
        cout << "after shortcutting = " << get_sol_val(n, sn_sol, edges_weights) << endl;
        print_matrix(n, sn_sol);

        // check shortcut
        for (int i = 0; i < n; i++) {
            int acc = 0;
            for (int j = 0; j < n; j++) {
                acc += sn_sol[i][j];
                if (sn_sol[i][j] > 2)
                    cout << "error pior" << endl;
            }
            if (acc > 2) {
                cout << "error!" << endl;
            }
        }

        delete cost;
        delete graph;

        return sn_sol;
    }

}
