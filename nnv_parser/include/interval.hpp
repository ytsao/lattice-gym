#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include "log.hpp"

struct Interval {
public:
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

  // - scalar
  Interval operator-(double scalar) const {
    return Interval(lb, ub) - Interval(scalar, scalar);
  }

  // * scalar
  Interval operator*(double scalar) const {
    if (scalar >= 0) {
      return Interval(lb * scalar, ub * scalar);
    } else {
      return Interval(ub * scalar, lb * scalar);
    }
  }

  // / scalar
  Interval operator/(double scalar) const {
    if (scalar == 0)
      return Interval(lb, ub);
    double a = lb / scalar;
    double b = ub / scalar;
    return Interval(std::min(a, b), std::max(a, b));
  }

  void dump() const {
    Logger::log(Logger::Level::INFO,
                "[" + std::to_string(lb) + ", " + std::to_string(ub) + "]");
  }

private:
  double ub;
  double lb;
};

#endif // INTERVAL_HPP
