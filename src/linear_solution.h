#pragma once

#include "gurobi_c++.h"

#include <lemon/full_graph.h>
#include <lemon/gomory_hu.h>
#include <unordered_set>

#include "src/utils.h"

using namespace std;
using namespace lemon;

typedef FullGraph::NodeMap<bool> NodeBoolMap;

namespace LinearSMCP {


    class FeasibleSolCallback : public GRBCallback
    {
    public:
        GRBVar** _edge_vars;
        int _n;
        FullGraph* _graph = NULL;
        FullGraph::EdgeMap<double>* _cap = NULL;

        FeasibleSolCallback(int n, GRBVar** vars, FullGraph* graph)
            : _edge_vars(vars), _n(n), _graph(graph)
        {
            _cap = new FullGraph::EdgeMap<double>(*_graph);
        }

        ~FeasibleSolCallback() {
            delete _cap;
        }

    protected:
        void callback() {
            try {
                if (where == GRB_CB_MIPSOL) {

                    NodeBoolMap cutmap(*_graph);

                    double* tmp_solution;
                    for (int i = 0; i < _n; i++) {
                        tmp_solution = getSolution(_edge_vars[i], _n);
                        for (int j = i + 1; j < _n; j++) {
                            FullGraph::Node u = (*_graph)(i);
                            FullGraph::Node v = (*_graph)(j);
                            (*_cap)[_graph->edge(u, v)] = tmp_solution[j];
                        }
                    }

                    if (!is_valid_solution(&cutmap)) {
                        // add restrictions
                        update_model_constrains(cutmap);
                    }

                    // delete[] tmp_solution;
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

        bool is_valid_solution(NodeBoolMap* cutmap) {
            
            GomoryHu<FullGraph, FullGraph::EdgeMap<double>> ght(*_graph, *_cap);
            ght.run();
            int half_n = _n >> 1;
            for (int source = 0; source < half_n; source++) {
                int sink = source + half_n;

                FullGraph::Node u = (*_graph)(source);
                FullGraph::Node v = (*_graph)(sink);
                double flow_value = ght.minCutValue(u, v);
                ght.minCutMap(u, v, *cutmap);

                if (flow_value < 2.0) {
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
            addLazy(expr >= 2.0);

        }

    };

    double** solve(int n, float** edges_weights, double** sol) {

        FullGraph* graph = new FullGraph(n);
        FullGraph::EdgeMap<float>* cost = new FullGraph::EdgeMap<float>(*graph);

        GRBEnv* env = new GRBEnv();
        GRBVar** edge_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            edge_vars[i] = new GRBVar[n];


        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                sol[i][j] = 0.0f;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                FullGraph::Node u = (*graph)(i);
                FullGraph::Node v = (*graph)(j);

                if (i == j) {
                    continue;
                }
                (*cost)[graph->edge(u, v)] = edges_weights[i][j];
            }
        }

        env->set(GRB_IntParam_LogToConsole, 0);
        env->set(GRB_IntParam_Seed, 0);

        GRBModel model = GRBModel(*env);

        // Must set LazyConstraints parameter when using lazy constraints
        model.set(GRB_IntParam_LazyConstraints, 1);

        // Create decision variables

        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                edge_vars[i][j] = model.addVar(0.0, 2.0, edges_weights[i][j],
                    GRB_CONTINUOUS, "x_" + itos(i) + "_" + itos(j));
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
            model.addConstr(expr_vec[i] == 2.0, "deg2_" + itos(i));

        // set callback
        FeasibleSolCallback cb = FeasibleSolCallback(n, edge_vars, graph);
        model.setCallback(&cb);

        double** tmp_sol = new double* [n];

        model.optimize();

        if (model.get(GRB_IntAttr_SolCount) > 0) {
            for (int i = 0; i < n; i++)
                tmp_sol[i] = model.get(GRB_DoubleAttr_X, edge_vars[i], n);
        }

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                sol[i][j] = tmp_sol[i][j];

        // print_matrix(n, int_sol);

        for (int i = 0; i < n; i++)
            delete[] tmp_sol[i];
        delete[] tmp_sol;

        for (int i = 0; i < n; i++)
            delete[] edge_vars[i];
        delete[] edge_vars;

        delete env;
        delete graph;
        delete cost;

        return sol;
    }


}
