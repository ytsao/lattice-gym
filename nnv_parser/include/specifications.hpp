#ifndef SPECIFICATIONS_HPP
#define SPECIFICATIONS_HPP

#include "neuron.hpp"
#include <map>
#include <string>
#include <vector>

struct Specification {
  size_t numberOfInputs = 0;
  size_t numberOfOutputs = 0;
  std::map<std::string, Neuron> variables;

  // postconditions: Ay+ b<=0;
  // first dimension: the index of disjunctive terms;
  std::vector<std::vector<std::vector<int>>> A;
  std::vector<std::vector<double>> b;

  Specification() : numberOfInputs(0), numberOfOutputs(0) {}
};

#endif // SPECIFICATIONS_HPP
