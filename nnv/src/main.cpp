#include "deeppoly_propagation.hpp"
#include "log.hpp"
#include "naive_interval_propagation.hpp"
#include "network.hpp"
#include "symbolic_interval_propagation.hpp"
#include "utility.hpp"

void verify(const std::string &vnnlib_path, const std::string &onnx_path,
            const std::string &propagation_method, size_t &num_verified) {
  // create network object and parse vnnlib & onnx file.
  Network nnv;

  Logger::log(Logger::Level::INFO, "Parsing vnnlib: " + vnnlib_path + " ...");
  if (!nnv.read_vnnlib(vnnlib_path)) {
    Logger::log(Logger::Level::ERROR,
                "Could not parse vnnlib file " + vnnlib_path);
    return;
  }
  Logger::log(Logger::Level::INFO, "Parsing vnnlib is done.");
  Logger::log(Logger::Level::INFO, "Parsing onnx: " + onnx_path + " ...");
  if (!nnv.read_onnx(onnx_path)) {
    Logger::log(Logger::Level::ERROR, "Could not parse onnx file " + onnx_path);
    return;
  }
  Logger::log(Logger::Level::INFO, "Pasring onnx is done.");

  // Execute the propagation method.
  if (std::strcmp(propagation_method.c_str(), "ibp") == 0) {
    NaiveIntervalPropagation ibp;
    if (ibp.execute(nnv)) {
      Logger::log(Logger::Level::INFO,
                  "The naive interval propagation result is UNSAT");
      num_verified++;
    } else {
      Logger::log(Logger::Level::WARN,
                  "The naive interval propagation result is not able to have a "
                  "conclusive verification result.");
    }
  } else if (std::strcmp(propagation_method.c_str(), "sip") == 0) {
    SymbolicIntervalPropagation sip;
    if (sip.execute(nnv)) {
      Logger::log(Logger::Level::INFO,
                  "The symbolic interval propagation result is UNSAT");
      num_verified++;
    } else {
      Logger::log(
          Logger::Level::WARN,
          "The symbolic interval propagation result is not able to have a "
          "conclusive verification result.");
    }
  } else if (std::strcmp(propagation_method.c_str(), "deeppoly") == 0) {
    DeepPolyPropagation deeppoly;
    if (deeppoly.execute(nnv)) {
      Logger::log(Logger::Level::INFO,
                  "The deeppoly propagation result is UNSAT");
      num_verified++;
    } else {
      Logger::log(Logger::Level::WARN,
                  "The deeppoly propagation result is not able to have a "
                  "conclusive verification result.");
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
    }
  } else if (argc == 4) {
    verify(argv[1], argv[2], argv[3], num_verified);
  } else {
    Logger::log(Logger::Level::ERROR, "Unknown configuration.");
  }
  Logger::log(Logger::Level::INFO,
              "Number of verified: " + std::to_string(num_verified));

  return 0;
}
