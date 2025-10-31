#include "deeppoly_propagation.hpp"
#include "naive_interval_propagation.hpp"
#include "network.hpp"
#include "symbolic_interval_propagation.hpp"
#include "utility.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <input_file>" << std::endl;
  }

  std::vector<Utility::TaskEntry> tasks = Utility::read_csv(argv[1]);
  std::cout << "Number of tasks = " << tasks.size() << std::endl;
  for (Utility::TaskEntry task : tasks) {
    std::cout << "VNNLIB: " << task.vnnlib_path << ", ONNX: " << task.onnx_path
              << ", METHOD: " << task.propagation_method << std::endl;
    std::ifstream f(task.vnnlib_path);
    if (!f.is_open()) {
      std::cerr << "Error: Could not open file " << task.vnnlib_path
                << std::endl;
      return 1;
    }
    std::ostringstream buffer;
    buffer << f.rdbuf();
    std::string input = buffer.str();
    f.close();

    // create network object and parse vnnlib & onnx file.
    Network nnv;

    std::cout << "Parsing vnnlib: " << task.vnnlib_path << " ..." << std::endl;
    if (!nnv.read_vnnlib(task.vnnlib_path)) {
      std::cerr << "Error: Could not parse vnnlib file " << task.vnnlib_path
                << std::endl;
      return 1;
    }
    std::cout << "Parsing vnnlib is done." << std::endl;
    std::cout << "Parsing onnx: " << task.onnx_path << " ..." << std::endl;
    if (!nnv.read_onnx(task.onnx_path)) {
      std::cerr << "Error: Could not parse onnx file " << task.onnx_path
                << std::endl;
      return 1;
    }
    std::cout << "Parsing onnx is done." << std::endl;

    // Execute the propagation method.
    if (std::strcmp(task.propagation_method.c_str(), "ibp") == 0) {
      NaiveIntervalPropagation ibp;
      if (ibp.execute(nnv)) {
        std::cout << "The naive_interval_propagation result is UNSAT."
                  << std::endl;
      } else {
        std::cerr << "The naive_interval_propagation is not able to have "
                     "a conclusive verification result."
                  << std::endl;
      }
    } else if (std::strcmp(task.propagation_method.c_str(), "sip") == 0) {
      SymbolicIntervalPropagation sip;
      if (sip.execute(nnv)) {
        std::cout << "The symbolic_interval_propagation result is UNSAT."
                  << std::endl;
      } else {
        std::cerr << "The symbolic_interval_propagation is not able to have a "
                     "conclusive verification result."
                  << std::endl;
      }
    } else if (std::strcmp(task.propagation_method.c_str(), "deeppoly") == 0) {
      DeepPolyPropagation deeppoly;
      if (deeppoly.execute(nnv)) {
        std::cout << "The deeppoly_propagation result is UNSAT." << std::endl;
      } else {
        std::cerr
            << "The deeppoly_propagation is not able to have a conlcusive "
               "verification result."
            << std::endl;
      }
    }
    // nnv.print_all_bounds();
    // nnv.print_bound_at_layer(nnv.layers.size() - 2);
    nnv.print_bound_at_layer(nnv.layers.size() - 1);
  }

  return 0;
}
