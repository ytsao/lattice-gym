#include "naive_interval_propagation.hpp"
#include "network.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 3) {
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

  // create network object and parse vnnlib & onnx file.
  Network nnv;

  std::cout << "Parsing vnnlib: " << argv[1] << " ..." << std::endl;
  if (!nnv.read_vnnlib(argv[1])) {
    std::cerr << "Error: Could not parse vnnlib file " << argv[1] << std::endl;
    return 1;
  }
  std::cout << "Parsing vnnlib is done." << std::endl;
  std::cout << "Parsing onnx: " << argv[2] << " ..." << std::endl;
  if (!nnv.read_onnx(argv[2])) {
    std::cerr << "Error: Could not parse onnx file " << argv[2] << std::endl;
    return 1;
  }
  std::cout << "Parsing onnx is done." << std::endl;

  // TODO: Execute the propagation method.
  NaiveIntervalPropagation ibp;
  if (ibp.execute(nnv)) {
    std::cout << "The naive_interval_propagation is done." << std::endl;
  } else {
    std::cerr << "The naive_interval_propagation is failed." << std::endl;
  }

  return 0;
}
