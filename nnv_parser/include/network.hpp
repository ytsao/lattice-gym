#ifndef NETWORK_STRUCTURE_HPP
#define NETWORK_STRUCTURE_HPP

#include "layer.hpp"
#include "parser.hpp"
#include "specifications.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct Network {
public:
  std::vector<Layer> layers;
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

    spec = parser.parse(specifications);

    return true;
  }

  bool read_onnx(std::string onnx_filename) {
    return parser.load_network(onnx_filename, spec, layers);
  }

  void print_all_bounds() {
    for (size_t layer_id = 0; layer_id < layers.size(); ++layer_id) {
      print_bound_at_layer(layer_id);
    }
    return;
  }

  void print_bound_at_layer(size_t layer_id) {
    if (layer_id >= layers.size()) {
      std::cerr << "Error: Layer ID " << layer_id << " is out of range."
                << std::endl;
      return;
    }
    for (size_t neuron_id = 0; neuron_id < layers[layer_id].layer_size;
         ++neuron_id) {
      std::cout << "Layer " << layer_id << ", Neuron " << neuron_id << " : ["
                << layers[layer_id].neurons[neuron_id].bounds.getLb() << ", "
                << layers[layer_id].neurons[neuron_id].bounds.getUb() << "]\n";
    }
    return;
  }

private:
  NeuralNetworkParser parser;
};

#endif // NETWORK_STRUCTURE_HPP
