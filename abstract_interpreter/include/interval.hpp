#include <algorithm>

#include "ast.hpp"

class Interval {
public:
    using long long ll;

    ll lb;
    ll ub;

    Interval() : lb(0), ub(0) {}
    Interval(int lb, int ub) : lb(lb), ub(ub) {}

    void add(const Interval& other){
        lb += other.lb;
        ub += other.ub; 
        return;
    }

    void sub(const Interval& other){
        lb -= other.lb;
        ub -= other.ub;
        return;
    }

    void mul(const Interval& other){
        ll B[4];
        B[0] = lb * other.lb;
        B[1] = lb * other.ub;
        B[2] = ub * other.lb;
        B[3] = ub * other.ub;

        lb = std::min_element(B, B+4);
        ub = std::max_element(B, B+4);

        return;
    }

    void div(const Interval& other){
        if (0 < other.lb || other.ub < 0){
            std::cout << "[ERORR]: The division contains zero." << std::endl; 
        }

        ll B[4];
        B[0] = lb * (1 / other.lb);
        B[1] = lb * (1 / other.ub);
        B[2] = ub * (1 / other.lb);
        B[3] = ub * (1 / other.ub);

        lb = std::min_element(B, B+4);
        ub = std::max_element(B, B+4);

        return;
    }

    bool le(const Interval& other){
        return false;
    }

    bool leq(const Interval& other){
        return false;
    }
    
    bool ge(const Interval& other){
        return false;
    }
    
    bool geq(const Interval& other){
        return false;
    }
    
    bool eq(const Interval& other){
        return false;
    }
    
    bool neq(const Interval& other){
        return false;
    }

    void meet(const Interval& other){
        lb = std::max(lb, other.lb);
        ub = std::min(ub, other.ub);
    }

    void join(const Interval& other){
        lb = std::min(lb, other.lb);
        ub = std::max(ub, other.ub);
    }

    void widen(const Interval& other){
        if (lb > other.lb){
            lb = std::numeric_limits<int>::min();
        }
        if (ub < other.ub){
            ub = std::numeric_limits<int>::max();
        }
    }
};