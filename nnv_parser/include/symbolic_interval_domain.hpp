#ifndef SYMBOLIC_INTERVAL_DOMAIN_HPP
#define SYMBOLIC_INTERVAL_DOMAIN_HPP

#include "abstract_domain.hpp"

class SymbolicIntervalDomain : public AbstractDomain {
public:
  void gamma(Network &nnv, const size_t layer_idx) override {
    for (size_t i = 0; i < nnv.layers[layer_idx].layer_size; ++i) {
      if (!nnv.layers[layer_idx].neurons[i].isSymbolic)
        // It has been concretizied before
        continue;

      double lb = 0.0;
      double ub = 0.0;

      // lb
      for (const auto &term :
           nnv.layers[layer_idx].neurons[i].symbolic_lower_expression) {
        size_t dep_layer_id = std::get<0>(term.first);
        size_t dep_neuron_id = std::get<1>(term.first);

        if (term.second >= 0) {
          lb +=
              (nnv.layers[dep_layer_id].neurons[dep_neuron_id].bounds.getLb()) *
              term.second;
        } else if (term.second < 0) {
          lb +=
              (nnv.layers[dep_layer_id].neurons[dep_neuron_id].bounds.getUb()) *
              term.second;
        }
      }

      // ub
      for (const auto &term :
           nnv.layers[layer_idx].neurons[i].symbolic_upper_expression) {
        size_t dep_layer_id = std::get<0>(term.first);
        size_t dep_neuron_id = std::get<1>(term.first);

        if (term.second >= 0) {
          ub +=
              (nnv.layers[dep_layer_id].neurons[dep_neuron_id].bounds.getUb()) *
              term.second;
        } else if (term.second < 0) {
          ub +=
              (nnv.layers[dep_layer_id].neurons[dep_neuron_id].bounds.getLb()) *
              term.second;
        }
      }

      nnv.layers[layer_idx].neurons[i].bounds.setLb(
          lb + nnv.layers[layer_idx].biases[i]);
      nnv.layers[layer_idx].neurons[i].bounds.setUb(
          ub + nnv.layers[layer_idx].biases[i]);
    }

    return;
  }

  void first_layer_transformer(Layer &current_layer) override {
    // Add symbolic expression in the input layer.
    for (size_t i = 0; i < current_layer.layer_size; ++i) {
      current_layer.neurons[i].symbolic_lower_expression[std::make_tuple(
          current_layer.neurons[i].layer_id, current_layer.neurons[i].id)] =
          1.0;
      current_layer.neurons[i].symbolic_upper_expression[std::make_tuple(
          current_layer.neurons[i].layer_id, current_layer.neurons[i].id)] =
          1.0;
    }

    return;
  }

