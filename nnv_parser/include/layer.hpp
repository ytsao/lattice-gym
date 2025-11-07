#ifndef LAYER_HPP
#define LAYER_HPP

#include "neuron.hpp"
#include <vector>

enum class LayerType {
  First,
  Sub,
  Div,
  Constant,
  MatMul,
  Add,
  Gemm,
  Conv,
  Flatten,
  Relu,
};

struct Layer {
public:
  using tensor4d = std::vector<std::vector<std::vector<std::vector<float>>>>;

  LayerType type;

  // for colorful images, we have 3 values
  std::vector<float> sub_values;
  std::vector<float> div_values;

  // for convolutional layer
  size_t pads;
  size_t strides;
  size_t dilation;
  size_t group;
  size_t kernel_width;
  size_t kernel_height;
  size_t conv_output_width;
  size_t conv_output_height;

  std::vector<std::vector<float>> weights;
  tensor4d convolution_weights;

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
