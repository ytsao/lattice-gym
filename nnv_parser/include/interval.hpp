#ifndef INTERVAL_HPP
#define INTERVAL_HPP

struct Interval {
public:
  Interval() : ub(9999.99), lb(-9999.99) {}

  void setLb(double _lb) { lb = _lb; }
  void setUb(double _ub) { ub = _ub; }
  double getLb() { return lb; }
  double getUb() { return ub; }

private:
  double ub;
  double lb;
};

#endif // INTERVAL_HPP
