
#include "2apx_survival_net.h"

using namespace std;


// void findsubtour(int n, double** sol, int* tourlenP, int* tour);

// Subtour elimination callback.  Whenever a feasible solution is found,
// find the smallest subtour, and add a subtour elimination constraint
// if the tour doesn't visit every node.

// class subtourelim : public GRBCallback
// {
// public:
//     GRBVar** vars;
//     int n;
    
//     subtourelim(GRBVar** xvars, int xn) {
//         vars = xvars;
//         n = xn;
//     }
// protected:
//     void callback() {
//         try {
//             if (where == GRB_CB_MIPSOL) {
//                 // Found an integer feasible solution - does it visit every node?
//                 double** x = new double* [n];
//                 int* tour = new int[n];
//                 int i, j, len;
//                 for (i = 0; i < n; i++)
//                     x[i] = getSolution(vars[i], n);

//                 findsubtour(n, x, &len, tour);

//                 if (len < n) {
//                     // Add subtour elimination constraint
//                     GRBLinExpr expr = 0;
//                     for (i = 0; i < len; i++)
//                         for (j = i + 1; j < len; j++)
//                             expr += vars[tour[i]][tour[j]];
//                     addLazy(expr <= len - 1);
//                 }

//                 for (i = 0; i < n; i++)
//                     delete[] x[i];
//                 delete[] x;
//                 delete[] tour;
//             }
//         }
//         catch (GRBException e) {
//             cout << "Error number: " << e.getErrorCode() << endl;
//             cout << e.getMessage() << endl;
//         }
//         catch (...) {
//             cout << "Error during callback" << endl;
//         }
//     }
// };

// // Given an integer-feasible solution 'sol', find the smallest
// // sub-tour.  Result is returned in 'tour', and length is
// // returned in 'tourlenP'.

// void findsubtour(int      n,
//     double** sol,
//     int* tourlenP,
//     int* tour)
// {
//     bool* seen = new bool[n];
//     int bestind, bestlen;
//     int i, node, len, start;

//     for (i = 0; i < n; i++)
//         seen[i] = false;

//     start = 0;
//     bestlen = n + 1;
//     bestind = -1;
//     node = 0;
//     while (start < n) {
//         for (node = 0; node < n; node++)
//             if (!seen[node])
//                 break;
//         if (node == n)
//             break;
//         for (len = 0; len < n; len++) {
//             tour[start + len] = node;
//             seen[node] = true;
//             for (i = 0; i < n; i++) {
//                 if (sol[node][i] > 0.5 && !seen[i]) {
//                     node = i;
//                     break;
//                 }
//             }
//             if (i == n) {
//                 len++;
//                 if (len < bestlen) {
//                     bestlen = len;
//                     bestind = start;
//                 }
//                 start += len;
//                 break;
//             }
//         }
//     }

//     for (i = 0; i < bestlen; i++)
//         tour[i] = tour[bestind + i];
//     *tourlenP = bestlen;

//     delete[] seen;
// }

// Euclidean distance between points 'i' and 'j'.

double distance(double* x,
    double* y,
    int     i,
    int     j)
{
    double dx = x[i] - x[j];
    double dy = y[i] - y[j];

    return sqrt(dx * dx + dy * dy);
}

int solveLP(int n)
{
    double* x = new double[n];
    double* y = new double[n];

    int i;
    for (i = 0; i < n; i++) {
        x[i] = ((double)rand()) / RAND_MAX;
        y[i] = ((double)rand()) / RAND_MAX;
    }

    GRBEnv* env = NULL;
    GRBVar** edge_vars = NULL;

    edge_vars = new GRBVar * [n];
    for (i = 0; i < n; i++)
        edge_vars[i] = new GRBVar[n];

    try {
        int j;

        env = new GRBEnv();
        GRBModel model = GRBModel(*env);

        // Must set LazyConstraints parameter when using lazy constraints

        // model.set(GRB_IntParam_LazyConstraints, 1);

        // Create binary decision variables

        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                edge_vars[i][j] = model.addVar(0.0, 1.0, distance(x, y, i, j),
                    GRB_CONTINUOUS, "x_" + itos(i) + "_" + itos(j));
                edge_vars[j][i] = model.addVar(0.0, 1.0, distance(x, y, i, j),
                    GRB_CONTINUOUS, "x_" + itos(j) + "_" + itos(i));
            }
        }

        // Degree-2 constraints

        for (i = 0; i < n; i++) {
            GRBLinExpr expr_in = 0;
            GRBLinExpr expr_out = 0;
            for (j = 0; j < n; j++) {
                expr_in += edge_vars[j][i];
                expr_out += edge_vars[i][j];
            }
            model.addConstr(expr_in == 2, "deg2_" + itos(i)); // update to >= 1 later
            model.addConstr(expr_out == 2, "deg2_" + itos(i));
        }

        // Forbid edge from node back to itself

        for (i = 0; i < n; i++)
            edge_vars[i][i].set(GRB_DoubleAttr_UB, 0);

        // Set callback function

        subtourelim cb = subtourelim(edge_vars, n);
        model.setCallback(&cb);

        // Optimize model

        model.optimize();

        // Extract solution

        if (model.get(GRB_IntAttr_SolCount) > 0) {
            double** sol = new double* [n];
            for (i = 0; i < n; i++)
                sol[i] = model.get(GRB_DoubleAttr_X, edge_vars[i], n);

            int* tour = new int[n];
            int len;

            findsubtour(n, sol, &len, tour);
            assert(len == n);

            cout << "Tour: ";
            for (i = 0; i < len; i++)
                cout << tour[i] << " ";
            cout << endl;

            for (i = 0; i < n; i++)
                delete[] sol[i];
            delete[] sol;
            delete[] tour;
        }

    }
    catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    catch (...) {
        cout << "Error during optimization" << endl;
    }

    for (i = 0; i < n; i++)
        delete[] edge_vars[i];
    delete[] edge_vars;
    delete[] x;
    delete[] y;
    delete env;
    return 0;
}





// bool is_valid_solution(bool** solution, std::vector<int> source2sink = { 2, 3 }, vector<int>* current_cycle) {

//     for (int source; source < source2sink.size(); source++) {
//         int sink = source2sink[source];

//         int current = source;
//         bool sink_found = false;    

//         // vector<int> current_cycle;

//         do {
//             // look for sink
//             // ao longo do processo, adiciona quais vertices fazem parte do ciclo em um vetor
//             // if sink found, flag = True

//         } while(current != source);

//         if (!sink_found) {
//             return false;
//         }

//     }
//     return true;
// }

// int solve(int n) {

//     bool** current_solution = new bool* [n];
//     for (int i = 0; i < n; i++) {
//         current_solution[i] = new bool[n];
//     }

//     vector<int>* current_cycle = NULL;

//     // atualizar current solution com solução da LP


//     do {



//     } while(is_valid_solution(current_solution));

// }
