#pragma once

#include "gurobi_c++.h"

#include <lemon/full_graph.h>
#include <lemon/preflow.h>
#include <unordered_set>

#include "src/utils.h"

using namespace std;
using namespace lemon;

namespace Exact {

    bool is_valid_int_solution(int n, int** current_solution, vector<int>& source2sink, vector<int>* stack) {

        for (int s = 0; s < source2sink.size(); s++) {
            int t = source2sink[s];

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
        vector<int> _source2sink;

        FeasibleSolCallback(GRBVar** vars, int n, vector<int>& source2sink)
            : _edge_vars(vars), _n(n), _source2sink(source2sink)
        {
            _curr_sol = new int* [n];
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

                    vector<int>* invalid_cycle;

                    if (!is_valid_int_solution(_n, _curr_sol, _source2sink, invalid_cycle)) {
                        // add restrictions
                        update_model_constrains(*invalid_cycle);
                    }

                    delete[] tmp_solution;
                }
            }
            catch (GRBException e) {
                cout << "Error number: " << e.getErrorCode() << endl;
                cout << e.getMessage() << endl;
            }
            catch (...) {
                cout << "Error during callback" << endl;
            }
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
                    if (j > i)
                        expr += _edge_vars[i][j];
                    else
                        expr += _edge_vars[j][i];
                }
            }
            addLazy(expr >= 2);

        }

    };

    GRBModel* init_gurobi_model(
        int n,
        GRBVar** edge_vars,
        vector<pair<float, float>>& vertices,
        vector<int>& source2sink
    ) {

        GRBEnv* env = NULL;

        try {

            env = new GRBEnv();
            GRBModel* model = new GRBModel(*env);

            // Must set LazyConstraints parameter when using lazy constraints
            model->set(GRB_IntParam_LazyConstraints, 1);

            // Create decision variables

            for (int i = 0; i < n; i++) {
                for (int j = i + 1; j < n; j++) {
                    float dist = vertices_distance(vertices[i], vertices[j]);
                    edge_vars[i][j] = model->addVar(0.0, 2.0, dist,
                        GRB_INTEGER, "x_" + itos(i) + "_" + itos(j));
                }
            }

            // Degree-2 constraints

            for (int i = 0; i < n; i++) {
                GRBLinExpr expr = 0;
                for (int j = i + 1; j < n; j++) {
                    expr += edge_vars[i][j];
                }
                model->addConstr(expr == 2, "deg2_" + itos(i));
            }

            // Forbid edge from node back to itself

            // for (int i = 0; i < n; i++)
            //     edge_vars[i][i].set(GRB_IntAttr_UB, 0);

            // set callback
            FeasibleSolCallback cb = FeasibleSolCallback(edge_vars, n, source2sink);
            model->setCallback(&cb);
            
            return model;

        }
        catch (GRBException e) {
            cout << "Error number: " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        }
        catch (...) {
            cout << "Error during optimization" << endl;
        }

    }


    void optimize(int n, GRBModel* model, GRBVar** vars, double** sol) {

        try {
            model->optimize();

            if (model->get(GRB_IntAttr_SolCount) > 0) {
                
                for (int i = 0; i < n; i++)
                    sol[i] = model->get(GRB_DoubleAttr_X, vars[i], n);

            }

        }
        catch (GRBException e) {
            cout << "Error number: " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        }
        catch (...) {
            cout << "Error during callback" << endl;
        }
    }

    void solve(int n, vector<int>& source2sink, vector<pair<float, float>>& vertices) {
        GRBVar** edge_vars = NULL;
        edge_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            edge_vars[i] = new GRBVar[n];
        GRBModel* model = init_gurobi_model(n, edge_vars, vertices, source2sink);

        double** sol = new double* [n];
        optimize(n, model, edge_vars, sol);

        print_matrix(n, sol);

        for (int i = 0; i < n; i++)
            delete[] sol[i];
        delete[] sol;
    }


}
