// steiner-multicycle-problem-3-apx.cpp : Defines the entry point for the application.
//

#include <iostream>

#include "gurobi_c++.h"
#include "src/relaxed_solver_surviv_net.h"

#include <vector>

// #include <lemon/list_graph.h>


#include <lemon/lmath.h>

#include<lemon/graph_to_eps.h>
#include<lemon/list_graph.h>

int test()
{
    std::vector<int> source2sink = { 2, 3 };
    std::vector<Vertex> vertices = {
        Vertex(0.0f, 0.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
        Vertex(0.0f, 1.0f),
    };

    GRBEnv env = new GRBEnv(true);
    env.set("LogFile", "mip1.log");
    env.start();

    // Create an empty model
    GRBModel model = GRBModel(env);

    RelaxedSurvivableNetworkSolver solver(model, 4, source2sink, vertices);

    solver.solve(model);

    return 0;
}

int main(int argc, char* argv[]) {
    // test();
  lemon::ListGraph g;
  typedef lemon::ListGraph::Node Node;
  typedef lemon::ListGraph::NodeIt NodeIt;
  typedef lemon::ListGraph::Edge Edge;
  typedef lemon::dim2::Point<int> Point;
  
  Node n1=g.addNode();
  Node n2=g.addNode();
  Node n3=g.addNode();
  Node n4=g.addNode();
  Node n5=g.addNode();
  std::cout <<"foio";
    return 0;
}