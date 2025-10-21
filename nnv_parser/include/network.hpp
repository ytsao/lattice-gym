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
    return parser.load_network(onnx_filename, layers);
  }

private:
  NeuralNetworkParser parser;
};

#endif // NETWORK_STRUCTURE_HPP
