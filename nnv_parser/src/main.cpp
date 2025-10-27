#include "deeppoly_propagation.hpp"
#include "naive_interval_propagation.hpp"
#include "network.hpp"
#include "symbolic_interval_propagation.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 4) {
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

  // Execute the propagation method.
  if (std::strcmp(argv[3], "ibp") == 0) {
    NaiveIntervalPropagation ibp;
    if (ibp.execute(nnv)) {
      std::cout << "The naive_interval_propagation is done." << std::endl;
      nnv.print_all_bounds();
    } else {
      std::cerr << "The naive_interval_propagation is failed." << std::endl;
    }
  } else if (std::strcmp(argv[3], "sip") == 0) {
    SymbolicIntervalPropagation sip;
    if (sip.execute(nnv)) {
      std::cout << "The symbolic_interval_propagation is done." << std::endl;
      nnv.print_all_bounds();
    } else {
      std::cerr << "The symbolic_interval_propagation is failed." << std::endl;
    }
  } else if (std::strcmp(argv[3], "deeppoly") == 0) {
    DeepPolyPropagation deeppoly;
    if (deeppoly.execute(nnv)) {
      std::cout << "The deeppoly_propagation is done." << std::endl;
      nnv.print_all_bounds();
    } else {
      std::cerr << "The deeppoly_propagation is failed." << std::endl;
    }
  }

  return 0;
}
