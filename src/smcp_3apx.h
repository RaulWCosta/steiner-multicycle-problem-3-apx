#pragma once

#include <lemon/matching.h>
#include <lemon/adaptors.h>
#include <lemon/list_graph.h>
#include <tuple>
#include <unordered_set>
#include <list>
#include <limits>

#include "src/surv_net_2apx.h"
#include "src/utils.h"

using namespace lemon;

namespace ApxSMCP {


    void get_odd_degree_nodes(int n, int** sol, vector<int>* odd_degree_nodes) {

        vector<int> degrees = get_vertices_degrees(n, sol);

        // init vector values
        for (int i = 0; i < n; i++)
            if (degrees[i] % 2)
                odd_degree_nodes->push_back(i);

    }

    void calculate_sol_euclidean_path(int n, int init_vert, int** sol, vector<int>* stack, list<int>* euclidean_path) {

        stack->push_back(init_vert);
        while (stack->size()) {
            int curr_vert = (*stack)[stack->size() - 1];
            stack->pop_back();

            // bool added_neighboor = false;
            for (int i = 0; i < n; i++) {
                if (i == curr_vert)
                    continue;
                if (sol[curr_vert][i]) {
                    // added_neighboor = true;
                    stack->push_back(i);
                    sol[curr_vert][i]--;
                    sol[i][curr_vert]--;
                }
            }
            // if (!added_neighboor) {
            //     break;
            // }

            euclidean_path->push_back(curr_vert);
        }

        euclidean_path->push_back(init_vert); // close cycle
    }

    void add_euclidian_path_in_solution(int n, std::list<int>& euclidean_path, int** sol) {
        int a, b;
        auto it = euclidean_path.begin();
        auto end_it = euclidean_path.end();
        if (it != end_it) {
            a = *it;
            ++it;
        }
        for (; it != end_it; ++it) {
            b = *it;

            sol[a][b] += 1;
            sol[b][a] += 1;

            a = b;
        }
    }

    void shortcut_euclidian_path(int n, int init_vert, list<int>* euclidean_path) {
        vector<bool> visited_in_path(n, false);
        // shortcut euclidean path

        int curr_vert = init_vert;
        visited_in_path[curr_vert] = true;

        list<int>::iterator it = next(euclidean_path->begin());
        list<int>::iterator end_it = prev(euclidean_path->end());

        vector<list<int>::iterator> to_remove_entries;

        int count = 0;
        for (; it != end_it; ++it) {
            int i = *it;
            if (visited_in_path[i]) {
                to_remove_entries.push_back(it);
            }
            visited_in_path[i] = true;
            count++;
        }

        for (auto it = to_remove_entries.rbegin(); it != to_remove_entries.rend(); ++it) {
            euclidean_path->erase(*it);
        }

    }

    int** short_cutting(int n, int** sol) {

        // get any initial vertex
        int curr_vert = 0;

        list<int>* euclidean_path = new list<int>;

        vector<bool> visited(n, false);
        visited[curr_vert] = true;

        vector<int>* stack = new vector<int>;

        // for each component
        while (curr_vert != -1) {

            stack->clear();
            euclidean_path->clear();

            calculate_sol_euclidean_path(n, curr_vert, sol, stack, euclidean_path);

            shortcut_euclidian_path(n, curr_vert, euclidean_path);

            add_euclidian_path_in_solution(n, *euclidean_path, sol);

            for (auto it = euclidean_path->begin(); it != euclidean_path->end(); ++it) {
                int i = *it;
                visited[i] = true;
            }

            curr_vert = -1;
            for (int i = 0; i < n; i++) {
                if (!visited[i]) {
                    curr_vert = i;
                    break;
                }
            }
        }

        delete stack;
        delete euclidean_path;

        return sol;
    }

    int** solve(
        int n,
        float** edges_weights,
        ListGraph* graph,
        ListGraph::EdgeMap<float>* cost,
        int** sn_sol
    ) {

        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                sn_sol[i][j] = 0;

        SurvivableNetwork::init_graph(n, graph);
        
        SurvivableNetwork::init_cost_map(n, *graph, edges_weights, cost);

        SurvivableNetwork::solve(n, graph, cost, sn_sol);

        std::vector<int> odd_degree_nodes;
        odd_degree_nodes.reserve(n + 1);

        get_odd_degree_nodes(n, sn_sol, &odd_degree_nodes);

        if (odd_degree_nodes.size() > 0) {

            ListGraph::NodeMap<bool> filter(*graph, false);
            for (auto& i : odd_degree_nodes) {
                ListGraph::Node u = graph->nodeFromId(i);
                filter.set(u, true);
            }
            FilterNodes<ListGraph> subgraph(*graph, filter);
            // std::cout << countNodes(subgraph) << std::endl;

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

            delete inverted_cost;

        }

        // print_matrix(n, sn_sol);
        // verify_solution("file", n, sn_sol);

        // cout << "before shortcutting = " << get_sol_val(n, sn_sol, edges_weights) << endl;
        // print_matrix(n, sn_sol);
        sn_sol = short_cutting(n, sn_sol);
        // cout << "after shortcutting = " << get_sol_val(n, sn_sol, edges_weights) << endl;

        return sn_sol;
    }

}
