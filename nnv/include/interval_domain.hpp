#ifndef INTERVAL_DOMAIN_HPP
#define INTERVAL_DOMAIN_HPP

#include "abstract_domain.hpp"

class IntervalDomain : public AbstractDomain {
public:
  void gamma(Network &nnv, size_t layer_idx) override { return; }

  void first_layer_transformer(Layer &current_layer) override { return; }

  void subtraction_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    to_layer.neurons = from_layer.neurons;
    to_layer.layer_size = from_layer.layer_size;
    to_layer.biases = from_layer.biases;
    to_layer.lower_biases = from_layer.lower_biases;
    to_layer.upper_biases = from_layer.upper_biases;

    return;
  }

  void division_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    to_layer.neurons = from_layer.neurons;
    to_layer.layer_size = from_layer.layer_size;
    to_layer.biases = from_layer.biases;
    to_layer.lower_biases = from_layer.lower_biases;
    to_layer.upper_biases = from_layer.upper_biases;

    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      int dim = i / (to_layer.input_height * to_layer.input_width);
      to_layer.neurons[i].bounds = (from_layer.neurons[i].bounds - from_layer.sub_values[dim]) / to_layer.div_values[dim];
    }

    return;
  }

  void flatten_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    to_layer.neurons = from_layer.neurons;
    to_layer.layer_size = from_layer.layer_size;
    to_layer.biases = from_layer.biases;
    to_layer.lower_biases = from_layer.lower_biases;
    to_layer.upper_biases = from_layer.upper_biases;

    return;
  }

  void relu_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds.setLb(std::max(0.0, from_layer.neurons[i].bounds.getLb()));
      to_layer.neurons[i].bounds.setUb(std::max(0.0, from_layer.neurons[i].bounds.getUb()));
    }

    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    for (size_t j = 0; j < to_layer.layer_size; ++j) {
      for (size_t i = 0; i < from_layer.layer_size; ++i) {
        to_layer.neurons[j].bounds = to_layer.neurons[j].bounds + from_layer.neurons[i].bounds * to_layer.weights[i][j];
      }
    }

    return;
  }
  void add_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds = from_layer.neurons[i].bounds + to_layer.biases[i];
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      for (size_t j = 0; j < from_layer.layer_size; ++j) {
        to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + from_layer.neurons[j].bounds * to_layer.weights[i][j];
      }

      double bias = to_layer.biases[i];
      to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + bias;
    }

    return;
  }

  void convolutional_layer_transformer(const Layer &from_layer,Layer &to_layer) override {
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      for (size_t j = 0; j < from_layer.layer_size; ++j) {
        to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + from_layer.neurons[j].bounds * to_layer.weights[i][j];
      }

      double bias = to_layer.biases[i];
      to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + bias;
    }

    return;
  }
};

#endif // INTERVAL_DOMAIN_HPP
