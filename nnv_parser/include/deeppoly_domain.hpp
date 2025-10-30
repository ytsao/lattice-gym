#ifndef DEEPPOLY_DOMAIN_HPP
#define DEEPPOLY_DOMAIN_HPP

#include "abstract_domain.hpp"

class DeepPolyDomain : public AbstractDomain {
public:
  void gamma(Network &nnv, size_t layer_idx) override {
    // Applying back-substitution to compute concrete bounds from DeepPoly
    // expressions with respect to the input layer.
    std::cout << "  Applying back-substitution for Layer " << layer_idx
              << " ..." << std::endl;
    back_substitution(nnv, layer_idx);
    std::cout << " Finished back-substitution for Layer " << layer_idx << "."
              << std::endl;

    return;
  }

  void flatten_layer_transformer(Layer &current_layer) override {
    std::vector<double> zero_expression(current_layer.layer_size, 0.0);
    for (size_t i = 0; i < current_layer.layer_size; ++i) {
      current_layer.deeppoly_lower_expressions.push_back(zero_expression);
      current_layer.deeppoly_upper_expressions.push_back(zero_expression);

      // x_i = x_j;
      current_layer.deeppoly_lower_expressions[i][i] = 1;
      current_layer.deeppoly_upper_expressions[i][i] = 1;
    }

    return;
  }

  void relu_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    std::cout << "We're in relu_layer_transformer()" << std::endl;
    create_deeppoly_expressions(from_layer, to_layer);

    // initialize biases
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      to_layer.lower_biases.push_back(0.0);
      to_layer.upper_biases.push_back(0.0);
    }

    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      double lb = from_layer.neurons[i].bounds.getLb();
      double ub = from_layer.neurons[i].bounds.getUb();

      if (lb >= 0.0) {
        to_layer.neurons[i].bounds.setLb(lb);
        to_layer.neurons[i].bounds.setUb(ub);

        // x_i = x_j;
        to_layer.deeppoly_lower_expressions[i][i] = 1;
        to_layer.deeppoly_upper_expressions[i][i] = 1;
      } else if (ub <= 0.0) {
        to_layer.neurons[i].bounds.setLb(0.0);
        to_layer.neurons[i].bounds.setUb(0.0);
      } else if (lb < 0.0 && ub > 0.0) {
        to_layer.neurons[i].lambda = std::abs(lb) <= ub ? true : false;

        to_layer.deeppoly_lower_expressions[i][i] = to_layer.neurons[i].lambda;
        to_layer.deeppoly_upper_expressions[i][i] = ub / (ub - lb);
        to_layer.upper_biases[i] += (ub * (-lb)) / (ub - lb);
      }
    }

    std::cout << "Finished relu_layer_transformer()" << std::endl;

    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,
                                Layer &to_layer) override {
    std::cout << "We're in matmul_layer_transformer()" << std::endl;
    create_deeppoly_expressions(from_layer, to_layer);

    if (to_layer.lower_biases.empty() && to_layer.upper_biases.empty()) {
      for (size_t i = 0; i < to_layer.layer_size; ++i) {
        to_layer.lower_biases.push_back(0.0);
        to_layer.upper_biases.push_back(0.0);
      }
    }

    for (size_t j = 0; j < to_layer.layer_size; ++j) {
      for (size_t i = 0; i < from_layer.layer_size; ++i) {
        double weights = to_layer.weights[i][j];

        to_layer.deeppoly_lower_expressions[j][i] += weights;
        to_layer.deeppoly_upper_expressions[j][i] += weights;
      }
    }

    std::cout << "Finished matmul_layer_transformer()" << std::endl;

    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override {
    create_deeppoly_expressions(from_layer, to_layer);

    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.deeppoly_lower_expressions[i][i] = 1;
      to_layer.deeppoly_upper_expressions[i][i] = 1;

      // update the biases
      to_layer.lower_biases[i] += from_layer.lower_biases[i];
      to_layer.upper_biases[i] += from_layer.upper_biases[i];
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    std::cout << "We're in gemm_layer_transformer()" << std::endl;
    create_deeppoly_expressions(from_layer, to_layer);

    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      for (size_t j = 0; j < from_layer.layer_size; ++j) {
        double weights = to_layer.weights[i][j];

        to_layer.deeppoly_lower_expressions[i][j] += weights;
        to_layer.deeppoly_upper_expressions[i][j] += weights;
      }

      double bias = to_layer.biases[i];
      to_layer.lower_biases[i] += bias;
      to_layer.upper_biases[i] += bias;
    }

    std::cout << "Finished gemm_layer_transformer()" << std::endl;

    return;
  }

