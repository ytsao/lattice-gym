#ifndef SYMBOLIC_INTERVAL_PROPAGATION_HPP
#define SYMBOLIC_INTERVAL_PROPAGATION_HPP

#include "propagation.hpp"
#include "symbolic_interval_domain.hpp"
#include <iostream>

class SymbolicIntervalPropagation : public Propagation {
public:
  SymbolicIntervalDomain a;

  bool execute(Network &nnv) override {
    std::cout << "Executing Symbolic Interval Propagation..." << std::endl;

    create_auxiliary_layer(nnv);

    for (size_t layer_idx = 0; layer_idx < nnv.layers.size(); ++layer_idx) {
      std::cout << "Processing Layer " << layer_idx << " ... " << std::endl;
      if (nnv.layers[layer_idx].type == LayerType::Sub) {
        a.subtraction_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Flatten) {
        a.flatten_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Relu) {
        a.relu_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      } else if (nnv.layers[layer_idx].type == LayerType::MatMul) {
        a.matmul_layer_transformer(nnv.layers[layer_idx - 1],
                                   nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      } else if (nnv.layers[layer_idx].type == LayerType::Add) {
        a.add_layer_transformer(nnv.layers[layer_idx - 1],
                                nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      } else if (nnv.layers[layer_idx].type == LayerType::Gemm) {
        a.gemm_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      }
    }

    return nnv.spec.check(nnv.layers.back());
  }
};

#endif // SYMBOLIC_INTERVAL_PROPAGATION_HPP
