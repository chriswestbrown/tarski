#include "nnet.h"
#include <math.h>
#include <bits/stdc++.h>
using namespace std;
namespace nnet_interpreter {

double majority(double* arr, int n)
{
    int count = 0;
    int other_index = 0;
    for(int i=1; i<n; i++){
      if(arr[i] == arr[1]){
        count++;
      }
      else{
        other_index = i;
      }
    }
    return (count >=(n/2)+1 ? arr[0] : arr[other_index]);
}

  float sigmoid(float x)
  {
     float exp_value;
     float return_value;

     /*** Exponential calculation ***/
     exp_value = exp((double) -x);

     /*** Final sigmoid value ***/
     return_value = 1 / (1 + exp_value);

     return return_value;
  }

  /**
     Parent class that splits off into two usable types: Input(for features) and
     MiddleNode for all other Nodes. Nodes are never actually created.
  */
  class Node{
  public:
    virtual ~Node() { }
    virtual double getValue(vector<double> &values){return values[node_num];}
    int node_num;
  };

  /**
     Subclass of Node, essentially placeholders for values that will come in as
     part of the feature array.
  */
  class InputNode : public Node{
  public:
    InputNode(int nn){
      this->node_num=nn;
    }
  private:
  };

  /**
     Subclass of Node, holds all values required to make calculations: inputs, weights,
     bias, activation function.
  */
  class MiddleNode : public Node{
  public:
    MiddleNode(string f, int nn, vector<int> &i, vector<double> &w, double b);
    virtual double getValue(vector<double> &values);
    string getFunc();
  private:
    string func;
    vector<int> inputs;
    vector<double>weights;
    double bias;
  };


  //Constructor
  MiddleNode::MiddleNode(string f, int nn, vector<int> &i, vector<double> &w, double b){
    this->func = f;
    this->node_num = nn;
    this->inputs = i;
    this->weights = w;
    this->bias = b;
  }

  string MiddleNode::getFunc(){
    return this->func;
  }
  /**
     Uses weights and inputs (as indexes to an array of values), to calculate the dot
     product of weights and values and then adding in bias. Then checks the activation
     function and applies it to the calculated value.
  */
  double MiddleNode::getValue(vector<double> &values){
    double ret = 0.0;
    for(int i=0; i<this->inputs.size(); i++){
      ret += values[this->inputs.at(i)]*this->weights.at(i);
    }
    ret += this->bias;
    if(this->func.compare("relu")==0){
      return max(ret,0.0);
    }
    else if(this->func.compare("linear")==0){
      return ret;
    }
    else if(this->func.compare("tanh")==0){
      return std::tanh(ret);
    }
    else if(this->func.compare("sigmoid")==0){
      return sigmoid(ret);
    }
  }

