#include <fstream>
#include <iostream>
#include <string>

struct Interval {
public:
  double ub;
  double lb;
};

struct Neuron {
public:
  Interval concrete_values;
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

struct Network {
public:
  Layer *layers;
  double *postconditions;

  bool read_vnnlib(std::string vnnlib_filename) {
    bool bresult = false;

    std::fstream vnnlib_file(vnnlib_filename);
    if (!vnnlib_file.is_open()) {
      std::cerr << "Error: Could not open file " << vnnlib_filename
                << std::endl;
      return bresult;
    }

    std::string line;
    while (std::getline(vnnlib_file, line)) {
      std::cout << line << std::endl;
    }

    vnnlib_file.close();

    return bresult;
  }
};
