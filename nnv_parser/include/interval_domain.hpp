#ifndef INTERVAL_DOMAIN_HPP
#define INTERVAL_DOMAIN_HPP

#include "abstract_domain.hpp"

class IntervalDomain : public AbstractDomain {
public:
  void subtraction_layer_transformer(const Layer &from_layer,
                                     Layer &to_layer) override {
    // TODO:
    return;
  }
  void flatten_layer_transformer(const Layer &from_layer,
                                 Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      from_layer.neurons[i] = to_layer.neurons[i];
    }
    return;
  }
  void relu_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds.setLb(
          std::max(0.0, from_layer.neurons[i].bounds.getLb()));
      to_layer.neurons[i].bounds.setUb(
          std::max(0.0, from_layer.neurons[i].bounds.getUb()));
    }
    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,
                                Layer &to_layer) override {
    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      double bias = from_layer.biases[i];
      to_layer.neurons[i].bounds.setLb(from_layer.neurons[i].bounds.getLb() +
                                       bias);
      to_layer.neurons[i].bounds.setUb(from_layer.neurons[i].bounds.getUb() +
                                       bias);
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    // TODO:
    return;
  }
};

#endif // INTERVAL_DOMAIN_HPP
