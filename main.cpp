
#include <vector>
#include <utility>
#include <fstream>
#include <string>

#include "src/utils.h"
#include "src/smcp_3apx.h"
#include "src/exact_solution.h"

using namespace std;


int main(int argc, char* argv[]) {

    vector<string>* files = new vector<string>;
    string dir = "../../../allInst";
    findDataFiles(dir, files);


    for (string& file : *files) {

        char* str = "../../../allInst/m10Q10s555.tsp.ccpdp";
        string new_file = str;

        int n = 0;
        vector<pair<float, float>> vertices;
        vertices.reserve(n);

        read_instance(new_file, &n, &vertices);

        // init edges_weights matrix
        float** edges_weights = new float* [n];
        for (int i = 0; i < n; i++)
            edges_weights[i] = new float[n];
        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                edges_weights[i][j] = vertices_distance(vertices[i], vertices[j]);
        
        // allocate memory to solution
        int** sol = new int* [n];
        for (int i = 0; i < n; i++)
            sol[i] = new int[n];


        sol = ExactSMCP::solve(n, edges_weights, sol);
        verify_solution(file, n, sol);
        float exact = get_sol_val(n, sol, edges_weights);
        print_matrix(n, sol);

        sol = ApxSMCP::solve(n, edges_weights, sol);
        verify_solution(file, n, sol);
        cout << "3apx = " << get_sol_val(n, sol, edges_weights) << endl;
        cout << "exact = " << exact << endl;
        cout << "file = " << file << endl;

        for (int i = 0; i < n; i++)
            delete[] sol[i];
        delete[] sol;
        return 0;
    }

    delete files;

}

// #include "src/tsp.h"

// using namespace std;

// int main(int argc, char* argv[]) {
//     solve();
//     return 0;
// }