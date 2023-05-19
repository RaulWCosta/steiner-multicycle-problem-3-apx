#pragma once

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include "libs/dirent.h"

using namespace std;

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


void read_instance(string& circuitfilename, int* n, vector<pair<float, float>>* vertices, vector<int>* source2sink) {
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
        cout << "Deu problema!" << endl;
        exit(1);
    }

    int indice;
    float x, y;
    vertices = new vector<pair<float, float>>;

    for (int i = 0; i < nnodes && !in.eof(); i++) {
        getline(in, linha);
        sscanf_s(linha.c_str(), "%d\t%f\t%f", &indice, &x, &y);
        pair<float, float> elem = {x, y};
        vertices->push_back(elem);
    }

    source2sink = new vector<int>;
    for (int i = 0; i < nnodes / 2; i++) {
        source2sink->push_back(i + (int)(nnodes/2));
    }

    *n = nnodes;

}


void findDataFiles(string folder, vector<string> *files){

    DIR *dir;
    struct dirent *diread;

    if ( (dir = opendir(folder.c_str())) == nullptr){
        cout << "Deu problema!" << endl;
        exit(1);
    }

    while( (diread = readdir(dir)) != nullptr) {
        if ( strcmp(diread->d_name, "files.lst") == 0 ){
            //abre o arquivo "files.lst" e pega a lista de arquivos com os problemas para analisar
            string file_path;
            string line = "";
            file_path = folder + "/files.lst";
            fstream file_obj;
            file_obj.open(file_path, ios::in);
            while(getline(file_obj, line)) {
                string full_path;
                full_path = folder + "/" + line;
                files->push_back(full_path);
            }

        }else{
            if ( (diread->d_type != isFile)
            &&  (strcmp(diread->d_name,".")  != 0)
            &&  (strcmp(diread->d_name, "..") != 0) ){
                string aux_folder = folder +"/"+ string(diread->d_name);
                findDataFiles( aux_folder, files);
            }
        }
    }

}