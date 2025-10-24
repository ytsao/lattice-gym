#ifndef INTERVAL_DOMAIN_HPP
#define INTERVAL_DOMAIN_HPP

#include "abstract_domain.hpp"

class IntervalDomain : public AbstractDomain {
public:
  void subtraction_layer_transformer(const Layer &current_layer) override {
    return;
  }
  void flatten_layer_transformer(const Layer &current_layer) override {
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
    for (size_t j = 0; j < to_layer.weights[0].size(); ++j) {
      for (size_t i = 0; i < to_layer.weights.size(); ++i) {
        to_layer.neurons[j].bounds =
            to_layer.neurons[j].bounds +
            from_layer.neurons[i].bounds * to_layer.weights[i][j];
      }
    }

    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds =
          from_layer.neurons[i].bounds + to_layer.biases[i];
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    for (size_t i = 0; i < to_layer.weights.size(); ++i) {
      for (size_t j = 0; j < to_layer.weights[i].size(); ++j) {
        to_layer.neurons[i].bounds =
            to_layer.neurons[i].bounds +
            from_layer.neurons[j].bounds * to_layer.weights[i][j];
      }
      double bias = to_layer.biases[i];
      to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + bias;
    }

    return;
  }
};

#endif // INTERVAL_DOMAIN_HPP
