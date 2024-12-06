#include "ast.hpp"

class Interval {
public:
    int lower;
    int upper;

    Interval() : lower(0), upper(0) {}
    Interval(int lower, int upper) : lower(lower), upper(upper) {}

    void add(){return;}
    void sub(){return;}
    void mul(){return;}
    void div(){return;}

    void le(){return;}
    void leq(){return;}
    void ge(){return;}
    void geq(){return;}
    void eq(){return;}
    void neq(){return;}

    void meet(const Interval& other){
        lower = std::max(lower, other.lower);
        upper = std::min(upper, other.upper);
    }

    void join(const Interval& other){
        lower = std::min(lower, other.lower);
        upper = std::max(upper, other.upper);
    }

    void widen(const Interval& other){
        if (lower > other.lower){
            lower = std::numeric_limits<int>::min();
        }
        if (upper < other.upper){
            upper = std::numeric_limits<int>::max();
        }
    }
};