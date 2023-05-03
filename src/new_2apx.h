#pragma once

#include "gurobi_c++.h"

#include <lemon/full_graph.h>
#include <lemon/preflow.h>

#include <unordered_set>

#include "src/utils.h"

using namespace std;
using namespace lemon;

namespace SurvivableNetwork {

    class LPSolver {

    public:

        int _n;
        GRBModel* _model;
        GRBVar** _edge_vars;
        vector<int> _source2sink;
        FullDigraph* _graph = NULL;
        FullDigraph::ArcMap<float>* _cap = NULL;

        LPSolver(int n, GRBModel* model, GRBVar** edge_vars, vector<int>& source2sink)
            : _n(n), _model(model), _edge_vars(edge_vars), _source2sink(source2sink)
        {
            _graph = new FullDigraph(n);
            _cap = new FullDigraph::ArcMap<float>(*_graph);

        }

        double** solve() {

            bool valid_relaxed_solution = false;
            unordered_set<int> s_set;

            while(true) {

                // update _cap inplace
                lp_solver_run();

                // run max flow or each terminal pair
                valid_relaxed_solution = is_valid_relaxed_solution(&s_set);

                if (valid_relaxed_solution) {
                    break;
                }

                update_model_constrains(s_set);
                s_set.clear();

            }

            double** sol = new double* [_n];
            for (int i = 0; i < _n; i++)
                sol[i] = new double[_n];

            for (int i = 0; i < _n; i++) {
                for (int j = 0; j < _n; j++) {
                    FullDigraph::Node u = (*_graph)(i);
                    FullDigraph::Node v = (*_graph)(j);
                    sol[i][j] = (*_cap)[_graph->arc(u, v)];
                }
            }
            return sol;

        }

    private:

        bool is_valid_relaxed_solution(unordered_set<int>* s_set) {

            for (int source = 0; source < _source2sink.size(); source++) {
                int sink = _source2sink[source];

                FullDigraph::Node u = (*_graph)(source);
                FullDigraph::Node v = (*_graph)(sink);

                Preflow< FullDigraph, FullDigraph::ArcMap<float> > preflow(*_graph, *_cap, u, v);
                preflow.run();
                float flow_value = preflow.flowValue();
                if (flow_value < 1.0) {
                    for(int i = 0; i < _n; i++) {
                        FullDigraphBase::Node node = (*_graph)(i);
                        if (preflow.minCut(node)) {
                            s_set->insert(i);
                        }
                    }
                    return false;
                }

            }
            return true;

        }

        void update_model_constrains(unordered_set<int>& s_set) {

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

            GRBLinExpr expr_out = 0;
            GRBLinExpr expr_in = 0;
            for (int& i : s_values) {
                for (int& j : not_s_values) {
                    expr_out += _edge_vars[i][j];
                    expr_in += _edge_vars[j][i];
                }
            }
            _model->addConstr(expr_out >= 1.0);
            _model->addConstr(expr_in >= 1.0);

        }

        // run optimize and update cap for maxflow check
        void lp_solver_run() {

            try {

                _model->optimize();

                // Extract solution
                for (int i = 0; i < _n; i++) {
                    double* sol = _model->get(GRB_DoubleAttr_X, _edge_vars[i], _n);

                    for (int j = 0; j < _n; j++) {
                        FullDigraph::Node u = (*_graph)(i);
                        FullDigraph::Node v = (*_graph)(j);
                        (*_cap)[_graph->arc(u, v)] = sol[j];
                    }

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

    };



    GRBModel* init_gurobi_model(int n, GRBVar** edge_vars, vector<pair<float, float>>& vertices) {

        GRBEnv* env = NULL;

        try {

            env = new GRBEnv();
            GRBModel* model = new GRBModel(*env);

            // Create decision variables
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    float dist = vertices_distance(vertices[i], vertices[j]);
                    edge_vars[i][j] = model->addVar(0.0, 1.0, dist,
                        GRB_CONTINUOUS, "x_" + itos(i) + "_" + itos(j));
                }
            }

            // Degree-2 constraints

            for (int i = 0; i < n; i++) {
                GRBLinExpr expr_in = 0;
                GRBLinExpr expr_out = 0;
                for (int j = 0; j < n; j++) {
                    expr_in += edge_vars[j][i];
                    expr_out += edge_vars[i][j];
                }
                model->addConstr(expr_in >= 1, "deg2_" + itos(i)); // update to >= 1 later
                model->addConstr(expr_out >= 1, "deg2_" + itos(i));
            }

            // Forbid edge from node back to itself

            for (int i = 0; i < n; i++)
                edge_vars[i][i].set(GRB_DoubleAttr_UB, 0);

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


    void update_int_solution(int n, double** lp_solution, int** current_solution, GRBVar** edge_vars, GRBModel& model) {

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int old_value = current_solution[i][j];
                current_solution[i][j] = (int)(lp_solution[i][j] + 0.5);
                if (!old_value && current_solution[i][j]) {
                    // trava o valor dessa aresta como 1
                    model.addConstr(edge_vars[i][j] == 1.0, "add_edge_(" + itos(i) + "," + itos(j) + ")");
                }
            }
        }

    }


    bool is_valid_int_solution(int n, int** current_solution, vector<int>& source2sink) {

        vector<int> stack;

        for (int s = 0; s < source2sink.size(); s++) {
            int t = source2sink[s];

            bool flag_valid_cycle = false;
            vector<bool> visited(n, false);

            stack.clear();

            visited[s] = true;
            stack.push_back(s);

            while (!stack.empty()) {
                int curr_node = stack[stack.size() - 1];
                stack.pop_back();

                if (curr_node == t) {
                    flag_valid_cycle = true;
                    break;
                }

                for (int i = 0; i < n; i++) {
                    if (!current_solution[curr_node][i])
                        continue;

                    if (!visited[i]) {
                        visited[i] = true;
                        stack.push_back(i);
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


    // solve 2-apx
    int** solve(int n, vector<int>& source2sink, vector<pair<float, float>>& vertices) {

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
