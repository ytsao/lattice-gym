#ifndef NEURON_HPP
#define NEURON_HPP

#include "interval.hpp"
#include <tuple>
#include <unordered_map>

struct TupleHash {
  template <typename T1, typename T2>
  std::size_t operator()(const std::tuple<T1, T2> &t) const noexcept {
    auto h1 = std::hash<T1>{}(std::get<0>(t));
    auto h2 = std::hash<T2>{}(std::get<1>(t));
    return h1 ^ (h2 << 1);
  }
};

struct Neuron {
public:
  size_t id; // the index in the same layer, it starts from 0.
  size_t layer_id;
  Interval bounds; // for simplicity

  // These two objects are used in symbolic_interval_domain & deeppoly_domain.
  bool lambda = true;
  // double *deeppoly_lower_expression;
  // double *deeppoly_upper_expression;

  // extract weights from symbolic expression by <layer_id, id>.
  // PS both ids start from 0.
  bool isSymbolic = true;
  std::unordered_map<std::tuple<size_t, size_t>, double, TupleHash>
      symbolic_lower_expression;
  std::unordered_map<std::tuple<size_t, size_t>, double, TupleHash>
      symbolic_upper_expression;

  Neuron() : id(0), bounds() {}

  void setId(int _id) { id = _id; }

  void setLayerId(int _layer_id) { layer_id = _layer_id; }

  bool operator==(const Neuron &other) const {
    return (id == other.id) && (layer_id == other.layer_id);
  }
};

#endif // NEURON_HPP
