#ifndef SPECIFICATIONS_HPP
#define SPECIFICATIONS_HPP

#include "layer.hpp"
#include <map>
#include <string>
#include <vector>

struct Specification {
public:
  size_t numberOfInputs = 0;
  size_t numberOfOutputs = 0;
  std::map<std::string, Neuron> variables;

  // postconditions: Ay+ b>=0;
  // first dimension: the index of disjunctive terms;
  std::vector<std::vector<std::vector<int>>> A;
  std::vector<std::vector<double>> b;
  std::vector<std::vector<char>> relations;

  Specification() : numberOfInputs(0), numberOfOutputs(0) {}

  bool check(const Layer &spec_layer) {
    /*
     * true: UNSAT
     * false: SAT
     * */
    if (A.empty()) {
      // This is for prop_1 || the case that only replies on output bounds,
      // i.e., y <= 10 or y >= 3.
      for (const auto &variable : variables) {
        if (variable.first.substr(0, 1) == "Y") {
          double spec_lb = variable.second.bounds.getLb();
          double neuron_lb = spec_layer.neurons[variable.second.id].bounds.getLb();
          if (neuron_lb < spec_lb && spec_lb != 0)
            return false;

          double spec_ub = variable.second.bounds.getUb();
          double neuron_ub = spec_layer.neurons[variable.second.id].bounds.getUb();
          if (neuron_ub > spec_ub && spec_ub != 0)
            return false;
        }
      }
      return true;
    }

    bool *postconditions_verification_result = new bool[A.size()];
    for (size_t disjunctive_id = 0; disjunctive_id < A.size(); ++disjunctive_id) {
      bool postcondition_satisfied = false;
      for (size_t conjunctive_id = 0; conjunctive_id < A[disjunctive_id].size(); ++conjunctive_id) {
        size_t neuron_id = disjunctive_id * A[disjunctive_id].size() + conjunctive_id;
        if (spec_layer.neurons[neuron_id].bounds.getLb() > 0) {
          postcondition_satisfied |= true;
        } else
          postcondition_satisfied |= false;
      }
      postconditions_verification_result[disjunctive_id] = postcondition_satisfied;
    }

    for (size_t disjunctive_id = 0; disjunctive_id < A.size(); ++disjunctive_id) {
      if (postconditions_verification_result[disjunctive_id] == false) {
        delete[] postconditions_verification_result;
        return false;
      }
    }
    delete[] postconditions_verification_result;
    return true;
  }
};

#endif // SPECIFICATIONS_HPP
