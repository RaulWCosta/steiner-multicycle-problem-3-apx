#include "gurobi_c++.h"

class ExactSolver {

public:
    GRBVar** _edges_vars = nullptr;
    GRBLinExpr* _degree_vars = nullptr;
    int _n;

	ExactSolver(GRBModel& model, int n, const std::vector<int>& source_to_sink, const std::vector<Vertex>& vertices) {
        _n = n;

        _edges_vars = new GRBVar * [n];
        for (int i = 0; i < n; i++)
            _edges_vars[i] = new GRBVar[n];

        _degree_vars = new GRBLinExpr [n];

        model.set(GRB_IntParam_LazyConstraints, 1);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                _edges_vars[i][j] = model.addVar(0, 2, distance(vertices[i], vertices[j]),
                    GRB_INTEGER, "edge_" + std::to_string(i) + "_" + std::to_string(j));
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

    ~ExactSolver() {
        
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
