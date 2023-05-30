#pragma once

#include "gurobi_c++.h"

#include <lemon/full_graph.h>
#include <lemon/preflow.h>
#include <unordered_set>

#include "src/utils.h"

using namespace std;
using namespace lemon;

namespace ExactSMCP {

    bool is_valid_int_solution(int n, int** current_solution, vector<int>* stack) {
        int half_n = (int)(n/2);
        for (int s = 0; s < half_n; s++) {
            int t = s + half_n;

            bool flag_valid_cycle = false;
            vector<bool> visited(n, false);

            stack->clear();

            visited[s] = true;
            stack->push_back(s);

            while (!stack->empty()) {
                int curr_node = (*stack)[stack->size() - 1];
                stack->pop_back();

                if (curr_node == t) {
                    flag_valid_cycle = true;
                    break;
                }

                for (int i = 0; i < n; i++) {
                    if (!current_solution[curr_node][i])
                        continue;

                    if (!visited[i]) {
                        visited[i] = true;
                        stack->push_back(i);
                    }
                }
            }

            if (!flag_valid_cycle) {
                // ciclo invalido!
                return false;
            }

        }
        return true;

    }

    class FeasibleSolCallback : public GRBCallback
    {
    public:
        GRBVar** _edge_vars;
        int _n;
        int** _curr_sol;
        vector<int>* _invalid_cycle;

        FeasibleSolCallback(GRBVar** vars, int n)
            : _edge_vars(vars), _n(n)
        {
            _curr_sol = new int* [n];
            _invalid_cycle = new vector<int>;
            for (int i = 0; i < n; i++)
                _curr_sol[i] = new int[n];
        }

    protected:
        void callback() {
            try {
                if (where == GRB_CB_MIPSOL) {
                    double* tmp_solution;
                    for (int i = 0; i < _n; i++) {
                        tmp_solution = getSolution(_edge_vars[i], _n);
                        for (int j = i + 1; j < _n; j++)
                            _curr_sol[i][j] = (int)tmp_solution[j];
                    }

                    if (!is_valid_int_solution(_n, _curr_sol, _invalid_cycle)) {
                        // add restrictions
                        update_model_constrains(*_invalid_cycle);
                    }

                    delete[] tmp_solution;
                }
                return;
            }
            catch (GRBException e) {
                cout << "Error number: " << e.getErrorCode() << endl;
                cout << e.getMessage() << endl;
            }
            catch (...) {
                cout << "Error during callback" << endl;
            }
            exit(1);
        }

        void update_model_constrains(vector<int>& invalid_cycle) {

            unordered_set<int> s_set;
            for(int& node : invalid_cycle) {
                s_set.insert(node);
            }

            vector<int> s_values;
            vector<int> not_s_values;

            for (int i = 0; i < _n; i++) {
                const bool is_in = s_set.find(i) != s_set.end();

                if (is_in) {
                    s_values.push_back(i);
                } else {
                    not_s_values.push_back(i);
                }
            }

            GRBLinExpr expr = 0;
            for (int& i : s_values) {
                for (int& j : not_s_values) {
                    expr += _edge_vars[i][j];
                }
            }
            addLazy(expr >= 2);

        }

    };

    void solve(int n, vector<pair<float, float>>& vertices, int** int_sol) {

        GRBEnv* env = new GRBEnv();
        GRBModel model = GRBModel(*env);

        GRBVar** edge_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            edge_vars[i] = new GRBVar[n];

        // Must set LazyConstraints parameter when using lazy constraints
        model.set(GRB_IntParam_LazyConstraints, 1);

        // Create decision variables

        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                float dist = vertices_distance(vertices[i], vertices[j]);
                edge_vars[i][j] = model.addVar(0.0, 2.0, dist,
                    GRB_INTEGER, "x_" + itos(i) + "_" + itos(j));
                edge_vars[j][i] = edge_vars[i][j];
            }
        }

        // Forbid edge from node back to itself
        for (int i = 0; i < n; i++)
            edge_vars[i][i].set(GRB_DoubleAttr_UB, 0);

        // Degree-2 constraints
        vector<GRBLinExpr> expr_vec(n, 0);

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                expr_vec[i] += edge_vars[i][j];
                expr_vec[j] += edge_vars[i][j];
            }
        }
        for (int i = 0; i < n; i++)
            model.addConstr(expr_vec[i] >= 2, "deg2_" + itos(i));

        // set callback
        FeasibleSolCallback cb = FeasibleSolCallback(edge_vars, n);
        model.setCallback(&cb);

        double** sol = new double* [n];

        model.optimize();

        if (model.get(GRB_IntAttr_SolCount) > 0) {
            for (int i = 0; i < n; i++)
                sol[i] = model.get(GRB_DoubleAttr_X, edge_vars[i], n);
        }

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                int_sol[i][j] = (int)(sol[i][j] + 0.001);

        // print_matrix(n, int_sol);

        for (int i = 0; i < n; i++)
            delete[] sol[i];
        delete[] sol;
    }


}
