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
      std::cout << "Processing Layer " << layer_idx << " ... " << std::endl;
      if (nnv.layers[layer_idx].type == LayerType::Sub) {
        a.subtraction_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Flatten) {
        std::cout << "starting flatten layer transformation\n";
        a.flatten_layer_transformer(nnv.layers[layer_idx]);
        std::cout << "finished flatten layer transformation\n";
      } else if (nnv.layers[layer_idx].type == LayerType::Relu) {
        std::cout << "starting relu layer transformation\n";
        a.relu_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
        std::cout << "finished relu layer transformation\n";
      } else if (nnv.layers[layer_idx].type == LayerType::MatMul) {
        std::cout << "starting matmul layer transformation\n";
        a.matmul_layer_transformer(nnv.layers[layer_idx - 1],
                                   nnv.layers[layer_idx]);
        std::cout << "finished matmul layer transformation\n";
      } else if (nnv.layers[layer_idx].type == LayerType::Add) {
        std::cout << "starting add layer transformation\n";
        a.add_layer_transformer(nnv.layers[layer_idx - 1],
                                nnv.layers[layer_idx]);
        std::cout << "finished add layer transformation\n";
      } else if (nnv.layers[layer_idx].type == LayerType::Gemm) {
        std::cout << "starting gemm layer transformation\n";
        a.gemm_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
        std::cout << "finished gemm layer transformation\n";
      } else {
        std::cout << "Unknown layer type!" << std::endl;
      }
    }

    return nnv.spec.check(nnv.layers.back());
  }
};

#endif // NAIVE_INTERVAL_PROPAGATION_HPP
