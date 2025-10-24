#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include <iostream>
#include <limits>

#define MAX_DOUBLE (std::numeric_limits<double>::max())
#define MIN_DOUBLE (std::numeric_limits<double>::lowest())

struct Interval {
public:
  // Interval() : lb(MIN_DOUBLE), ub(MAX_DOUBLE) {}
  // Interval() : lb(-999999), ub(999999) {}
  Interval() : lb(0), ub(0) {}
  Interval(double _lb, double _ub) : lb(_lb), ub(_ub) {}

  void setLb(double _lb) { lb = _lb; }
  void setUb(double _ub) { ub = _ub; }
  double getLb() const { return lb; }
  double getUb() const { return ub; }

  Interval operator+(const Interval &other) const {
    return Interval(lb + other.lb, ub + other.ub);
  }

  // + scalar
  Interval operator+(double scalar) const {
    return Interval(lb + scalar, ub + scalar);
  }

  // - Interval
  Interval operator-(const Interval &other) const {
    return Interval(lb - other.ub, ub - other.lb);
  }

  // * scalar
  Interval operator*(double scalar) const {
    if (scalar >= 0) {
      return Interval(lb * scalar, ub * scalar);
    } else {
      return Interval(ub * scalar, lb * scalar);
    }
  }

  void print() const {
    std::cout << "[" << lb << ", " << ub << "]" << std::endl;
  }

private:
  double ub;
  double lb;
};

#endif // INTERVAL_HPP
