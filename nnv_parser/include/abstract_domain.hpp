#ifndef ABSTRACT_DOMAIN_HPP
#define ABSTRACT_DOMAIN_HPP

#include "network.hpp"

class AbstractDomain {
public:
  virtual ~AbstractDomain() = default;

  /*
   * It is the concretization function that maps the abstract element to the
   * interval bounds by its dependency layer in the network.
   * */
  virtual void gamma(Network &nnv, const size_t layer_idx) { return; }

  virtual void subtraction_layer_transformer(const Layer &current_layer) {
    return;
  };
  virtual void flatten_layer_transformer(const Layer &current_layer) {
    return;
  };
  virtual void flatten_layer_transformer(Layer &current_layer) { return; };
  virtual void relu_layer_transformer(const Layer &from_layer,
                                      Layer &to_layer) = 0;
  virtual void matmul_layer_transformer(const Layer &from_layer,
                                        Layer &to_layer) = 0;
  virtual void add_layer_transformer(const Layer &from_layer,
                                     Layer &to_layer) = 0;
  virtual void gemm_layer_transformer(const Layer &from_layer,
                                      Layer &to_layer) = 0;
};

#endif // ABSTRACT_DOMAIN_HPP
