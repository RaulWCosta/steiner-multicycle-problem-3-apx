#pragma once

#include "gurobi_c++.h"

#include <lemon/gomory_hu.h>
#include <lemon/concepts/maps.h>
#include <lemon/list_graph.h>

#include <unordered_set>

#include "src/utils.h"

using namespace std;
using namespace lemon;

typedef ListGraph Graph;
typedef ListGraph::EdgeMap<bool> EdgeBoolMap;

namespace SurvivableNetwork {

    class LPSolver {

    public:

        int _n;
        GRBModel* _model;
        GRBVar* _edge_vars;
        ListGraph* _graph;
        ListGraph::EdgeMap<float>* _cap = nullptr;

        LPSolver(int n, GRBModel* model, GRBVar* edge_vars, ListGraph* graph)
            : _n(n), _model(model), _edge_vars(edge_vars), _graph(graph)
        {
        }


        void solve(ListGraph::EdgeMap<float>* cap) {

            bool valid_relaxed_solution = false;

            while(!valid_relaxed_solution) {
                valid_relaxed_solution = true;

                // update cap inplace
                lp_solver_run(cap);

                GomoryHu<ListGraph, ListGraph::EdgeMap<float>> ght(*_graph, *cap);
                ght.run();
                int half_n = _n >> 1;
                for (int source = 0; source < half_n; source++) {
                    int sink = source + half_n;

                    ListGraph::Node u = _graph->nodeFromId(source);
                    ListGraph::Node v = _graph->nodeFromId(sink);
                    float flow_value = ght.minCutValue(u, v);

                    if (flow_value > 1.999) { // rounding error
                        continue;
                    }

                    GRBLinExpr expr = 0;
                    for(GomoryHu<ListGraph, ListGraph::EdgeMap<float>>::MinCutEdgeIt e(ght, u, v); e != INVALID; ++e) {
                        ListGraph::Edge edge = ListGraph::Edge(e);
                        expr += _edge_vars[_graph->id(edge)];
                    }
                    _model->addConstr(expr >= 2.0);
                    valid_relaxed_solution = false;
                }

            }

        }

    private:

        // run optimize and update cap for maxflow check
        void lp_solver_run(ListGraph::EdgeMap<float>* cap) {

            try {

                _model->optimize();

                double* sol = _model->get(GRB_DoubleAttr_X, _edge_vars, (_n * _n) >> 1);

                for (ListGraph::EdgeIt e(*_graph); e != INVALID; ++e) {
                    (*cap)[e] = sol[_graph->id(e)];
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



    GRBModel* init_gurobi_model(int n, GRBVar* edge_vars, ListGraph& graph, ListGraph::EdgeMap<float>& cost) {

        GRBEnv* env = nullptr;

        try {

            env = new GRBEnv();
            env->set(GRB_IntParam_LogToConsole, 0);
            env->set(GRB_IntParam_Seed, 0);

            GRBModel* model = new GRBModel(*env);

            // Create decision variables, only upper right
            
            for (ListGraph::EdgeIt e(graph); e != INVALID; ++e) {

                int edge_id = graph.id(e);
                float dist = cost[e];

                edge_vars[edge_id] = model->addVar(0.0, 1.0, dist,
                    GRB_CONTINUOUS, "edge_" + itos(edge_id));

            }
            

            // Degree-2 constraints on nodes*
            vector<GRBLinExpr> expr_vec(n, 0);

            for (ListGraph::EdgeIt e(graph); e != INVALID; ++e) {
                int edge_id = graph.id(e);
                int left = graph.id(graph.u(e));
                int right = graph.id(graph.v(e));

                expr_vec[left] += edge_vars[edge_id];
                expr_vec[right] += edge_vars[edge_id];
            }

            for (int i = 0; i < n; i++) {
                model->addConstr(expr_vec[i] >= 2, "deg2_" + itos(i));
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


    void round_up_relaxed_solution(int n, ListGraph& graph, ListGraph::EdgeMap<float>& lp_sol, GRBVar* edge_vars, GRBModel& model, int** curr_sol) {

        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                curr_sol[i][j] = 0;

        for (ListGraph::EdgeIt e(graph); e != INVALID; ++e) {

            int left = graph.id(graph.u(e));
            int right = graph.id(graph.v(e));

            int edge_id = graph.id(e);

            curr_sol[left][right] += (int)(lp_sol[e] + 0.5001); // account for rounding error
            curr_sol[right][left] = curr_sol[left][right];

            if (curr_sol[left][right])
                model.addConstr(edge_vars[edge_id] == 1.0, "add_edge_(" + itos(edge_id) + ")");

        }

        // print_matrix(n, curr_sol);

    }


    void init_graph(int n, ListGraph* graph) {
        for (int i = 0; i < n; i++) {
            graph->addNode();
        }
        
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                ListGraph::Node u = graph->nodeFromId(i);
                ListGraph::Node v = graph->nodeFromId(j);

                graph->addEdge(u, v);
            }
        }

        // add duplicated edges for terminals
        int half_n = n >> 1;
        for (int i = 0; i < half_n; i++) {
            ListGraph::Node u = graph->nodeFromId(i);
            ListGraph::Node v = graph->nodeFromId(i + half_n);
            graph->addEdge(v, u);
        }
    }

    void init_cost_map(int n, ListGraph& graph, float** edges_weights, ListGraph::EdgeMap<float>* cost) {
        
        for (ListGraph::EdgeIt e(graph); e != INVALID; ++e) {

            int left = graph.id(graph.u(e));
            int right = graph.id(graph.v(e));

            (*cost)[e] = edges_weights[left][right];
        }

    }

    // solve 2-apx
    void solve(int n, ListGraph* graph, ListGraph::EdgeMap<float>* cost, int** int_solution) {

        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                int_solution[i][j] = 0;

        bool flag_valid_solution = false;

        // cria modelo e adiciona restrições "base"
        GRBVar* edge_vars = new GRBVar [ (n * n) >> 1 ];
        GRBModel* model = init_gurobi_model(n, edge_vars, *graph, *cost);

        // vector with vertices within an invalid cycle, i.e. there is some vertex in the cycle which
        //  is not connected to it's pair
        ListGraph::EdgeMap<float>* lp_sol = new ListGraph::EdgeMap<float>(*graph);

        LPSolver lp_solver = LPSolver(n, model, edge_vars, graph);

        // // enquanto modelo nao retorna solucao viavel
        while(!flag_valid_solution) {

            // rodar LP até solução viável do relaxado
            lp_solver.solve(lp_sol);

            // adiciona valores >= 0.5 no int_solution
            round_up_relaxed_solution(n, *graph, *lp_sol, edge_vars, *model, int_solution);

            flag_valid_solution = verify_terminals_connected(n, int_solution);

        }

        delete edge_vars;
        delete lp_sol;
        delete model;

    }

}
