#ifndef SYMBOLIC_INTERVAL_DOMAIN_HPP
#define SYMBOLIC_INTERVAL_DOMAIN_HPP

#include "abstract_domain.hpp"

class SymbolicIntervalDomain : public AbstractDomain {
public:
  void subtraction_layer_transformer(const Layer &from_layer,
                                     Layer &to_layer) override {
    return;
  }
  void flatten_layer_transformer(const Layer &from_layer,
                                 Layer &to_layer) override {
    return;
  }
  void relu_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,
                                Layer &to_layer) override {
    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override {
    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    return;
  }
};

#endif // SYMBOLIC_INTERVAL_DOMAIN_HPP