private:
  void create_deeppoly_expressions(const Layer &from_layer, Layer &to_layer) {
    // Initialize deeppoly expressions for each neuron in the to_layer
    // The size of expressions in to_layer depends on the size of from_layer.
    std::vector<double> zero_expression(from_layer.layer_size, 0.0);
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      to_layer.deeppoly_lower_expressions.push_back(zero_expression);
      to_layer.deeppoly_upper_expressions.push_back(zero_expression);
    }

    return;
  }

  void back_substitution(Network &nnv, size_t start_layer_idx) {
    // input and the first hidden layer depend on input layer directly.
    // Copy the deeppoly expressions from the start_layer_idx
    // Create additional memory to store, instead of using reference or
    // std::move;
    // Since we don't want to modify the original deeppoly expressions at
    // start_layer_idx layer.
    std::vector<std::vector<double>> tmp_lower_expressions =
        nnv.layers[start_layer_idx].deeppoly_lower_expressions;
    std::vector<std::vector<double>> tmp_upper_expressions =
        nnv.layers[start_layer_idx].deeppoly_upper_expressions;
    std::vector<std::vector<double>> resulting_lower_expressions;
    std::vector<std::vector<double>> resulting_upper_expressions;

    // for the layers after the first hidden layer.
    std::vector<float> lower_biases(nnv.layers[start_layer_idx].layer_size,
                                    0.0);
    std::vector<float> upper_biases(nnv.layers[start_layer_idx].layer_size,
                                    0.0);
    for (size_t layer_idx = start_layer_idx; layer_idx > 0; --layer_idx) {
      // We do matrix multiplication from the start_layer_idx down to the input
      // layer.

      // Initialize the size of resulting expressions
      size_t sizeOfExpression =
          nnv.layers[layer_idx - 1].deeppoly_lower_expressions[0].size();
      std::cout << "sizeOfExpression = " << sizeOfExpression << std::endl;
      std::vector<double> zero_expression(sizeOfExpression, 0.0);
      for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
        resulting_lower_expressions.push_back(zero_expression);
        resulting_upper_expressions.push_back(zero_expression);
      }

      for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
        for (size_t j = 0; j < nnv.layers[layer_idx - 1].layer_size; ++j) {
          for (size_t k = 0; k < sizeOfExpression; ++k) {
            // TODO: we might need to consider the sign of each coefficient such
            // that to substitute with corresponding lower or upper expressions.

            // Update lower expression
            resulting_lower_expressions[i][k] +=
                tmp_lower_expressions[i][j] *
                nnv.layers[layer_idx - 1].deeppoly_lower_expressions[j][k];

            // Update upper expression
            resulting_upper_expressions[i][k] +=
                tmp_upper_expressions[i][j] *
                nnv.layers[layer_idx - 1].deeppoly_upper_expressions[j][k];
          }
        }
        lower_biases[i] += nnv.layers[layer_idx].lower_biases[i];
        upper_biases[i] += nnv.layers[layer_idx].upper_biases[i];
      }

      tmp_lower_expressions = std::move(resulting_lower_expressions);
      tmp_upper_expressions = std::move(resulting_upper_expressions);

      resulting_lower_expressions.clear();
      resulting_upper_expressions.clear();
    }

    std::cout << "After substitution, computing concrete bounds..."
              << std::endl;

    for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
      // compute concrete bounds;
      double lb = 0.0;
      double ub = 0.0;
      for (size_t j = 0; j < nnv.input_size; ++j) {
        // lb
        if (tmp_lower_expressions[i][j] >= 0) {
          lb += tmp_lower_expressions[i][j] *
                nnv.layers[0].neurons[j].bounds.getLb();
        } else {
          lb += tmp_lower_expressions[i][j] *
                nnv.layers[0].neurons[j].bounds.getUb();
        }

        // ub
        if (tmp_upper_expressions[i][j] >= 0) {
          ub += tmp_upper_expressions[i][j] *
                nnv.layers[0].neurons[j].bounds.getUb();
        } else {
          ub += tmp_upper_expressions[i][j] *
                nnv.layers[0].neurons[j].bounds.getLb();
        }
      }
      lb += lower_biases[i];
      ub += upper_biases[i];

      nnv.layers[start_layer_idx].neurons[i].bounds.setLb(lb);
      nnv.layers[start_layer_idx].neurons[i].bounds.setUb(ub);
    }

    return;
  }
};

#endif // DEEPPOLY_DOMAIN_HPP
