#include "deeppoly_propagation.hpp"
#include "naive_interval_propagation.hpp"
#include "network.hpp"
#include "symbolic_interval_propagation.hpp"
#include "utility.hpp"
#include <iostream>

void verify(const std::string &vnnlib_path, const std::string &onnx_path,
            const std::string &propagation_method, size_t &num_verified) {
  // create network object and parse vnnlib & onnx file.
  Network nnv;

  std::cout << "Parsing vnnlib: " << vnnlib_path << " ..." << std::endl;
  if (!nnv.read_vnnlib(vnnlib_path)) {
    std::cerr << "Error: Could not parse vnnlib file " << vnnlib_path
              << std::endl;
    return;
  }
  std::cout << "Parsing vnnlib is done." << std::endl;
  std::cout << "Parsing onnx: " << onnx_path << " ..." << std::endl;
  if (!nnv.read_onnx(onnx_path)) {
    std::cerr << "Error: Could not parse onnx file " << onnx_path << std::endl;
    return;
  }
  std::cout << "Parsing onnx is done." << std::endl;

  // Execute the propagation method.
  if (std::strcmp(propagation_method.c_str(), "ibp") == 0) {
    NaiveIntervalPropagation ibp;
    if (ibp.execute(nnv)) {
      std::cout << "The naive_interval_propagation result is UNSAT."
                << std::endl;
      num_verified++;
    } else {
      std::cerr << "The naive_interval_propagation is not able to have "
                   "a conclusive verification result."
                << std::endl;
    }
  } else if (std::strcmp(propagation_method.c_str(), "sip") == 0) {
    SymbolicIntervalPropagation sip;
    if (sip.execute(nnv)) {
      std::cout << "The symbolic_interval_propagation result is UNSAT."
                << std::endl;
      num_verified++;
    } else {
      std::cerr << "The symbolic_interval_propagation is not able to have a "
                   "conclusive verification result."
                << std::endl;
    }
  } else if (std::strcmp(propagation_method.c_str(), "deeppoly") == 0) {
    DeepPolyPropagation deeppoly;
    if (deeppoly.execute(nnv)) {
      std::cout << "The deeppoly_propagation result is UNSAT." << std::endl;
      num_verified++;
    } else {
      std::cerr << "The deeppoly_propagation is not able to have a conlcusive "
                   "verification result."
                << std::endl;
    }
  }
  // nnv.dump_all_bounds();
  nnv.dump_bounds_at_layer(nnv.layers.size() - 2);
  nnv.dump_bounds_at_layer(nnv.layers.size() - 1);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <input_file>" << std::endl;
  }

  size_t num_verified = 0;
  if (argc == 2) {
    std::vector<Utility::TaskEntry> tasks = Utility::read_csv(argv[1]);
    for (Utility::TaskEntry task : tasks) {
      verify(task.vnnlib_path, task.onnx_path, task.propagation_method,
             num_verified);
      std::cout << "Number of verified: " << num_verified << std::endl;
    }
  } else if (argc == 4) {
    verify(argv[1], argv[2], argv[3], num_verified);
  } else {
    std::cerr << "Error: unknown configuration." << std::endl;
  }

  return 0;
}
