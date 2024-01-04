
#include <vector>
#include <utility>
#include <fstream>
#include <string>
#include <chrono>
#include <tuple>

#include "src/utils.h"
#include "src/smcp_3apx.h"
#include "src/linear_solution.h"

#define DEBUG_FILE 0

int MAX_INSTANCE_SIZE = 600;

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
    // struct Result *approximation;
    struct Result *approximation_survival_network;
    struct Result *approximation_perfect_matching;
    struct Result *approximation_shortcutting;
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

    double relaxed = get_solution_value(n, double_sol, edges_weights);

    for (int i = 0; i < n; i++)
        delete[] edge_vars[i];
    delete[] edge_vars;

    delete env;
    delete graph;
    delete cost;

    struct Result result = { relaxed, (long long int)relaxed_time };
    return result;
}

std::tuple<Result, Result, Result> execute_approximation(int n, float** edges_weights, int** int_sol) {

    // allocate memory
    ListGraph* graph = new ListGraph();
    graph->reserveNode(n);
    graph->reserveEdge((n * n) >> 1);
    ListGraph::EdgeMap<float>* cost = new ListGraph::EdgeMap<float>(*graph);

    GRBVar* edge_vars = new GRBVar [ (n * n) >> 1 ];
    ListGraph::EdgeMap<float>* aux_lp_sol = new ListGraph::EdgeMap<float>(*graph);

    list<int>* euclidean_path = new list<int>;
    vector<int>* stack = new vector<int>;

    // execute survivable network algorithm
    auto start_time = std::chrono::high_resolution_clock::now();
    SurvivableNetwork::solve(
        n,
        graph,
        cost,
        edges_weights,
        edge_vars,
        aux_lp_sol,
        int_sol
    );
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    double sn_val = get_solution_value(n, int_sol, edges_weights);
    struct Result result_sn = { sn_val, (long long int)execution_time };

    // execute perfect matching
    start_time = std::chrono::high_resolution_clock::now();
    ApxSMCP::solve_without_shortcutting(
        n,
        edges_weights,
        graph,
        cost,
        int_sol // assumes survivable network was executed prior
    );

    end_time = std::chrono::high_resolution_clock::now();
    execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    double pm_val = get_solution_value(n, int_sol, edges_weights);
    struct Result result_perf_match = { pm_val, (long long int)execution_time };
    verify_solution(n, int_sol);

    // execute short-cutting
    start_time = std::chrono::high_resolution_clock::now();
    ApxSMCP::short_cutting(n, euclidean_path, stack, int_sol);
    end_time = std::chrono::high_resolution_clock::now();
    execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    double sc_val = get_solution_value(n, int_sol, edges_weights);
    struct Result result_shortcutting = { sc_val, (long long int)execution_time };

    verify_solution(n, int_sol);

    delete cost;
    delete graph;
    delete edge_vars;
    delete aux_lp_sol;
    delete stack;
    delete euclidean_path;

    return std::make_tuple(result_sn, result_perf_match, result_shortcutting);
}

void save_result_to_file(std::ofstream *result_file, ExecutionTracker tracker) {

    if (result_file->is_open()) {
        (*result_file) << tracker.instance.instance_file << ";";
        (*result_file) << tracker.instance.num_vertices << ";";

        (*result_file) << tracker.linear_relaxation->value << ";";
        (*result_file) << tracker.linear_relaxation->execution_time << ";";
        
        (*result_file) << tracker.approximation_survival_network->value << ";";
        (*result_file) << tracker.approximation_survival_network->execution_time << ";";

        (*result_file) << tracker.approximation_perfect_matching->value << ";";
        (*result_file) << tracker.approximation_perfect_matching->execution_time << ";";

        (*result_file) << tracker.approximation_shortcutting->value << ";";
        (*result_file) << tracker.approximation_shortcutting->execution_time;

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
#if DEBUG_FILE
    files->push_back("../../../testInst/rg-256-q-1x1.0001.ccpdp");
    std::cout << "my directory is " << argv[0] << std::endl;
#else
    findDataFiles(dir, files);
#endif

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
        result_file << "instance;";
        result_file << "num_vertices;";

        result_file << "relaxed_val;";
        result_file << "relaxed_time;";
        
        result_file << "survive_net_val;";
        result_file << "survive_net_time;";

        result_file << "perfect_matching_val;";
        result_file << "perfect_matching_time;";

        result_file << "short_cutting_val;";
        result_file << "short_cutting_time";

        result_file << endl;
    } else {
        std::cerr << "Error: Unable to save result." << std::endl;
        exit(1);
    }

    for (string& file : *files) {

        std::cout << "executing file " << file << std::endl;
        int n = 0;
        vector<pair<float, float>> vertices;

        read_instance(file, &n, &vertices);

        // init edges_weights matrix
        for (int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                edges_weights[i][j] = vertices_distance(vertices[i], vertices[j]);

        Result linear_relaxation_result = execute_linear_relaxation(n, edges_weights, double_sol);

        Result sn_result, perf_match_result, short_cutting_result;
        std::tie(sn_result, perf_match_result, short_cutting_result) = execute_approximation(
            n, edges_weights, int_sol
        );

        struct ExecutionTracker tracker = {
            struct Instance {
                file,
                n
            },
            &linear_relaxation_result,
            &sn_result,
            &perf_match_result,
            &short_cutting_result
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
