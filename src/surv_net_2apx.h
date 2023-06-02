#pragma once

#include "gurobi_c++.h"

// #include <lemon/full_graph.h>
// #include <lemon/preflow.h>
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

    // TODO delete this class
    class LPSolver {

    public:

        int _n;
        GRBModel* _model;
        GRBVar* _edge_vars;
        ListGraph* _graph;
        ListGraph::EdgeMap<float>* _cap = nullptr;

        LPSolver(int n, GRBModel* model, GRBVar* edge_vars, ListGraph* graph, ListGraph::EdgeMap<float>* cap)
            : _n(n), _model(model), _edge_vars(edge_vars), _graph(graph), _cap(cap)
        {
        }


        ListGraph::EdgeMap<float>* solve() {

            bool valid_relaxed_solution = false;


            while(!valid_relaxed_solution) {
                valid_relaxed_solution = true;

                // update _cap inplace
                lp_solver_run();

                // EdgeBoolMap cutmap(*_graph);

                GomoryHu<ListGraph, ListGraph::EdgeMap<float>> ght(*_graph, *_cap);
                ght.run();
                int half_n = (int)(_n / 2);
                for (int source = 0; source < half_n; source++) {
                    int sink = source + half_n;

                    ListGraph::Node u = _graph->nodeFromId(source);
                    ListGraph::Node v = _graph->nodeFromId(sink);
                    float flow_value = ght.minCutValue(u, v);
                    // ght.minCutMap(u, v, cutmap);

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

            return _cap;
        }

    private:

        // bool is_valid_relaxed_solution(EdgeBoolMap* cutmap) {
            


        // }

        // void update_model_constrains(EdgeBoolMap& cutmap) {

        //     vector<int> s_values;
        //     vector<int> not_s_values;

        //     for (int i = 0; i < _n; i++) {
        //         ListGraph::Node u = _graph->nodeFromId(i);
        //         if (cutmap[u]) {
        //             s_values.push_back(i);
        //         } else {
        //             not_s_values.push_back(i);
        //         }
        //     }

        //     GRBLinExpr expr = 0;
        //     for (int& i : s_values) {
        //         for (int& j : not_s_values) {
                    
        //             ListGraph::Node a = _graph->nodeFromId(i);
        //             ListGraph::Node b = _graph->nodeFromId(j);

        //             ListGraph::Edge e1 = _graph->edge(a, b);
        //             ListGraph::Edge e2 = _graph->edge(b, a);

        //             if (e1 != INVALID)
        //                 expr += _edge_vars[_graph->id(e1)];
        //             if (e2 != INVALID)
        //                 expr += _edge_vars[_graph->id(e2)];
        //         }
        //     }
        //     _model->addConstr(expr >= 2.0);

        // }

        // run optimize and update cap for maxflow check
        void lp_solver_run() {

            try {

                _model->optimize();

                double* sol = _model->get(GRB_DoubleAttr_X, _edge_vars, (int)(_n*_n/2));

                for (ListGraph::EdgeIt e(*_graph); e != INVALID; ++e) {
                    (*_cap)[e] = sol[_graph->id(e)];
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

    }


    bool is_valid_int_solution(int n, int** sol) {

    vector<int> vertices_degree = get_vertices_degrees(n, sol);

    for (int i = 0; i < vertices_degree.size(); i++) {
        if (vertices_degree[i] % 2) {
            return false;
        }
    }

    // check if terminals are connected
    return verify_terminals_connected(n, sol);
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
        int half_n = (int)(n / 2);
        for (int i = 0; i < half_n; i++) {
            ListGraph::Node u = graph->nodeFromId(i);
            ListGraph::Node v = graph->nodeFromId(i + half_n);
            graph->addEdge(v, u);
        }
    }

    void init_cost_map(int n, ListGraph& graph, vector<pair<float, float>>& vertices, ListGraph::EdgeMap<float>* cost) {
        
        for (ListGraph::EdgeIt e(graph); e != INVALID; ++e) {

            int left = graph.id(graph.u(e));
            int right = graph.id(graph.v(e));

            float dist = vertices_distance(vertices[left], vertices[right]);
            (*cost)[e] = dist;

        }

    }

    // solve 2-apx
    void solve(int n, vector<pair<float, float>>& vertices, int** int_solution) {

        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                int_solution[i][j] = 0;

        ListGraph* graph = new ListGraph();
        graph->reserveNode(n);
        graph->reserveEdge((int)(n*n/2));

        init_graph(n, graph);
        ListGraph::EdgeMap<float>* cost = new ListGraph::EdgeMap<float>(*graph);
        init_cost_map(n, *graph, vertices, cost);

        bool flag_valid_solution = false;

        // cria modelo e adiciona restrições "base"
        GRBVar* edge_vars = new GRBVar [ (int)(n*n/2) ];
        GRBModel* model = init_gurobi_model(n, edge_vars, *graph, *cost);

        // vector with vertices within an invalid cycle, i.e. there is some vertex in the cycle which
        //  is not connected to it's pair
        ListGraph::EdgeMap<float>* lp_sol = new ListGraph::EdgeMap<float>(*graph);

        LPSolver lp_solver = LPSolver(n, model, edge_vars, graph, lp_sol);

        // // enquanto modelo nao retorna solucao viavel
        while(!flag_valid_solution) {

            // rodar LP até solução viável do relaxado
            lp_sol = lp_solver.solve();

            // adiciona valores >= 0.5 no int_solution
            round_up_relaxed_solution(n, *graph, *lp_sol, edge_vars, *model, int_solution);

            flag_valid_solution = is_valid_int_solution(n, int_solution);

        }

    }

}
