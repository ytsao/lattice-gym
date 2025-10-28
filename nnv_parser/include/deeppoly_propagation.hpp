#ifndef DEEPPOLY_PROPAGATION_HPP
#define DEEPPOLY_PROPAGATION_HPP

#include "deeppoly_domain.hpp"
#include "propagation.hpp"
#include <iostream>

class DeepPolyPropagation : public Propagation {
public:
  DeepPolyDomain a;

  bool execute(Network &nnv) override {
    std::cout << "Executing Symbolic DeepPoly Propagation..." << std::endl;

    // TODO: create an additional layer to express the postconditions.
    // Ref: the implementation I did in Marabou.
    create_auxiliary_layer(nnv);

    for (size_t layer_idx = 0; layer_idx < nnv.layers.size(); ++layer_idx) {
      std::cout << "Processing Layer " << layer_idx << " ... " << std::endl;
      if (nnv.layers[layer_idx].type == LayerType::Sub) {
        a.subtraction_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Flatten) {
        a.flatten_layer_transformer(nnv.layers[layer_idx]);
      } else if (nnv.layers[layer_idx].type == LayerType::Relu) {
        a.relu_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      } else if (nnv.layers[layer_idx].type == LayerType::MatMul) {
        a.matmul_layer_transformer(nnv.layers[layer_idx - 1],
                                   nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      } else if (nnv.layers[layer_idx].type == LayerType::Add) {
        a.add_layer_transformer(nnv.layers[layer_idx - 1],
                                nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      } else if (nnv.layers[layer_idx].type == LayerType::Gemm) {
        a.gemm_layer_transformer(nnv.layers[layer_idx - 1],
                                 nnv.layers[layer_idx]);
        a.gamma(nnv, layer_idx);
      }
    }

    return true;
  }

private:
  void create_auxiliary_layer(Network &nnv) {
    std::cout << "  Creating auxiliary layer for postconditions..."
              << std::endl;

    if (nnv.spec.A.empty()) {
      // For prop_1.vnnlib
      std::cout << "  No postconditions specified. Skipping auxiliary layer."
                << std::endl;
      return;
    }

    Layer auxiliary_layer;
    auxiliary_layer.type = LayerType::MatMul;

    std::cout << "  Number of OR conditions: " << nnv.spec.A.size()
              << std::endl;
    std::cout << "  Number of AND conditions per OR condition: "
              << nnv.spec.A[0].size() << std::endl;
    auxiliary_layer.layer_size =
        nnv.spec.A.size() *
        nnv.spec.A[0]
            .size(); // number of OR conditions * number of AND conditions.

    std::cout << "  Auxiliary layer size: " << auxiliary_layer.layer_size
              << std::endl;

    // Initialize the size of weights matrix.
    std::vector<float> zero_weights(auxiliary_layer.layer_size, 0.0f);
    for (size_t j = 0; j < nnv.output_size; ++j) {
      auxiliary_layer.weights.push_back(zero_weights);
    }

    // Initialize deeppoly expressions for each neuron in the auxiliary_layer
    auxiliary_layer.deeppoly_lower_expressions =
        std::vector<std::vector<double>>(
            auxiliary_layer.layer_size,
            std::vector<double>(nnv.layers.back().layer_size, 0.0));
    auxiliary_layer.deeppoly_upper_expressions =
        std::vector<std::vector<double>>(
            auxiliary_layer.layer_size,
            std::vector<double>(nnv.layers.back().layer_size, 0.0));

    // Create zero biases for auxiliary layer
    std::vector<float> zero_biases(auxiliary_layer.layer_size, 0.0);
    auxiliary_layer.lower_biases = zero_biases;
    auxiliary_layer.upper_biases = zero_biases;

    // Create a set of new neurons
    for (size_t i = 0; i < auxiliary_layer.layer_size; ++i) {
      Neuron neuron;
      neuron.setId(i);
      neuron.setLayerId(nnv.layers.size());
      auxiliary_layer.neurons.push_back(neuron);
    }

    for (size_t i = 0; i < nnv.spec.A.size(); ++i) {
      for (size_t j = 0; j < nnv.spec.A[i].size(); ++j) {
        for (size_t k = 0; k < nnv.spec.A[i][j].size(); ++k) {
          double coef = nnv.spec.A[i][j][k];
          std::cout << "Coef: " << coef << " for neuron index "
                    << (i * nnv.spec.A[i].size() + j) << " and output index "
                    << k << std::endl;
          auxiliary_layer.weights[i * nnv.spec.A[i].size() + j][k] = coef;

          auxiliary_layer
              .deeppoly_lower_expressions[i * nnv.spec.A[i].size() + j][k] =
              coef;
          auxiliary_layer
              .deeppoly_upper_expressions[i * nnv.spec.A[i].size() + j][k] =
              coef;
        }
      }
    }

    nnv.layers.push_back(auxiliary_layer);

    std::cout << "Finished creating auxiliary layer." << std::endl;

    return;
  }
};

#endif // DEEPPOLY_PROPAGATION_HPP
