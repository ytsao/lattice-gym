#ifndef DEEPPOLY_DOMAIN_HPP
#define DEEPPOLY_DOMAIN_HPP

#include "abstract_domain.hpp"

class DeepPolyDomain : public AbstractDomain {
public:
  void gamma(Network &nnv, size_t layer_idx) override {
    // Applying back-substitution to compute concrete bounds from DeepPoly
    // expressions with respect to the input layer.
    back_substitution(nnv, layer_idx);

    return;
  }

  void flatten_layer_transformer(Layer &current_layer) override {
    std::cout << "We're in flatten_layer_transformer of DeepPolyDomain."
              << std::endl;
    for (size_t i = 0; i < current_layer.layer_size; ++i) {
      current_layer.deeppoly_lower_expressions.push_back(
          new double[current_layer.layer_size]);
      current_layer.deeppoly_upper_expressions.push_back(
          new double[current_layer.layer_size]);
    }

    return;
  }

  void relu_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    std::cout << "We're in relu_layer_transformer of DeepPolyDomain."
              << std::endl;
    create_deeppoly_expressions(from_layer, to_layer);

    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      double lb = from_layer.neurons[i].bounds.getLb();
      double ub = from_layer.neurons[i].bounds.getUb();

      if (lb >= 0) {
        to_layer.neurons[i].bounds.setLb(lb);
        to_layer.neurons[i].bounds.setUb(ub);

        // x_i = x_j;
        to_layer.deeppoly_lower_expressions[i][i] = 1;
        to_layer.deeppoly_upper_expressions[i][i] = 1;
      } else if (ub <= 0) {
        to_layer.neurons[i].bounds.setLb(0);
        to_layer.neurons[i].bounds.setUb(0);
      } else if (lb < 0 && ub > 0) {
        to_layer.neurons[i].lambda = std::abs(lb) <= ub ? true : false;

        to_layer.deeppoly_lower_expressions[i][i] = to_layer.neurons[i].lambda;
        to_layer.deeppoly_upper_expressions[i][i] = ub / (ub - lb);
        to_layer.upper_biases[i] += (ub * (-lb)) / (ub - lb);
      }
    }

    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,
                                Layer &to_layer) override {
    std::cout << "We're in matmul_layer_transformer of DeepPolyDomain."
              << std::endl;
    create_deeppoly_expressions(from_layer, to_layer);
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      to_layer.lower_biases.push_back(0.0);
      to_layer.upper_biases.push_back(0.0);
    }

    for (size_t j = 0; j < to_layer.layer_size; ++j) {
      for (size_t i = 0; i < from_layer.layer_size; ++i) {
        double weights = to_layer.weights[i][j];

        to_layer.deeppoly_lower_expressions[j][i] += weights;
        to_layer.deeppoly_upper_expressions[j][i] += weights;
      }
    }

    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override {
    std::cout << "We're in add_layer_transformer of DeepPolyDomain."
              << std::endl;
    create_deeppoly_expressions(from_layer, to_layer);

    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      // TODO: check if we can copy the expressions here.
      // // copy deeppoly expressions
      // to_layer.deeppoly_lower_expressions.push_back(
      //     from_layer.deeppoly_lower_expressions[i]);
      // to_layer.deeppoly_upper_expressions.push_back(
      //     from_layer.deeppoly_upper_expressions[i]);

      // update the biases
      to_layer.lower_biases[i] += from_layer.lower_biases[i];
      to_layer.upper_biases[i] += from_layer.upper_biases[i];
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    std::cout << "We're in gemm_layer_transformer of DeepPolyDomain."
              << std::endl;
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

    return;
  }

