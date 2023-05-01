#pragma once

#include "gurobi_c++.h"
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <sstream>

int solve(int n);
string itos(int i) { stringstream s; s << i; return s.str(); }
double distance(double* x, double* y, int i, int j);