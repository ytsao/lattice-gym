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

  // Neuron *neurons;
  std::vector<Neuron> neurons;
  size_t layer_size = 0;

  // For indicating if the neuron has dependency from previous layers.
  // To do substitution, we can apply matrix multiplication recursively.
  //
  // If the values in a row are all zeros, it means that the neuron is
  // concretized, we should replace it as a constant by its corresponding
  // concrete interval.
  //
  // row := the variables in the current layer,
  // column := the variables in the previous layer.
  std::vector<std::vector<bool>> auxiliary_matrix;
};

#endif // LAYER_HPP
