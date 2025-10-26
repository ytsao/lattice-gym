#ifndef DEEPPOLY_PROPAGATION_HPP
#define DEEPPOLY_PROPAGATION_HPP

#include "deeppoly_domain.hpp"
#include "propagation.hpp"
#include <iostream>

class DeepPolyPropagation : public Propagation {
public:
  DeepPolyDomain a;

  bool execute(Network &nnv) override {
    std::cout << "Executing Symbolic DeepPoly Propagation..." << std::endl;

    for (size_t layer_idx = 0; layer_idx < nnv.layers.size(); ++layer_idx) {
      if (nnv.layers[layer_idx].type == LayerType::Sub) {
        a.subtraction_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Flatten) {
        a.flatten_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Relu) {
        a.relu_layer_transformer(nnv.layers[layer_idx],
                                 nnv.layers[layer_idx + 1]);
      } else if (nnv.layers[layer_idx].type == LayerType::MatMul) {
        a.matmul_layer_transformer(nnv.layers[layer_idx],
                                   nnv.layers[layer_idx + 1]);
      } else if (nnv.layers[layer_idx].type == LayerType::Add) {
        a.add_layer_transformer(nnv.layers[layer_idx - 1],
                                nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Gemm) {
        a.gemm_layer_transformer(nnv.layers[layer_idx],
                                 nnv.layers[layer_idx + 1]);
      }
    }

    return false;
  }
};

#endif // DEEPPOLY_PROPAGATION_HPP
