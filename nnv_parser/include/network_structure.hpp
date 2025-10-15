#ifndef NETWORK_STRUCTURE_HPP
#define NETWORK_STRUCTURE_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Interval {
public:
  Interval() : ub(9999.99), lb(-9999.99) {}

  void setLb(double _lb) { lb = _lb; }
  void setUb(double _ub) { ub = _ub; }
  double getLb() { return lb; }
  double getUb() { return ub; }

private:
  double ub;
  double lb;
};

struct Neuron {
public:
  size_t id; // the index in the same layer, it starts from 0.
  std::vector<Interval> bounds; // each dimension represents a disjunctive term.

  Neuron() : id(0), bounds() {}

  void setId(int _id) { id = _id; }
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
  std::vector<std::vector<float>> weights;
  std::vector<float> biases;
  Neuron *neurons;
  double layer_size;
};

struct Specification {
  size_t numberOfInputs = 0;
  size_t numberOfOutputs = 0;
  std::map<std::string, Neuron> variables;

  // postconditions: Ay+ b<=0;
  // first dimension: the index of disjunctive terms;
  std::vector<std::vector<std::vector<int>>> A;
  std::vector<std::vector<double>> b;

  Specification() : numberOfInputs(0), numberOfOutputs(0) {}
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