  void subtraction_layer_transformer(const Layer &from_layer,
                                     Layer &to_layer) override {
    // Add symbolic expression in the input layer.
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds.setLb(from_layer.neurons[i].bounds.getLb());
      to_layer.neurons[i].bounds.setUb(from_layer.neurons[i].bounds.getUb());

      to_layer.neurons[i].symbolic_lower_expression[std::make_tuple(
          from_layer.neurons[i].layer_id, from_layer.neurons[i].id)] = 1.0;
      to_layer.neurons[i].symbolic_upper_expression[std::make_tuple(
          from_layer.neurons[i].layer_id, from_layer.neurons[i].id)] = 1.0;
    }
    return;
  }

  void division_layer_transformer(const Layer &from_layer,
                                  Layer &to_layer) override {
    // Add symbolic expression in the input layer.
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds.setLb(from_layer.neurons[i].bounds.getLb());
      to_layer.neurons[i].bounds.setUb(from_layer.neurons[i].bounds.getUb());

      to_layer.neurons[i].symbolic_lower_expression[std::make_tuple(
          from_layer.neurons[i].layer_id, from_layer.neurons[i].id)] = 1.0;
      to_layer.neurons[i].symbolic_upper_expression[std::make_tuple(
          from_layer.neurons[i].layer_id, from_layer.neurons[i].id)] = 1.0;
    }
    return;
  }

  void flatten_layer_transformer(const Layer &from_layer,
                                 Layer &to_layer) override {
    // // Normalization
    // for (size_t dim = 0; dim < current_layer.sub_values.size(); ++dim) {
    //   for (size_t i = 0; i < current_layer.layer_size; ++i) {
    //     current_layer.neurons[i].bounds =
    //         (current_layer.neurons[i].bounds -
    //         current_layer.sub_values[dim]) / current_layer.div_values[i];
    //   }
    // }

    // Add symbolic expression in the input layer.
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      to_layer.neurons[i].bounds.setLb(from_layer.neurons[i].bounds.getLb());
      to_layer.neurons[i].bounds.setUb(from_layer.neurons[i].bounds.getUb());

      to_layer.neurons[i].symbolic_lower_expression[std::make_tuple(
          from_layer.neurons[i].layer_id, from_layer.neurons[i].id)] = 1.0;
      to_layer.neurons[i].symbolic_upper_expression[std::make_tuple(
          from_layer.neurons[i].layer_id, from_layer.neurons[i].id)] = 1.0;
    }

    return;
  }

  void relu_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      double lb = from_layer.neurons[i].bounds.getLb();
      double ub = from_layer.neurons[i].bounds.getUb();

      if (lb >= 0.0) {
        to_layer.neurons[i].isSymbolic = true;
        to_layer.neurons[i].symbolic_lower_expression =
            from_layer.neurons[i].symbolic_lower_expression;
        to_layer.neurons[i].symbolic_upper_expression =
            from_layer.neurons[i].symbolic_upper_expression;

        to_layer.lower_biases.push_back(from_layer.lower_biases[i]);
        to_layer.upper_biases.push_back(from_layer.upper_biases[i]);
      } else if (ub <= 0.0) {
        to_layer.neurons[i].symbolic_lower_expression.clear();
        to_layer.neurons[i].symbolic_upper_expression.clear();

        to_layer.neurons[i].symbolic_lower_expression[std::make_tuple(
            to_layer.neurons[i].layer_id, to_layer.neurons[i].id)] = 1.0;
        to_layer.neurons[i].symbolic_upper_expression[std::make_tuple(
            to_layer.neurons[i].layer_id, to_layer.neurons[i].id)] = 1.0;

        to_layer.neurons[i].isSymbolic = false;

        to_layer.neurons[i].bounds.setLb(0.0);
        to_layer.neurons[i].bounds.setUb(0.0);

        to_layer.biases.push_back(0.0);
      } else if (lb < 0.0 && ub > 0.0) {
        to_layer.neurons[i].symbolic_lower_expression.clear();
        to_layer.neurons[i].symbolic_upper_expression.clear();

        to_layer.neurons[i].symbolic_lower_expression[std::make_tuple(
            to_layer.neurons[i].layer_id, to_layer.neurons[i].id)] = 1.0;
        to_layer.neurons[i].symbolic_upper_expression[std::make_tuple(
            to_layer.neurons[i].layer_id, to_layer.neurons[i].id)] = 1.0;

        to_layer.neurons[i].isSymbolic = false;

        to_layer.neurons[i].bounds.setLb(0.0);
        to_layer.neurons[i].bounds.setUb(ub);

        to_layer.biases.push_back(0.0);
      }
    }

    return;
  }
  void matmul_layer_transformer(const Layer &from_layer,
                                Layer &to_layer) override {
    // Initialize symbolic expressions for each neuron in the to_layer
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      to_layer.biases.push_back(0.0);
    }

    // Start the computation
    for (size_t j = 0; j < to_layer.layer_size; ++j) {
      for (size_t i = 0; i < from_layer.layer_size; ++i) {
        double weights = to_layer.weights[i][j];

        if (weights >= 0.0) {
          for (const auto &term :
               from_layer.neurons[i].symbolic_lower_expression) {
            to_layer.neurons[j].symbolic_lower_expression[term.first] +=
                weights * term.second;
          }
          for (const auto &term :
               from_layer.neurons[i].symbolic_upper_expression) {
            to_layer.neurons[j].symbolic_upper_expression[term.first] +=
                weights * term.second;
          }
        } else if (weights < 0.0) {
          for (const auto &term :
               from_layer.neurons[i].symbolic_lower_expression) {
            to_layer.neurons[j].symbolic_upper_expression[term.first] +=
                weights * term.second;
          }
          for (const auto &term :
               from_layer.neurons[i].symbolic_upper_expression) {
            to_layer.neurons[j].symbolic_lower_expression[term.first] +=
                weights * term.second;
          }
        }
      }
    }

    return;
  }
  void add_layer_transformer(const Layer &from_layer,
                             Layer &to_layer) override {
    for (size_t i = 0; i < from_layer.layer_size; ++i) {
      // copy symbolic expressions
      to_layer.neurons[i].symbolic_lower_expression =
          from_layer.neurons[i].symbolic_lower_expression;
      to_layer.neurons[i].symbolic_upper_expression =
          from_layer.neurons[i].symbolic_upper_expression;

      // The biases will be considered in gamma function.
    }

    return;
  }
  void gemm_layer_transformer(const Layer &from_layer,
                              Layer &to_layer) override {
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      for (size_t j = 0; j < from_layer.layer_size; ++j) {
        double weights = to_layer.weights[i][j];

        if (weights >= 0.0) {
          for (const auto &term :
               from_layer.neurons[j].symbolic_lower_expression) {
            to_layer.neurons[i].symbolic_lower_expression[term.first] +=
                weights * term.second;
          }
          for (const auto &term :
               from_layer.neurons[j].symbolic_upper_expression) {
            to_layer.neurons[i].symbolic_upper_expression[term.first] +=
                weights * term.second;
          }
        } else if (weights < 0.0) {
          for (const auto &term :
               from_layer.neurons[j].symbolic_lower_expression) {
            to_layer.neurons[i].symbolic_upper_expression[term.first] +=
                weights * term.second;
          }
          for (const auto &term :
               from_layer.neurons[j].symbolic_upper_expression) {
            to_layer.neurons[i].symbolic_lower_expression[term.first] +=
                weights * term.second;
          }
        }
      }
    }

    return;
  }

  void convolutional_layer_transformer(const Layer &from_layer,
                                       Layer &to_layer) override {
    for (size_t i = 0; i < to_layer.layer_size; ++i) {
      for (size_t j = 0; j < from_layer.layer_size; ++j) {
        double weights = to_layer.weights[i][j];

        if (weights >= 0.0) {
          for (const auto &term :
               from_layer.neurons[j].symbolic_lower_expression) {
            to_layer.neurons[i].symbolic_lower_expression[term.first] +=
                weights * term.second;
          }
          for (const auto &term :
               from_layer.neurons[j].symbolic_upper_expression) {
            to_layer.neurons[i].symbolic_upper_expression[term.first] +=
                weights * term.second;
          }
        } else if (weights < 0.0) {
          for (const auto &term :
               from_layer.neurons[j].symbolic_lower_expression) {
            to_layer.neurons[i].symbolic_upper_expression[term.first] +=
                weights * term.second;
          }
          for (const auto &term :
               from_layer.neurons[j].symbolic_upper_expression) {
            to_layer.neurons[i].symbolic_lower_expression[term.first] +=
                weights * term.second;
          }
        }
      }
    }
    return;
  }
};

#endif // SYMBOLIC_INTERVAL_DOMAIN_HPP
