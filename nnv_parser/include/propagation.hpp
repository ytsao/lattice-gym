#ifndef PROPAGATION_HPP
#define PROPAGATION_HPP

#include "network.hpp"

class Propagation {
public:
  virtual ~Propagation() = default;

  virtual bool execute(Network &nnv) = 0;
};

#endif // PROPAGATION_HPP
