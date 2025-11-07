#ifndef NETWORK_STRUCTURE_HPP
#define NETWORK_STRUCTURE_HPP

#include "layer.hpp"
#include "log.hpp"
#include "parser.hpp"
#include "specifications.hpp"
#include <fstream>
#include <sstream>
#include <string>

struct Network {
public:
  std::vector<Layer> layers;
  Specification spec;
  size_t input_size;
  size_t output_size;
  size_t input_layer_id;

  bool read_vnnlib(std::string vnnlib_filename) {
    std::fstream vnnlib_file(vnnlib_filename);
    if (!vnnlib_file.is_open()) {
      Logger::log(Logger::Level::ERROR,
                  "Could not open file " + vnnlib_filename);
      return false;
    }

    std::ostringstream buffer;
    buffer << vnnlib_file.rdbuf();
    std::string specifications = buffer.str();
    vnnlib_file.close();

    spec = parser.parse(specifications);
    input_size = spec.numberOfInputs;
    output_size = spec.numberOfOutputs;

    return true;
  }

  bool read_onnx(std::string onnx_filename) {
    bool bresult = parser.load_network(onnx_filename, spec, layers);
    if (bresult) {
      input_layer_id = 0;
    }
    return bresult;
  }

  void dump_all_bounds() {
    for (size_t layer_id = 0; layer_id < layers.size(); ++layer_id) {
      dump_bounds_at_layer(layer_id);
    }
    return;
  }

  void dump_bounds_at_layer(size_t layer_id) {
    if (layer_id >= layers.size()) {
      Logger::log(Logger::Level::ERROR,
                  "Layer id " + std::to_string(layer_id) + " is out of range.");
      return;
    }
    for (size_t neuron_id = 0; neuron_id < layers[layer_id].layer_size;
         ++neuron_id) {
      Logger::log(Logger::Level::INFO,
                  "Layer " + std::to_string(layer_id) + ", Neuron " +
                      std::to_string(neuron_id) + " : [" +
                      std::to_string(
                          layers[layer_id].neurons[neuron_id].bounds.getLb()) +
                      ", " +
                      std::to_string(
                          layers[layer_id].neurons[neuron_id].bounds.getUb()) +
                      "]");
      Logger::log(
          Logger::Level::INFO,
          "Bias: [" + std::to_string(layers[layer_id].lower_biases[neuron_id]) +
              ", " + std::to_string(layers[layer_id].upper_biases[neuron_id]) +
              "]");
    }
    return;
  }

private:
  NeuralNetworkParser parser;
};

#endif // NETWORK_STRUCTURE_HPP
