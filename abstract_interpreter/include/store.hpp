#include "ast.hpp"
#include "interval.hpp"

class InterValStore {
public:
    std::map<std::string, Interval> store;

    void set(const std::string& name, const Interval& value){
        store[name] = value;
    }

    Interval get(const std::string& name){
        return store[name];
    }

    void add(const std::string& name, const Interval& value){
        store[name].add();
    }
    
    void sub(const std::string& name, const Interval& value){
        store[name].sub();
    }

    void mul(const std::string& name, const Interval& value){
        store[name].mul();
    }

    void div(const std::string& name, const Interval& value){
        store[name].div();
    }

    void le(const std::string& name, const Interval& value){
        store[name].le();
    }

    void leq(const std::string& name, const Interval& value){
        store[name].leq();
    }

    void ge(const std::string& name, const Interval& value){
        store[name].ge();
    }

    void geq(const std::string& name, const Interval& value){
        store[name].geq();
    }

    void eq(const std::string& name, const Interval& value){
        store[name].eq();
    }

    void neq(const std::string& name, const Interval& value){
        store[name].neq();
    }

    void join(const std::string& name, const Interval& value){
        store[name].join(value);
    }

    void meet(const std::string& name, const Interval& value){
        store[name].meet(value);
    }

    void widen(const std::string& name, const Interval& value){
        store[name].widen(value);
    }
};