#ifndef ABSTRACT_DOMAIN_HPP
#define ABSTRACT_DOMAIN_HPP

#include "layer.hpp"

class AbstractDomain {
public:
  virtual ~AbstractDomain() = default;

  virtual void subtraction_layer_transformer(const Layer &from_layer,
                                             Layer &to_layer) = 0;
  virtual void flatten_layer_transformer(const Layer &from_layer,
                                         Layer &to_layer) = 0;
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
