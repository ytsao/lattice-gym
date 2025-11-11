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
  using tensor1d = std::vector<float>;
  using tensor2d = std::vector<std::vector<float>>;
  using tensor4d = std::vector<std::vector<std::vector<std::vector<float>>>>;

  LayerType type;

  // for colorful images, we have 3 values
  tensor1d sub_values;
  tensor1d div_values;

  // for convolutional layer
  size_t input_channels;
  size_t output_channels;
  size_t input_height;
  size_t input_width;
  size_t pads;
  size_t strides;
  size_t dilation;
  size_t group;
  size_t kernel_width;
  size_t kernel_height;
  size_t conv_input_width;
  size_t conv_input_height;
  size_t conv_output_width;
  size_t conv_output_height;

  tensor2d weights;
  tensor4d convolution_weights;

  tensor1d biases;
  tensor1d lower_biases;
  tensor1d upper_biases;

  std::vector<Neuron> neurons;
  size_t layer_size = 0;

  // DeepPoly expressions
  std::vector<std::vector<double>> deeppoly_lower_expressions;
  std::vector<std::vector<double>> deeppoly_upper_expressions;
};

#endif // LAYER_HPP
