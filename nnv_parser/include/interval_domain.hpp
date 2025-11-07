#ifndef INTERVAL_DOMAIN_HPP
#define INTERVAL_DOMAIN_HPP

#include "abstract_domain.hpp"

class IntervalDomain : public AbstractDomain
{
public:
  void gamma(Network &nnv, size_t layer_idx) override { return; }

  void first_layer_transformer(Layer &current_layer) override { return; }

  void subtraction_layer_transformer(const Layer &from_layer,
                                     Layer &to_layer) override
  {
    // for (size_t i = 0; i < from_layer.sub_values.size(); ++i) {
    //   to_layer.sub_values.push_back(from_layer.sub_values[i]);
    // }

    to_layer.neurons = from_layer.neurons;
    to_layer.layer_size = from_layer.layer_size;
    to_layer.biases = from_layer.biases;
    to_layer.lower_biases = from_layer.lower_biases;
    to_layer.upper_biases = from_layer.upper_biases;

    return;
  }

  void division_layer_transformer(const Layer &from_layer,
                                  Layer &to_layer) override
  {
    to_layer.neurons = from_layer.neurons;
    to_layer.layer_size = from_layer.layer_size;
    to_layer.biases = from_layer.biases;
    to_layer.lower_biases = from_layer.lower_biases;
    to_layer.upper_biases = from_layer.upper_biases;

    // Normalization
    for (size_t dim = 0; dim < from_layer.sub_values.size(); ++dim)
    {
      for (size_t i = 0; i < to_layer.layer_size; ++i)
      {
        to_layer.neurons[i].bounds =
            (from_layer.neurons[i].bounds - from_layer.sub_values[dim]) /
            to_layer.div_values[dim];
      }
    }

    return;
  }

  void flatten_layer_transformer(const Layer &from_layer,
                                 Layer &to_layer) override
  {
    to_layer.neurons = from_layer.neurons;
    to_layer.layer_size = from_layer.layer_size;
    to_layer.biases = from_layer.biases;
    to_layer.lower_biases = from_layer.lower_biases;
    to_layer.upper_biases = from_layer.upper_biases;

    return;
  }

  void relu_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override
  {
    for (size_t i = 0; i < from_layer.layer_size; ++i)
    {
      to_layer.neurons[i].bounds.setLb(
          std::max(0.0, from_layer.neurons[i].bounds.getLb()));
      to_layer.neurons[i].bounds.setUb(
          std::max(0.0, from_layer.neurons[i].bounds.getUb()));
    }

    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,
                                Layer &to_layer) override
  {
    for (size_t j = 0; j < to_layer.layer_size; ++j)
    {
      for (size_t i = 0; i < from_layer.layer_size; ++i)
      {
        to_layer.neurons[j].bounds =
            to_layer.neurons[j].bounds +
            from_layer.neurons[i].bounds * to_layer.weights[i][j];
      }
    }

    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override
  {
    for (size_t i = 0; i < from_layer.layer_size; ++i)
    {
      to_layer.neurons[i].bounds =
          from_layer.neurons[i].bounds + to_layer.biases[i];
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override
  {
    for (size_t i = 0; i < to_layer.layer_size; ++i)
    {
      for (size_t j = 0; j < from_layer.layer_size; ++j)
      {
        to_layer.neurons[i].bounds =
            to_layer.neurons[i].bounds +
            from_layer.neurons[j].bounds * to_layer.weights[i][j];
      }
      double bias = to_layer.biases[i];
      to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + bias;
    }

    return;
  }

  void convolutional_layer_transformer(const Layer &from_layer,
                                       Layer &to_layer) override
  {
    // for (size_t i = 0; i < to_layer.layer_size; ++i) {
    //   for (size_t j = 0; j < from_layer.layer_size; ++j) {
    //     to_layer.neurons[i].bounds =
    //         to_layer.neurons[i].bounds +
    //         from_layer.neurons[j].bounds * to_layer.weights[i][j];
    //   }
    //   double bias = to_layer.biases[i];
    //   to_layer.neurons[i].bounds = to_layer.neurons[i].bounds + bias;
    // }

    // FIX: I should use 4D tensor directly, instead of using tranformed 2d
    // tensor.
    // Also, consider its biase.
    Logger::log(Logger::Level::DEBUG,
                "Con input channels = " +
                    std::to_string(to_layer.input_channels));
    Logger::log(Logger::Level::DEBUG,
                "Con input height = " +
                    std::to_string(to_layer.conv_input_height));
    Logger::log(Logger::Level::DEBUG,
                "Con input width = " +
                    std::to_string(to_layer.conv_input_width));
    Logger::log(Logger::Level::DEBUG,
                "Con output channels = " +
                    std::to_string(to_layer.output_channels));
    Logger::log(Logger::Level::DEBUG,
                "Con output height = " +
                    std::to_string(to_layer.conv_output_height));
    Logger::log(Logger::Level::DEBUG,
                "Con output width = " +
                    std::to_string(to_layer.conv_output_width));
    for (size_t i = 0; i < to_layer.conv_output_height; ++i)
    {
      for (size_t j = 0; j < to_layer.conv_output_width; ++j)
      {
        for (size_t k = 0; k < to_layer.output_channels; ++k)
        {
          size_t row =
              k * (to_layer.conv_output_height * to_layer.conv_output_width) +
              i * to_layer.conv_output_width + j;

          for (size_t di = 0; di < to_layer.kernel_height; ++di)
          {
            for (size_t dj = 0; dj < to_layer.kernel_width; ++dj)
            {
              for (size_t dk = 0; dk < to_layer.input_channels; ++dk)
              {
                int hIndex =
                    static_cast<int>(to_layer.strides * i + di - to_layer.pads);
                int wIndex =
                    static_cast<int>(to_layer.strides * j + dj - to_layer.pads);

                if (wIndex >= 0 && wIndex < (int)to_layer.conv_input_width &&
                    hIndex >= 0 && hIndex < (int)to_layer.conv_input_height)
                {
                  size_t col = dk * (to_layer.conv_input_height *
                                     to_layer.conv_input_width) +
                               hIndex * to_layer.conv_input_width + wIndex;
                  to_layer.neurons[row].bounds =
                      to_layer.neurons[row].bounds +
                      from_layer.neurons[col].bounds *
                          to_layer
                              .convolution_weights[k][dk][di]
                                                  [dj]; // same index pattern as
                                                        // Marabou
                }
              }
            }
          }
          to_layer.neurons[row].bounds =
              to_layer.neurons[row].bounds + to_layer.biases[k];
        }
      }
    }

    return;
  }
};

#endif // INTERVAL_DOMAIN_HPP
