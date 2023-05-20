
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

        int n = 0;
        vector<pair<float, float>>* vertices = new vector<pair<float, float>>;;
        vector<int>* source2sink = new vector<int>;

        read_instance(file, &n, vertices, source2sink);

        // int n = 4;

        // vector<int> source2sink = {2, 3};
        // vector<pair<float, float>> vertices = {
        //     pair<float, float>(0.0, 0.0),
        //     pair<float, float>(1.0, 0.0),
        //     pair<float, float>(1.0, 1.0),
        //     pair<float, float>(0.0, 1.0),
        // };

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
                dist = vertices_distance((*vertices)[i], (*vertices)[j]);
                (*cost)[graph->edge(u, v)] = dist;
            }
        }

        int** sn_sol = SurvivableNetwork::solve(n, *source2sink, *graph, *cost);

        sn_sol = ApxSMCP::solve(n, sn_sol, *graph, *cost);

        print_matrix(n, sn_sol);

        // ExactSMCP::solve(n, source2sink, vertices);
        delete vertices;
        delete source2sink;

    }

}
