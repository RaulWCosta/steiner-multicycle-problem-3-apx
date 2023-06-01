
#include <vector>
#include <utility>
#include <fstream>
#include <string>

#include "src/utils.h"
#include "src/surv_net_2apx.h"
#include "src/smcp_3apx.h"
#include "src/exact_solution.h"

using namespace std;


int main(int argc, char* argv[]) {

    vector<string>* files = new vector<string>;
    string dir = "../../../allInst";
    findDataFiles(dir, files);


    for (string& file : *files) {

        // char* str = "../../../allInst/m10Q10s555.tsp.ccpdp";
        // string new_file = str;

        int n = 0;
        vector<pair<float, float>> vertices;
        vertices.reserve(n);

        read_instance(file, &n, &vertices);

        // int n = 4;

        // vector<pair<float, float>> vertices = {
        //     pair<float, float>(0.0, 0.0),
        //     pair<float, float>(1.0, 0.0),
        //     pair<float, float>(1.0, 1.0),
        //     pair<float, float>(0.0, 1.0),
        // };

        // F
        int** sol = new int* [n];
        for (int i = 0; i < n; i++)
            sol[i] = new int[n];

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                sol[i][j] = 0;

        FullGraph* graph = new FullGraph(n);
        FullGraph::EdgeMap<float>* cost = new FullGraph::EdgeMap<float>(*graph);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                FullGraph::Node u = (*graph)(i);
                FullGraph::Node v = (*graph)(j);
                float dist;
                if (i == j) {
                    continue;
                }
                dist = vertices_distance(vertices[i], vertices[j]);
                (*cost)[graph->edge(u, v)] = dist;
            }
        }

        ExactSMCP::solve(n, vertices, *graph, sol);
        float exact = get_sol_val(n, sol, *graph, *cost);

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                sol[i][j] = 0;

        SurvivableNetwork::solve(n, *graph, *cost, sol);

        cout << "file = " << file << endl;

        sol = ApxSMCP::solve(n, sol, *graph, *cost);

        cout << "exact = " << exact << endl;

        // print_matrix(n, sol);

        verify_solution(file, n, sol);

        delete graph;
        delete cost;
        for (int i = 0; i < n; i++)
            delete[] sol[i];
        delete[] sol;

    }

    delete files;

}

// #include "src/tsp.h"

// using namespace std;

// int main(int argc, char* argv[]) {
//     solve();
//     return 0;
// }