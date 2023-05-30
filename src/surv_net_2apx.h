#pragma once

#include "gurobi_c++.h"

#include <lemon/full_graph.h>
// #include <lemon/preflow.h>
#include <lemon/gomory_hu.h>
#include <lemon/concepts/maps.h>
#include <lemon/list_graph.h>

#include <unordered_set>

#include "src/utils.h"

using namespace std;
using namespace lemon;

typedef ListGraph Graph;
typedef FullGraph::NodeMap<bool> NodeBoolMap;

namespace SurvivableNetwork {

    class LPSolver {

    public:

        int _n;
        GRBModel* _model;
        GRBVar** _edge_vars;
        FullGraph* _graph = NULL;
        FullGraph::EdgeMap<float>* _cap = NULL;
        double** _current_solution = nullptr;

        LPSolver(int n, GRBModel* model, GRBVar** edge_vars, FullGraph* graph)
            : _n(n), _model(model), _edge_vars(edge_vars), _graph(graph)
        {
            _cap = new FullGraph::EdgeMap<float>(*_graph);

            _current_solution = new double* [_n];
            for (int i = 0; i < _n; i++)
                _current_solution[i] = new double[_n];
        }

        double** solve() {

            bool valid_relaxed_solution = false;


            while(true) {

                // update _cap inplace
                lp_solver_run();

                NodeBoolMap cutmap(*_graph);
                // run max flow or each terminal pair
                valid_relaxed_solution = is_valid_relaxed_solution(&cutmap);

                if (valid_relaxed_solution) {
                    break;
                }

                update_model_constrains(cutmap);

            }



            for (int i = 0; i < _n; i++) {
                for (int j = 0; j < _n; j++) {
                    if (i == j) {
                        _current_solution[i][j] = 0.0;
                        continue;
                    }
                    FullGraph::Node u = (*_graph)(i);
                    FullGraph::Node v = (*_graph)(j);
                    _current_solution[i][j] = (*_cap)[_graph->edge(u, v)];
                }
            }
            return _current_solution;
        }

    private:

        bool is_valid_relaxed_solution(NodeBoolMap* cutmap) {
            
            GomoryHu<FullGraph, FullGraph::EdgeMap<float>> ght(*_graph, *_cap);
            ght.run();
            int half_n = (int)(_n / 2);
            for (int source = 0; source < half_n; source++) {
                int sink = source + half_n;

                FullGraph::Node u = (*_graph)(source);
                FullGraph::Node v = (*_graph)(sink);
                float flow_value = ght.minCutValue(u, v);
                ght.minCutMap(u, v, *cutmap);

                if (flow_value < 1.999) { // rounding error
                    return false;
                }
            }
            return true;

        }

        void update_model_constrains(NodeBoolMap& cutmap) {

            vector<int> s_values;
            vector<int> not_s_values;

            for (int i = 0; i < _n; i++) {
                FullGraph::Node u = (*_graph)(i);
                if (cutmap[u]) {
                    s_values.push_back(i);
                } else {
                    not_s_values.push_back(i);
                }
            }

            GRBLinExpr expr = 0;
            for (int& i : s_values) {
                for (int& j : not_s_values) {
                    if (i < j)
                        expr += _edge_vars[i][j];
                    else
                        expr += _edge_vars[j][i];
                }
            }
            _model->addConstr(expr >= 2.0);

        }

        // run optimize and update cap for maxflow check
        void lp_solver_run() {

            try {

                _model->optimize();

                // Extract solution
                for (int i = 0; i < _n; i++) {
                    double* sol = _model->get(GRB_DoubleAttr_X, _edge_vars[i], _n);

                    for (int j = i + 1; j < _n; j++) {
                        FullGraph::Node u = (*_graph)(i);
                        FullGraph::Node v = (*_graph)(j);
                        (*_cap)[_graph->edge(u, v)] = sol[j];
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



    GRBModel* init_gurobi_model(int n, GRBVar** edge_vars, FullGraph& graph, FullGraph::EdgeMap<float>& cost) {

        GRBEnv* env = NULL;

        try {

            env = new GRBEnv();
            GRBModel* model = new GRBModel(*env);

            // Create decision variables, only upper right
            for (int i = 0; i < n; i++) {
                for (int j = i; j < n; j++) {
                    FullGraph::Node u = graph(i);
                    FullGraph::Node v = graph(j);
                    float dist = 0.0f;
                    if (i != j)
                        dist = cost[graph.edge(u, v)];
                    edge_vars[i][j] = model->addVar(0.0, 1.0, dist,
                        GRB_CONTINUOUS, "x_" + itos(i) + "_" + itos(j));
                    edge_vars[j][i] = edge_vars[i][j];
                }
            }

            for (int i = 0; i < n; i++)
                edge_vars[i][i].set(GRB_DoubleAttr_UB, 0);

            // Degree-2 constraints

            for (int i = 0; i < n; i++) {
                GRBLinExpr expr = 0;
                for (int j = 0; j < n; j++) {
                    expr += edge_vars[i][j];
                }
                model->addConstr(expr >= 2, "deg2_" + itos(i));
            }

            return model;

        }
        catch (GRBException e) {
            cout << "Error number: " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        }
        catch (...) {
            cout << "Error during optimization" << endl;
        }
        exit(1);
    }


    void update_int_solution(int n, double** lp_solution, int** current_solution, GRBVar** edge_vars, GRBModel& model) {

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int old_value = current_solution[i][j];
                current_solution[i][j] = (int)(lp_solution[i][j] + 0.5001); // rounding error
                if (!old_value && current_solution[i][j]) {
                    // trava o valor dessa aresta como 1
                    model.addConstr(edge_vars[i][j] == 1.0, "add_edge_(" + itos(i) + "," + itos(j) + ")");
                }
            }
        }

    }


    bool is_valid_int_solution(int n, int** current_solution) {

        vector<int> stack;
        int half_n = (int)(n / 2);
        for (int s = 0; s < half_n; s++) {
            int t = s + half_n;

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
    void solve(int n, FullGraph& graph, FullGraph::EdgeMap<float>& cost, int** int_solution) {

        bool flag_valid_solution = false;

        // cria modelo e adiciona restrições "base"
        GRBVar** edge_vars = NULL;
        edge_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            edge_vars[i] = new GRBVar[n];
        GRBModel* model = init_gurobi_model(n, edge_vars, graph, cost);

        // vector with vertices within an invalid cycle, i.e. there is some vertex in the cycle which
        //  is not connected to it's pair

        LPSolver lp_solver = LPSolver(n, model, edge_vars, &graph);

        // enquanto modelo nao retorna solucao viavel
        while(!flag_valid_solution) {

            // rodar LP até solução viável do relaxado
            double** lp_solution = lp_solver.solve();

            // print_matrix(n, lp_solution);
            // print_matrix(n, int_solution);

            // adiciona valores >= 0.5 no int_solution
            update_int_solution(n, lp_solution, int_solution, edge_vars, *model);

            print_matrix(n, int_solution);

            flag_valid_solution = is_valid_int_solution(n, int_solution);

        }

    }

}
