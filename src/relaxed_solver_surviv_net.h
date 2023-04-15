#pragma once

#include "gurobi_c++.h"

#include <vector>
#include <iostream>
#include <limits.h>
#include <queue>
#include <string.h>


class Flow {
public:
    int _n;
    bool* _visited;
    int* _parent_path;

    Flow(int n)
        : _n(n)
    {
        _visited = new bool[_n];
        _parent_path = new int[_n];
    }

    // max flow is greater than 2?
    bool fordFulkerson(int source, int sink, double** residual_graph, int* path) {
        int u, v;
        path = _parent_path;
        memset(_parent_path, -1, sizeof(*_parent_path) * _n);

        double max_flow = 0.0f; // There is no flow initially

        while (bfs(source, sink, residual_graph)) {

            double path_flow = std::numeric_limits<double>::max();
            for (v = source; v != sink; v = _parent_path[v]) {
                u = _parent_path[v];
                path_flow = std::min(path_flow, residual_graph[u][v]);
            }

            // update residual capacities of the edges and
            // reverse edges along the path
            for (v = source; v != sink; v = _parent_path[v]) {
                u = _parent_path[v];
                residual_graph[u][v] -= path_flow;
                residual_graph[v][u] += path_flow; // talvez tenha que ser mudado
            }

            // Add path flow to overall flow
            max_flow += path_flow;
            if (max_flow >= 2) {
                return true;
            }
        }
        return false;
    }

    
    bool bfs(int source, int sink, double** residual_graph) {

        memset(_visited, 0, _n);
        std::queue<int> q;

        q.push(source);
        _visited[source] = true;
        _parent_path[source] = -1;

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (int v = 0; v < _n; v++) {
                if (!_visited[v] && residual_graph[u][v] > 0) {
                    if (v == sink) {
                        _parent_path[v] = u;
                        return true;
                    }
                    q.push(v);
                    _parent_path[v] = u;
                    _visited[v] = true;
                }
            }
        }
        return false;
    }

};

class connectivity_constrain : public GRBCallback {
public:
    GRBVar** _edges_vars;
    double** _current_solution;
    int _n;
    std::vector<int> _source_to_sink;
    Flow* _flow;

    connectivity_constrain(GRBVar** edges_vars, int n, const std::vector<int>& source_to_sink)
        : _edges_vars(edges_vars), _n(n), _source_to_sink(source_to_sink)
    {
        _current_solution = new double* [_n];
        for (int i = 0; i < _n; i++)
            _current_solution[i] = new double[_n];

        _flow = &Flow(_n);
    }

protected:

    double** update_current_solution() {
        for (int i = 0; i < _n; i++)
            _current_solution[i] = getSolution(_edges_vars[i], _n);
        
        for (int i = 0; i < _n; i++) {
            for (int j = 0; j <= i; j++) {
                _current_solution[i][j] = (_current_solution[i][j] >= 0.5) ? 1.0 : _current_solution[i][j];
            }
        }

        return _current_solution;
    }

    void add_restrictions(const int* path) {

    }


    void callback() {
        double** current_solution = update_current_solution();
        int* path;

        for(int i =  0; i < _source_to_sink.size(); i++) {
            if (!_flow->fordFulkerson(i, _source_to_sink[i], current_solution, path)) {
                // flow is not greater or equal to 2
                // needs adicional restrictions
                add_restrictions(path);
            }
        }
        
        
    }

    // void callback() {
    //     try {
    //         // update residuals graph
    //         update_restrictions(_edges_vars);

    //         // run maxflow to check if solution is feeasible
    //         for (int i = 0; i < _source_to_sink.size(); i++) {
    //             int sink = _source_to_sink[i];
    //             if (!_flow.fordFulkerson(i, sink)) {
    //                 // update restrictions based on parent path
    //                 update_restrictions(i, sink);
    //             }
    //         }
    //     }
    //     catch (GRBException e) {
    //         std::cout << "Error number: " << e.getErrorCode() << std::endl;
    //         std::cout << e.getMessage() << std::endl;
    //     }
    //     catch (...) {
    //         std::cout << "Error during callback" << std::endl;
    //     }
    // }


};

struct Vertex {
    float x, y;
    // int degree = 0;

    Vertex(float x_, float y_) : x(x_), y(y_) {}
};

double distance(const Vertex& a, const Vertex& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;

    return sqrt(dx * dx + dy * dy);
}

class RelaxedSurvivableNetworkSolver {

public:
    GRBVar** _edges_vars = nullptr;
    GRBLinExpr* _degree_vars = nullptr;
    int _n;

	RelaxedSurvivableNetworkSolver(GRBModel& model, int n, const std::vector<int>& source_to_sink, const std::vector<Vertex>& vertices) {
        _n = n;

        _edges_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            _edges_vars[i] = new GRBVar[n];

        _degree_vars = new GRBLinExpr [n];

        model.set(GRB_IntParam_LazyConstraints, 1);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                _edges_vars[i][j] = model.addVar(0.0, 1.0, distance(vertices[i], vertices[j]),
                    GRB_CONTINUOUS, "edge_" + std::to_string(i) + "_" + std::to_string(j));
                _edges_vars[j][i] = _edges_vars[i][j];
            }
        }

        // Forbid edge from node back to itself
        for (int i = 0; i < n; i++) {
            _edges_vars[i][i].set(GRB_DoubleAttr_UB, 0);
        }

        // every vertex needs degree >= 2
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                _degree_vars[i] += _edges_vars[i][j];
            }
            model.addConstr(_degree_vars[i] >= 2);
        }

        // connectivity_constrain cb = connectivity_constrain(_edges_vars, n, source_to_sink);
        // model.setCallback(&cb);

        // model.optimize();
        // return;

	}

    ~RelaxedSurvivableNetworkSolver() {
        
    }

    void solve(GRBModel& model) {
        std::cout << "solve!" << std::endl;
        model.optimize();

        double** sol = new double* [_n];
        // for (int  i = 0; i  < _n; i++)
        //     sol[i] = new double[_n];

        for (int i = 0; i < _n; i++) {
            // for (int j = 0; j < n; j++) {
            sol[i] = model.get(GRB_DoubleAttr_X, _edges_vars[i], _n);
            // }
            // model.addConstr(_degree_vars[i] >= 2);
        }

        for (int i = 0; i < _n; i++)
            for (int j = 0; j <= i; j++) {
                std::cout << "[" << std::to_string(i) << ", " << std::to_string(j);
                std::cout << "] = " << sol[i][j] << std::endl;
            }
        // _edges_vars
    }

};
