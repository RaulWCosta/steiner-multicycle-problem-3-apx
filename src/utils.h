#pragma once

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <lemon/full_graph.h>

using namespace std;
using namespace lemon;

unsigned char isFile =0x8;

string itos(int i) { stringstream s; s << i; return s.str(); }

template<typename T>
void print_matrix(int n, T** m) {

    cout << "----------------" << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            cout << m[i][j] << " ";
        cout << endl;
    }
    cout << "----------------" << endl;
}


float vertices_distance(const std::pair<float, float>& a, const std::pair<float, float>& b) {
    float dx = a.first - b.first;
    float dy = a.second - b.second;

    return sqrt(dx * dx + dy * dy);
}


void read_instance(string& circuitfilename, int* n, vector<pair<float, float>>* vertices) {
    // string circuitfilename = "../../../allInst/toy.ccpdp";
    ifstream in;
    string linha, substring, name;
    int nnodes=0;
    int t;
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
            sscanf_s(substring.c_str(),"%d",&nnodes);
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
        cout << "Deu problema aqui!" << endl;
        exit(1);
    }

    int indice;
    float x, y;
    

    for (int i = 0; i < nnodes && !in.eof(); i++) {
        getline(in, linha);
        sscanf_s(linha.c_str(), "%d\t%f\t%f", &indice, &x, &y);
        pair<float, float> elem = {x, y};
        vertices->push_back(elem);
    }

    *n = nnodes;

}


void findDataFiles(string folder, vector<string> *files){

    string line = "";
    string file_path = folder + "/files.lst";
    std::ifstream file_obj(file_path);

    if (!file_obj.is_open()) {
        cout << "Deu problema 2!" << endl;
        exit(1);
    }

    while(getline(file_obj, line)) {
        string full_path;
        full_path = folder + "/" + line;
        files->push_back(full_path);
    }

}

    bool verify_terminals_connected(int n, int** sol) {
        int half_n = n >> 1;
        vector<int> stack;

        for (int s = 0; s < half_n; s++) {
            int t = s + half_n;

            bool flag_valid_cycle = false;
            vector<bool> visited(n, false);

            stack.clear();

            visited[s] = true;
            stack.push_back(s);

            while (!stack.empty()) {
                int curr_node = stack[stack.size() - 1];
                stack.pop_back();

                if (curr_node == t) {
                    flag_valid_cycle = true;
                    break;
                }

                for (int i = 0; i < n; i++) {
                    if (!sol[curr_node][i])
                        continue;

                    if (!visited[i]) {
                        visited[i] = true;
                        stack.push_back(i);
                    }
                }
            }

            if (!flag_valid_cycle) {
                // ciclo invalido!
                return false;
            }

        }
        return true;

    }


vector<int> get_vertices_degrees(int n, int** sol) {
    vector<int> degrees(n, 0);
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            degrees[i] += sol[i][j];
            degrees[j] += sol[i][j];
        }
    }
    return degrees;
}


void verify_solution(string file_name, int n, int** sol) {
    vector<int> vertices_degree = get_vertices_degrees(n, sol);

    for (int i = 0; i < vertices_degree.size(); i++) {
        if (vertices_degree[i] % 2) {
            cout << "file = " << file_name << " com vertice " << i << " de grau impar!" << endl;
            exit(1);
        }
    }

    // check if terminals are connected
    if (!verify_terminals_connected(n, sol)) {
        cout << "file = " << file_name << " com teminais desconectados!" << endl;
    }

}

float get_sol_val(int n, int** sol, float** edges_weights) {

    float total_cost = 0.0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {

            total_cost += sol[i][j] * edges_weights[i][j];

        }
    }

    return total_cost;
}