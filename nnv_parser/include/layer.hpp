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
  // Neuron *neurons;
  std::vector<Neuron> neurons;
  size_t layer_size = 0;
};

#endif // LAYER_HPP
