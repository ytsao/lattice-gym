#ifndef SPECIFICATIONS_HPP
#define SPECIFICATIONS_HPP

#include "neuron.hpp"
#include <map>
#include <string>
#include <vector>

struct Specification {
public:
  size_t numberOfInputs = 0;
  size_t numberOfOutputs = 0;
  std::map<std::string, Neuron> variables;

  // postconditions: Ay+ b<=0;
  // first dimension: the index of disjunctive terms;
  std::vector<std::vector<std::vector<int>>> A;
  std::vector<std::vector<double>> b;

  Specification() : numberOfInputs(0), numberOfOutputs(0) {}

  // TODO: check if the property is satisfied by the overapproximated outputs.
  bool check() { return false; }
};

#endif // SPECIFICATIONS_HPP
