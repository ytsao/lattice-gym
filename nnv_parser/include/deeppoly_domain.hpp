#ifndef DEEPPOLY_DOMAIN_HPP
#define DEEPPOLY_DOMAIN_HPP

#include "abstract_domain.hpp"

class DeepPolyDomain : public AbstractDomain {
public:
  void gamma(Network &nnv, size_t layer_idx) override { return; }

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

#endif // DEEPPOLY_DOMAIN_HPP