private:
  void create_deeppoly_expressions(const Layer &from_layer, Layer &to_layer) {
    // Initialize deeppoly expressions for each neuron in the to_layer
    // The size of expressions in to_layer depends on the size of from_layer.
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      to_layer.deeppoly_lower_expressions.push_back(
          new double[from_layer.layer_size]);
      to_layer.deeppoly_upper_expressions.push_back(
          new double[from_layer.layer_size]);
    }

    return;
  }

  void back_substitution(Network &nnv, size_t start_layer_idx) {
    // input and the first hidden layer depend on input layer directly.
    std::cout << "Concretization ... " << std::endl;

    // TODO: check that the input_size and store into nnv object;
    // Check why concretization has some problems.
    if (start_layer_idx <= 2) {
      for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
        double lb = 0.0;
        double ub = 0.0;
        for (size_t j = 0; j < nnv.layers[0].layer_size; ++j) {
          // lb
          if (nnv.layers[start_layer_idx].deeppoly_lower_expressions[i][j] >=
              0) {
            lb += nnv.layers[start_layer_idx].deeppoly_lower_expressions[i][j] *
                  nnv.layers[0].neurons[j].bounds.getLb();
          } else {
            lb += nnv.layers[start_layer_idx].deeppoly_lower_expressions[i][j] *
                  nnv.layers[0].neurons[j].bounds.getUb();
          }

          // ub
          if (nnv.layers[start_layer_idx].deeppoly_upper_expressions[i][j] >=
              0) {
            ub += nnv.layers[start_layer_idx].deeppoly_upper_expressions[i][j] *
                  nnv.layers[0].neurons[j].bounds.getUb();
          } else {
            ub += nnv.layers[start_layer_idx].deeppoly_upper_expressions[i][j] *
                  nnv.layers[0].neurons[j].bounds.getLb();
          }
        }
        nnv.layers[start_layer_idx].neurons[i].bounds.setLb(lb);
        nnv.layers[start_layer_idx].neurons[i].bounds.setUb(ub);
      }

      return;
    }

    // TODO: check if that is going to be the problem.
    if (nnv.layers[start_layer_idx].type == LayerType::Add) {
      for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
        nnv.layers[start_layer_idx].neurons[i].bounds.setLb(
            nnv.layers[start_layer_idx - 1].neurons[i].bounds.getLb() +
            nnv.layers[start_layer_idx].lower_biases[i]);

        nnv.layers[start_layer_idx].neurons[i].bounds.setUb(
            nnv.layers[start_layer_idx - 1].neurons[i].bounds.getUb() +
            nnv.layers[start_layer_idx].upper_biases[i]);
      }

      return;
    }

    // for the layers after the first hidden layer.
    std::vector<double *> substituted_lower_expressions;
    std::vector<double *> substituted_upper_expressions;
    for (size_t layer_idx = start_layer_idx; layer_idx > 2; --layer_idx) {
      // We do matrix multiplication from the start_layer_idx down to the input
      // layer.
      if (layer_idx == start_layer_idx) {
        for (size_t i = 0; i < nnv.layers[layer_idx].layer_size; ++i) {
          substituted_lower_expressions.push_back(new double[784]());
          substituted_upper_expressions.push_back(new double[784]());
        }
      }

      std::cout << "Procssing layer index: " << layer_idx << std::endl;

      for (size_t i = 0; i < nnv.layers[layer_idx].layer_size; ++i) {
        for (size_t j = 0; j < nnv.layers[layer_idx - 1].layer_size; ++j) {
          for (size_t k = 0; k < nnv.layers[layer_idx - 2].layer_size; ++k) {
            // Update lower expression
            substituted_lower_expressions[i][k] +=
                nnv.layers[layer_idx - 1].deeppoly_lower_expressions[j][k] *
                nnv.layers[layer_idx].deeppoly_lower_expressions[i][j];

            // Update upper expression
            substituted_upper_expressions[i][k] +=
                nnv.layers[layer_idx - 1].deeppoly_upper_expressions[j][k] *
                nnv.layers[layer_idx].deeppoly_upper_expressions[i][j];
          }
        }
      }
    }

    std::cout << "compute lbs & ubs ..." << std::endl;

    std::vector<double> lbs;
    std::vector<double> ubs;
    for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
      // compute concrete bounds;
      for (size_t j = 0; j < 784; ++j) {
        lbs.push_back(
            nnv.layers[start_layer_idx].deeppoly_lower_expressions[i][j] *
            nnv.layers[1].neurons[j].bounds.getLb());
        ubs.push_back(
            nnv.layers[start_layer_idx].deeppoly_upper_expressions[i][j] *
            nnv.layers[1].neurons[j].bounds.getUb());
      }
    }

    // updatae concrete bounds
    for (size_t i = 0; i < nnv.layers[start_layer_idx].layer_size; ++i) {
      nnv.layers[start_layer_idx].neurons[i].bounds.setLb(lbs[i]);
      nnv.layers[start_layer_idx].neurons[i].bounds.setUb(ubs[i]);
    }

    return;
  }
};

#endif // DEEPPOLY_DOMAIN_HPP
