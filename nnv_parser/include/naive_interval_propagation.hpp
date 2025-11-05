#ifndef NAIVE_INTERVAL_PROPAGATION_HPP
#define NAIVE_INTERVAL_PROPAGATION_HPP

#include "interval_domain.hpp"
#include "log.hpp"
#include "propagation.hpp"

class NaiveIntervalPropagation : public Propagation {
public:
  IntervalDomain a;

  bool execute(Network &nnv) override {

    Logger::log(Logger::Level::INFO,
                "Executing naive interval propagation ...");

    create_auxiliary_layer(nnv);

    for (size_t layer_idx = 0; layer_idx < nnv.layers.size(); ++layer_idx) {
      if (nnv.layers[layer_idx].type == LayerType::Sub) {
        a.subtraction_layer_transformer(nnv.layers[layer_idx],
                                        nnv.layers[layer_idx + 1]);
      } else if (nnv.layers[layer_idx].type == LayerType::Div) {
        a.division_layer_transformer(nnv.layers[layer_idx],
                                     nnv.layers[layer_idx + 1]);
      } else if (nnv.layers[layer_idx].type == LayerType::Flatten) {
        // FIX: if flatten is not the first layer, we should change the
        // implementation of its transformer.
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
      } else if (nnv.layers[layer_idx].type == LayerType::Conv) {
        // FIX: in some cases, convolutional layer could be the first layer in
        // the network. So that, we will not have layer_idx - 1.
        a.convolutional_layer_transformer(nnv.layers[layer_idx - 1],
                                          nnv.layers[layer_idx]);
      } else {
        Logger::log(Logger::ERROR, "Unknown layer type!");
        return false;
      }
    }

    return nnv.spec.check(nnv.layers.back());
  }
};

#endif // NAIVE_INTERVAL_PROPAGATION_HPP
