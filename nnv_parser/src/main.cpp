#include "ast.hpp"
#include "network_structure.hpp"
#include "parser.hpp"
#include "specifications.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <input_file>" << std::endl;
  }
  std::ifstream f(argv[1]);
  if (!f.is_open()) {
    std::cerr << "Error: Could not open file " << argv[1] << std::endl;
    return 1;
  }
  std::ostringstream buffer;
  buffer << f.rdbuf();
  std::string input = buffer.str();
  f.close();

  std::cout << "Parsing vnnlib: " << argv[1] << " ..." << std::endl;
  NeuralNetworkParser parser;
  Specification spec = parser.parse(input);
  parser.load_network("../networks/ACASXU_run2a_1_1_batch_2000.onnx");

  return 0;
}
