#ifndef NETWORK_STRUCTURE_HPP
#define NETWORK_STRUCTURE_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

struct Interval {
public:
  double ub;
  double lb;

  Interval() : ub(9999.99), lb(-9999.99) {}

  void setLb(double _lb) { lb = _lb; }
  void setUb(double _ub) { ub = _ub; }
};

struct Neuron {
public:
  Interval bounds;
};

enum class LayerType {
  FullyConnected,
  Convolutional,
  MaxPooling,
  Flatten,
  ReLU,
  Sigmoid,
  Tanh,
  Softmax
};

struct Layer {
public:
  LayerType type;
  double **weights;
  double *biases;
  Neuron *neurons;
  double layer_size;
};

struct Specification {
  int numberOfInputs;
  int numberOfOutputs;
  std::map<std::string, Interval> variables;

  // postconditions: Ay+ b<=0;
  double **A;
  double *b;

  Specification()
      : numberOfInputs(0), numberOfOutputs(0), A(nullptr), b(nullptr) {}
};

struct Network {
public:
  Layer *layers;
  Specification spec;

  bool read_vnnlib(std::string vnnlib_filename) {
    std::fstream vnnlib_file(vnnlib_filename);
    if (!vnnlib_file.is_open()) {
      std::cerr << "Error: Could not open file " << vnnlib_filename
                << std::endl;
      return false;
    }

    std::ostringstream buffer;
    buffer << vnnlib_file.rdbuf();
    std::string specifications = buffer.str();
    vnnlib_file.close();

    return true;
  }

private:
  bool parse(std::string specifications) { return true; }
};

#endif // NETWORK_STRUCTURE_HPP
