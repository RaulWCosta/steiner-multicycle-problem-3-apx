
#include <vector>
#include <utility>
#include <fstream>
#include <string>
#include <chrono>

#include "src/utils.h"
#include "src/smcp_3apx.h"
#include "src/linear_solution.h"

int MAX_INSTANCE_SIZE = 300;

struct Instance {
    string instance_file;
    int num_vertices;
};

struct Result {
    double value;
    long long int execution_time;
};

struct ExecutionTracker {
    struct Instance instance;
    struct Result *linear_relaxation;
    struct Result *approximation;
    // struct Result survival_network;
    // struct Result approximation_perfect_matching;
    // struct Result approximation_shortcutting;
};

Result execute_linear_relaxation(int n, float** edges_weights, double** double_sol) {

    // allocate memory
    FullGraph* graph = new FullGraph(n);
    FullGraph::EdgeMap<float>* cost = new FullGraph::EdgeMap<float>(*graph);

    GRBEnv* env = new GRBEnv();
    GRBVar** edge_vars = new GRBVar * [n];
    for (int i = 0; i < n; i++)
        edge_vars[i] = new GRBVar[n];

    auto start_time = std::chrono::high_resolution_clock::now();
    double_sol = LinearSMCP::solve(
        n,
        edges_weights,
        graph,
        cost,
        env,
        edge_vars,
        double_sol
    );
    auto end_time = std::chrono::high_resolution_clock::now();
    auto relaxed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    double relaxed = get_sol_val(n, double_sol, edges_weights);

    for (int i = 0; i < n; i++)
        delete[] edge_vars[i];
    delete[] edge_vars;

    delete env;
    delete graph;
    delete cost;

    struct Result result = { relaxed, (long long int)relaxed_time };
    return result;
}

Result execute_approximation(int n, float** edges_weights, int** int_sol) {

    // allocate memory
    ListGraph* graph = new ListGraph();
    graph->reserveNode(n);
    graph->reserveEdge((n * n) >> 1);
    ListGraph::EdgeMap<float>* cost = new ListGraph::EdgeMap<float>(*graph);

    auto start_time = std::chrono::high_resolution_clock::now();
    int_sol = ApxSMCP::solve(
        n,
        edges_weights,
        graph,
        cost,
        int_sol
    );
    auto end_time = std::chrono::high_resolution_clock::now();
    auto apx_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    verify_solution(n, int_sol);
    double apx = get_sol_val(n, int_sol, edges_weights);

    delete cost;
    delete graph;

    struct Result result = { apx, (long long int)apx_time };
    return result;
}

void save_result_to_file(std::ofstream *result_file, ExecutionTracker tracker) {

    if (result_file->is_open()) {
        (*result_file) << tracker.instance.instance_file << ";";
        (*result_file) << tracker.instance.num_vertices << ";";
        (*result_file) << tracker.linear_relaxation->value << ";";
        (*result_file) << tracker.linear_relaxation->execution_time << ";";
        (*result_file) << tracker.approximation->value << ";";
        (*result_file) << tracker.approximation->execution_time;

        (*result_file) << std::endl;
        result_file->flush();
    } else {
        std::cerr << "Error: Unable to save result." << std::endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {

    vector<string>* files = new vector<string>;
    string dir = "../../../testInst";
    findDataFiles(dir, files);

    // allocate memory to solution
    double** double_sol = new double* [MAX_INSTANCE_SIZE];
    for (int i = 0; i < MAX_INSTANCE_SIZE; i++)
        double_sol[i] = new double[MAX_INSTANCE_SIZE];
    float** edges_weights = new float* [MAX_INSTANCE_SIZE];
    for (int i = 0; i < MAX_INSTANCE_SIZE; i++)
        edges_weights[i] = new float[MAX_INSTANCE_SIZE];
    int** int_sol = new int* [MAX_INSTANCE_SIZE];
    for (int i = 0; i < MAX_INSTANCE_SIZE; i++)
        int_sol[i] = new int[MAX_INSTANCE_SIZE];

    // save header to results file
    std::ofstream result_file("../../../_result2.csv", std::ios::app);
    if (result_file.is_open()) {
        result_file << "instance;num_vertices;relaxed_val;relaxed_time;apx_val;apx_time" << endl;
    } else {
        std::cerr << "Error: Unable to save result." << std::endl;
        exit(1);
    }

    for (string& file : *files) {

        char* str = "../../../testInst/rg-256-q-1x1.0001.ccpdp";
        string new_file = str;

        std::cout << "executing file " << file << std::endl;
        int n = 0;
        vector<pair<float, float>> vertices;

        read_instance(new_file, &n, &vertices);

        // init edges_weights matrix
        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                edges_weights[i][j] = vertices_distance(vertices[i], vertices[j]);

        Result linear_relaxation_result = execute_linear_relaxation(n, edges_weights, double_sol);

        Result approximate_result = execute_approximation(n, edges_weights, int_sol);

        struct ExecutionTracker tracker = {
            struct Instance {
                file,
                n
            },
            &linear_relaxation_result,
            &approximate_result
        };

        save_result_to_file(&result_file, tracker);

    }

    // free allocated memory
    for (int i = 0; i < MAX_INSTANCE_SIZE; i++)
        delete[] double_sol[i];
    delete[] double_sol;

    for (int i = 0; i < MAX_INSTANCE_SIZE; i++)
        delete[] edges_weights[i];
    delete[] edges_weights;

    for (int i = 0; i < MAX_INSTANCE_SIZE; i++)
        delete[] int_sol[i];
    delete[] int_sol;

    delete files;

    result_file.close();

    return 0;
}
