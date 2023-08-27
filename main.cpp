
#include <vector>
#include <utility>
#include <fstream>
#include <string>
#include <chrono>

#include "src/utils.h"
#include "src/smcp_3apx.h"
// #include "src/exact_solution.h"
#include "src/linear_solution.h"

using namespace std;


int main(int argc, char* argv[]) {

    vector<string>* files = new vector<string>;
    string dir = "../../../testInst";
    findDataFiles(dir, files);

    std::ofstream result_file("../../../_result.csv", std::ios::app);
    if (result_file.is_open()) {
        result_file << "instance;relaxed_val;relaxed_time;apx_val;apx_time;num_vertices" << endl;
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
        double** double_sol = new double* [n];
        for (int i = 0; i < n; i++)
            double_sol[i] = new double[n];

        auto start_time = std::chrono::high_resolution_clock::now();
        //sol = ExactSMCP::solve(n, edges_weights, sol);
        double_sol = LinearSMCP::solve(n, edges_weights, double_sol);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto relaxed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        //verify_solution(file, n, sol);
        // float exact = 0.0;//get_sol_val(n, sol, edges_weights);
        double relaxed = get_sol_val(n, double_sol, edges_weights);

        // print_matrix(n, sol);

        // allocate memory to solution
        int** int_sol = new int* [n];
        for (int i = 0; i < n; i++)
            int_sol[i] = new int[n];

        start_time = std::chrono::high_resolution_clock::now();
        int_sol = ApxSMCP::solve(n, edges_weights, int_sol);
        end_time = std::chrono::high_resolution_clock::now();
        auto apx_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        verify_solution(file, n, int_sol);
        double apx = get_sol_val(n, int_sol, edges_weights);

        std::ofstream result_file("../../../_result.csv", std::ios::app);
        if (result_file.is_open()) {
            result_file << file << ";" << relaxed << ";" << relaxed_time << ";" << apx << ";" << apx_time << ";" << n << endl;
            result_file.close();
        } else {
            std::cerr << "Error: Unable to save result." << std::endl;
            exit(1);
        }

        for (int i = 0; i < n; i++)
            delete[] int_sol[i];
        delete[] int_sol;

        for (int i = 0; i < n; i++)
            delete[] double_sol[i];
        delete[] double_sol;

        for (int i = 0; i < n; i++)
            delete[] edges_weights[i];
        delete[] edges_weights;
    }

    delete files;

}
