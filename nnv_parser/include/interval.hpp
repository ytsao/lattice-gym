#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include <iostream>

struct Interval {
public:
  Interval() : lb(-9999.99), ub(9999.99) {}
  Interval(double _lb, double _ub) : lb(_lb), ub(_ub) {}

  void setLb(double _lb) { lb = _lb; }
  void setUb(double _ub) { ub = _ub; }
  double getLb() const { return lb; }
  double getUb() const { return ub; }

  Interval operator+(const Interval &other) const {
    return Interval(lb + other.getLb(), ub + other.getUb());
  }

  Interval operator-(const Interval &other) const {
    return Interval(lb - other.getUb(), ub - other.getLb());
  }

  Interval operator*(const double scalar) const {
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
