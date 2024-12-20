#ifndef _NNET_INTERPRETER_
#define _NNET_INTERPRETER_

#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <cmath>

namespace nnet_interpreter
{

  class Node;

  /**
     Contains a vector of nodes that make up the graph, indexed in a topological sort.
  */
  class Graph{
  public:
    enum DecisionKind{ZEROONE,SIGN};
    Graph(const char* graph_string);
    ~Graph();
    void setWeights(char* weight_string);
    void setBias(char* bias_string);
    void print_graph();
    void setInputs(const std::vector<double> &features);
    void setInputs(const std::vector<float> &features);
    double calculate();
    void printValues();
    void clean();
    DecisionKind getDecisionKind();
  private:
    std::vector<Node*> nodes;
    std::vector<double> values;
    std::vector<std::vector<Node*>> vote_nodes;
    std::vector<std::vector<double>> vote_values;
    int num_nodes, num_features, type, num_nets;
  };

}

#endif