  /**
     Takes an input string in the following format:
     #nodes ( type function node number bias [ inputs ] [ weights ] )...
     Everything must be seperated by a space
     Refer to graphtest.generateGraph() to get a proper graph string
     Parses the input stream, creates the corresponding nodes, and adds to the vector
  */
  Graph::Graph(const char* graph_string){
    stringstream ss(graph_string);
    ss >> num_nodes;
    if(num_nodes > 0){
	    type = 0;
	    values.resize(num_nodes);
	    num_features = 0;

	    for(int i=0; i<num_nodes;i++){
	      char temp;
	      char func[10];
	      char type;
	      int nn,ln,nil;
	      vector<int> inputs;
	      vector<double> weights;
	      double bias;

	      ss >> temp; // '('s
	      ss >> type;
	      ss >> func;
	      ss >> nn >> bias;
	      string n;
	      ss >> temp; // '['
	      while(ss >> n ){
		if(n.compare("]")==0)
		  break;
		inputs.push_back(stoi(n));
	      }
	      ss >> temp; // '['
	      while(ss >> n ){
		if(n.compare("]")==0)
		  break;
		weights.push_back(stod(n));
	      }
	      if(type == 'i'){
		num_features++;
		this->nodes.push_back(new InputNode(nn));
	      }
	      else if(type == 'm'){
		this->nodes.push_back(new MiddleNode(func,nn,inputs,weights,bias));
	      }
	      ss >> temp; // ')'
	    }
    }
    else{
    	type=1;
      cout << type << endl;
    	ss >> num_nets;
      vote_values.resize(num_nets);
      vote_nodes.resize(num_nets);
    	for(int k=0; k<num_nets; k++){
    	    ss >> num_nodes;
    	    vote_values[k].resize(num_nodes);
    	    num_features = 0;

    	    for(int i=0; i<num_nodes;i++){
    	      char temp;
    	      char func[10];
    	      char type;
    	      int nn,ln,nil;
    	      vector<int> inputs;
    	      vector<double> weights;
    	      double bias;

    	      ss >> temp; // '('s
    	      ss >> type;
    	      ss >> func;
    	      ss >> nn >> bias;
    	      string n;
    	      ss >> temp; // '['
    	      while(ss >> n ){
    		if(n.compare("]")==0)
    		  break;
    		inputs.push_back(stoi(n));
    	      }
    	      ss >> temp; // '['
    	      while(ss >> n ){
    		if(n.compare("]")==0)
    		  break;
    		weights.push_back(stod(n));
    	      }
    	      if(type == 'i'){
    		num_features++;
    		this->vote_nodes[k].push_back(new InputNode(nn));
    	      }
    	      else if(type == 'm'){
    		this->vote_nodes[k].push_back(new MiddleNode(func,nn,inputs,weights,bias));
    	      }
    	      ss >> temp; // ')'
    	    }
    	}
    }
  }


  /**
     Sets the inputs(features) once they are known. Features will change every run,
     but the weights will stay the same for an entire round.
  */
  void Graph::setInputs(const vector<double> &features){
	  if(type==0){
	    for(int i=0; i<features.size(); i++){
	      values[i] = features[i];
	    }
	  }
	  else if(type==1){
		  for(int k=0; k<num_nets; k++){
		    for(int i=0; i<features.size(); i++){
		  vote_values[k][i] = features[i];
		    }
		  }
    }
  }
  void Graph::setInputs(const vector<float> &features){
    if(type==0){
	    for(int i=0; i<features.size(); i++){
	      values[i] = features[i];
	    }
	  }
	  else if(type==1){
		  for(int k=0; k<num_nets; k++){
		    for(int i=0; i<features.size(); i++){
		      vote_values[k][i] = features[i];
		    }
		  }
    }
  }
  /**
     Uses memoization to develop the final value that will be returned by the model,
     by systematically calling each node's getValue() function and providing the current
     values if needed, in the order that they appear in the vector
  */
  double Graph::calculate(){
    if(type==0){
      for(int i = num_features; i < nodes.size(); ++i)
        values[i] = nodes[i]->getValue(values);
      return this->values[nodes.size() - 1];
    }
    else if(type==1){
      double votes[num_nets];
      for(int k=0; k<num_nets; k++){
        for(int i = num_features; i < vote_nodes[k].size(); ++i)
          vote_values[k][i] = vote_nodes[k][i]->getValue(vote_values[k]);
        votes[k] = this->vote_values[k][vote_nodes[k].size() - 1];
      }
      return majority(votes,num_nets);

    }

  }

  Graph::~Graph(){
    if(type==0){
      for(int i = 0; i < nodes.size(); ++i)
        delete nodes[i];
    }
    else if(type==1){
      for(int k = 0; k<num_nets; k++){
        for(int i = 0; i < vote_nodes[k].size(); ++i)
          delete vote_nodes[k][i];
      }
    }

  }

  Graph::DecisionKind Graph::getDecisionKind(){
    if(type==0){
      MiddleNode* final = (MiddleNode*)nodes[nodes.size()-1];
      if(final->getFunc().compare("tanh")==0) return Graph::SIGN;
      else return Graph::ZEROONE;
    }
    else if(type==1){
      MiddleNode* final = (MiddleNode*)vote_nodes[0][vote_nodes[0].size()-1];
      if(final->getFunc().compare("tanh")==0) return Graph::SIGN;
      else return Graph::ZEROONE;
    }
  }

}
