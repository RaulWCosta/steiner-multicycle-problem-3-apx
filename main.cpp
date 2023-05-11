
#include <vector>
#include <utility>

#include "src/surv_net_2apx.h"
#include "src/smcp_3apx.h"

#include "src/exact_solution.h"

using namespace std;

int main(int argc, char* argv[]) {
    int n = 4;

    vector<int> source2sink = {2, 3};
    vector<pair<float, float>> vertices = {
        pair<float, float>(0.0, 0.0),
        pair<float, float>(1.0, 0.0),
        pair<float, float>(1.0, 1.0),
        pair<float, float>(0.0, 1.0),
    };

    SurvivableNetwork::solve(n, source2sink, vertices);
    // ExactSMCP::solve(n, source2sink, vertices);
}
