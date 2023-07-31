
#include <vector>
#include <utility>
#include <fstream>
#include <string>
#include <chrono>

#include "src/utils.h"
#include "src/smcp_3apx.h"
#include "src/exact_solution.h"

using namespace std;


int main(int argc, char* argv[]) {

    vector<string>* files = new vector<string>;
    string dir = "../../../testInst";
    findDataFiles(dir, files);

    std::ofstream result_file("../../../_result.csv", std::ios::app);
    if (result_file.is_open()) {
        result_file << "instance;exact_val;exact_time;apx_val;apx_time;num_vertices" << endl;
        result_file.close();
    } else {
        std::cerr << "Error: Unable to save result." << std::endl;
        exit(1);
    }

    for (string& file : *files) {

        // char* str = "../../../testInst/m10Q10s555.tsp.ccpdp";
        // string new_file = str;

        int n = 0;
        vector<pair<float, float>> vertices;

        read_instance(file, &n, &vertices);

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

        auto start_time = std::chrono::high_resolution_clock::now();
        //sol = ExactSMCP::solve(n, edges_weights, sol);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto exact_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        //verify_solution(file, n, sol);
        float exact = 0.0;//get_sol_val(n, sol, edges_weights);

        // print_matrix(n, sol);

        start_time = std::chrono::high_resolution_clock::now();
        sol = ApxSMCP::solve(n, edges_weights, sol);
        end_time = std::chrono::high_resolution_clock::now();
        auto apx_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        verify_solution(file, n, sol);
        float apx = get_sol_val(n, sol, edges_weights);

        std::ofstream result_file("../../../_result.csv", std::ios::app);
        if (result_file.is_open()) {
            result_file << file << ";" << exact << ";" << exact_time << ";" << apx << ";" << apx_time << ";" << n << endl;
            result_file.close();
        } else {
            std::cerr << "Error: Unable to save result." << std::endl;
            exit(1);
        }

        for (int i = 0; i < n; i++)
            delete[] sol[i];
        delete[] sol;

        for (int i = 0; i < n; i++)
            delete[] edges_weights[i];
        delete[] edges_weights;
    }

    delete files;

}
