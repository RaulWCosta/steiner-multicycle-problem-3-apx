
#include <vector>
#include <utility>
#include <fstream>
#include <string>

#include "src/utils.h"
#include "src/surv_net_2apx.h"
#include "src/smcp_3apx.h"

#include "src/exact_solution.h"

using namespace std;


struct Instance {
    vector<pair<float, float>>* vertices;
    vector<int>* source2sink;
};

Instance read_instance(/*string& circuitfilename*/) {
    string circuitfilename = "allInst/toy.ccpdp";
    ifstream in;
    string linha, substring, name;
    int nnodes=0, t;
    in.open(circuitfilename);

    if (!in.is_open()) {
        cout << "ERROR: Problem to open file " << circuitfilename << "\n";
        exit(1);
    }

    while (!in.eof()) {
        getline(in, linha);
        // cout << "getline: " << linha << endl;
        t = linha.find("DIMENSION");
        if (t == string::npos) {
            continue;
        } else {
            t = linha.find(":");
            substring = linha.substr(t+1);
            sscanf(substring.c_str(),"%d",&nnodes);
            break;
        }
    }

    // find node field
    while (!in.eof()) {
        getline(in, linha);
        // cout << "getline: " << linha << endl;
        t = linha.find("NODE_COORD_SECTION");
        if (t == string::npos) {
            continue;
        }
        break;
    }

    if (in.eof()) {
        cout << "Deu problema!" << endl;
        exit(1);
    }

    int indice;
    float x, y;
    vector<pair<float, float>>* vertices = new vector<pair<float, float>>;

    for (int i = 0; i < nnodes && !in.eof(); i++) {
        getline(in, linha);
        sscanf(substring.c_str(), "%d\t%f\t%f", &indice, &x, &y);
        pair<float, float> elem = {x, y};
        vertices->push_back(elem);
    }

    vector<int>* source2sink = new vector<int>;
    for (int i = 0; i < nnodes / 2; i++) {
        source2sink->push_back(i + (int)(nnodes/2));
    }

    Instance inst = {
        vertices,
        source2sink
    };

}



int main(int argc, char* argv[]) {
    
    // read_instance();
    
    int n = 4;

    vector<int> source2sink = {2, 3};
    vector<pair<float, float>> vertices = {
        pair<float, float>(0.0, 0.0),
        pair<float, float>(1.0, 0.0),
        pair<float, float>(1.0, 1.0),
        pair<float, float>(0.0, 1.0),
    };

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

    int** sn_sol = SurvivableNetwork::solve(n, source2sink, *graph, *cost);
    // ApxSMCP::solve(n, sn_sol, *graph, *cost);

    // ExactSMCP::solve(n, source2sink, vertices);
}
