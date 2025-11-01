#ifndef PROPAGATION_HPP
#define PROPAGATION_HPP

#include "log.hpp"
#include "network.hpp"

class Propagation {
public:
  virtual ~Propagation() = default;

  virtual bool execute(Network &nnv) = 0;

protected:
  void create_auxiliary_layer(Network &nnv) {
    Layer auxiliary_layer;
    auxiliary_layer.type = LayerType::Gemm;

    if (nnv.spec.A.empty()) {
      // For prop_1.vnnlib
      for (const auto &output_var : nnv.spec.variables) {
        if (output_var.first.substr(0, 1) == "Y") {
          double spec_lb = output_var.second.bounds.getLb();
          double spec_ub = output_var.second.bounds.getUb();
          if (spec_lb != 0) {
            auxiliary_layer.layer_size++;
            Neuron neuron;
            neuron.setId(output_var.second.id);
            auxiliary_layer.neurons.push_back(neuron);
            auxiliary_layer.biases.push_back(spec_lb);
            auxiliary_layer.lower_biases.push_back(spec_lb);
            auxiliary_layer.upper_biases.push_back(spec_lb);
          }
          if (spec_ub != 0) {
            auxiliary_layer.layer_size++;
            Neuron neuron;
            neuron.setId(output_var.second.id);
            auxiliary_layer.neurons.push_back(neuron);
            auxiliary_layer.biases.push_back(-spec_ub);
            auxiliary_layer.lower_biases.push_back(-spec_ub);
            auxiliary_layer.upper_biases.push_back(-spec_ub);
          }
        }
      }

      // create weights link
      std::vector<float> zero_weights(nnv.output_size, 0.0f);
      for (size_t i = 0; i < auxiliary_layer.layer_size; ++i) {
        auxiliary_layer.weights.push_back(zero_weights);
        if (auxiliary_layer.lower_biases[i] > 0)
          auxiliary_layer.weights[i][i] = -1.0;
        else if (auxiliary_layer.lower_biases[i] < 0)
          auxiliary_layer.weights[i][i] = 1.0;
      }

      nnv.layers.push_back(auxiliary_layer);

      return;
    }

    // std::cout << "  Number of OR conditions: " << nnv.spec.A.size()
    //           << std::endl;
    // std::cout << "  Number of AND conditions per OR condition: "
    //           << nnv.spec.A[0].size() << std::endl;
    auxiliary_layer.layer_size =
        nnv.spec.A.size() *
        nnv.spec.A[0]
            .size(); // number of OR conditions * number of AND conditions.

    // std::cout << "  Auxiliary layer size: " << auxiliary_layer.layer_size
    //           << std::endl;

    // Initialize the size of weights matrix.
    std::vector<float> zero_weights(nnv.output_size, 0.0f);
    for (size_t j = 0; j < auxiliary_layer.layer_size; ++j) {
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

    // Create a set of new neurons and its lower & upper biases.
    for (size_t i = 0; i < auxiliary_layer.layer_size; ++i) {
      Neuron neuron;
      neuron.setId(i);
      neuron.setLayerId(nnv.layers.size());
      auxiliary_layer.neurons.push_back(neuron);
      auxiliary_layer.biases.push_back(0.0f);
      auxiliary_layer.lower_biases.push_back(0.0f);
      auxiliary_layer.upper_biases.push_back(0.0f);
    }

    for (size_t i = 0; i < nnv.spec.A.size(); ++i) {
      for (size_t j = 0; j < nnv.spec.A[i].size(); ++j) {
        char relation = nnv.spec.relations[i][j];
        for (size_t k = 0; k < nnv.spec.A[i][j].size(); ++k) {
          double coef = nnv.spec.A[i][j][k];
          auxiliary_layer.weights[i * nnv.spec.A[i].size() + j][k] = coef;
        }
      }
    }
    nnv.layers.push_back(auxiliary_layer);

    return;
  }
};

#endif // PROPAGATION_HPP
