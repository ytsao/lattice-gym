#ifndef LAYER_HPP
#define LAYER_HPP

#include "neuron.hpp"
#include <vector>

enum class LayerType {
  Sub,
  MatMul,
  Add,
  Gemm,
  Flatten,
  Relu,
};

struct Layer {
public:
  LayerType type;
  std::vector<std::vector<float>> weights;
  std::vector<float> biases;
  std::vector<float> lower_biases;
  std::vector<float> upper_biases;

  std::vector<Neuron> neurons;
  size_t layer_size = 0;

  // DeepPoly expressions
  std::vector<std::vector<double>> deeppoly_lower_expressions;
  std::vector<std::vector<double>> deeppoly_upper_expressions;
};

#endif // LAYER_HPP
