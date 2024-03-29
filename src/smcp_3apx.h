#pragma once

#include <lemon/adaptors.h>
#include <lemon/list_graph.h>
#include <lemon/matching.h>

#include <limits>
#include <list>
#include <tuple>
#include <unordered_set>

#include "src/surv_net_2apx.h"
#include "src/utils.h"

using namespace lemon;

namespace ApxSMCP {

void get_odd_degree_nodes(int n, int** sol, vector<int>* odd_degree_nodes) {
  vector<int> degrees = get_vertices_degrees(n, sol);

  // init vector values
  for (int i = 0; i < n; i++)
    if (degrees[i] % 2) odd_degree_nodes->push_back(i);
}

void calculate_sol_euclidean_path(int n, int init_vert, float** edges_weights,
                                  int** sol, vector<int>* neighboors,
                                  list<int>* euclidean_path) {
  neighboors->push_back(init_vert);
  while (neighboors->size()) {
    if (euclidean_path->size()) {
      // move closest vertex to last vertex on euclidean_path to the end of the
      // neighboors vector
      //  this is done to improvement the performance of the short-cutting
      float best_val = std::numeric_limits<float>::max();
      int best_idx;

      int last_vertex = euclidean_path->back();
      for (int i = 0; i < neighboors->size(); i++) {
        if (edges_weights[last_vertex][(*neighboors)[i]] < best_val) {
          best_val = edges_weights[last_vertex][(*neighboors)[i]];
          best_idx = i;
        }
      }

      // put closest at the end
      int aux;
      aux = (*neighboors)[neighboors->size() - 1];
      (*neighboors)[neighboors->size() - 1] = (*neighboors)[best_idx];
      (*neighboors)[best_idx] = aux;
    }

    int curr_vert = (*neighboors)[neighboors->size() - 1];
    neighboors->pop_back();

    for (int i = 0; i < n; i++) {
      if (i == curr_vert) continue;
      if (sol[curr_vert][i]) {
        neighboors->push_back(i);
        sol[curr_vert][i]--;
        sol[i][curr_vert]--;
      }
    }

    euclidean_path->push_back(curr_vert);
  }

  euclidean_path->push_back(init_vert);  // close cycle
}

void add_euclidian_path_in_solution(int n, std::list<int>& euclidean_path,
                                    int** sol) {
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

  for (; it != end_it; ++it) {
    int i = *it;
    if (visited_in_path[i]) {
      to_remove_entries.push_back(it);
    }
    visited_in_path[i] = true;
  }

  for (auto it = to_remove_entries.rbegin(); it != to_remove_entries.rend();
       ++it) {
    euclidean_path->erase(*it);
  }
}

int** short_cutting(int n, float** edges_weights, list<int>* euclidean_path,
                    vector<int>* stack, int** sol) {
  // get any initial vertex
  int curr_vert = 0;

  vector<bool> visited(n, false);
  visited[curr_vert] = true;

  // for each component
  while (curr_vert != -1) {
    stack->clear();
    euclidean_path->clear();

    calculate_sol_euclidean_path(n, curr_vert, edges_weights, sol, stack,
                                 euclidean_path);

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

  return sol;
}

int** solve_without_shortcutting(int n, float** edges_weights, ListGraph* graph,
                                 ListGraph::EdgeMap<float>* cost,
                                 int** sn_sol) {
  // assumes sn_sol contains survivable network solution

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

    FilterNodes<ListGraph>::EdgeMap<float>* inverted_cost =
        new FilterNodes<ListGraph>::EdgeMap<float>(subgraph);

    for (FilterNodes<ListGraph>::EdgeIt e(subgraph); e != INVALID; ++e) {
      (*inverted_cost)[e] = -(*cost)[e];
    }

    MaxWeightedPerfectMatching<FilterNodes<ListGraph>,
                               ListGraph::EdgeMap<float> >
        perf_match(subgraph, *inverted_cost);

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

  return sn_sol;
}

}  // namespace ApxSMCP
