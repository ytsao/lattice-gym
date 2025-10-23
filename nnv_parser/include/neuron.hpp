#ifndef NEURON_HPP
#define NEURON_HPP

#include "interval.hpp"
#include <vector>

struct Neuron {
public:
  size_t id; // the index in the same layer, it starts from 0.
  // std::vector<Interval> bounds; // each dimension represents a disjunctive
  // term.
  Interval bounds; // for simplicity

  Neuron() : id(0), bounds() {}

  void setId(int _id) { id = _id; }
};

#endif // NEURON_HPP
