#ifndef NAIVE_INTERVAL_PROPAGATION_HPP
#define NAIVE_INTERVAL_PROPAGATION_HPP

#include "interval_domain.hpp"
#include "propagation.hpp"
#include <iostream>

class NaiveIntervalPropagation : public Propagation {
public:
  IntervalDomain a;

  bool execute(Network &nnv) override {
    std::cout << "Executing Naive Interval Propagation..." << std::endl;

    create_auxiliary_layer(nnv);

    for (size_t layer_idx = 0; layer_idx < nnv.layers.size(); ++layer_idx) {
      if (nnv.layers[layer_idx].type == LayerType::Sub) {
        a.subtraction_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Flatten) {
        a.flatten_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Relu) {
        a.relu_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::MatMul) {
        a.matmul_layer_transformer(nnv.layers[layer_idx - 1],
                                   nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Add) {
        a.add_layer_transformer(nnv.layers[layer_idx - 1],
                                nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Gemm) {
        a.gemm_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
      } else {
        std::cout << "Unknown layer type!" << std::endl;
      }
    }

    return nnv.spec.check(nnv.layers.back());
  }
};

#endif // NAIVE_INTERVAL_PROPAGATION_HPP
