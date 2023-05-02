#pragma once

#include <sstream>
#include <iostream>

using namespace std;

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
