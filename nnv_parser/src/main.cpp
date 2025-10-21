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

  //
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

  return 0;
}
