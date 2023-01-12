#include "value.h"
#include "fmt/core.h"

template<typename T>
Value createValue(T value) {
    return Value{value};
}

Value number_val(double val) {
    return createValue(val);
}

Value bool_val(bool val) {
    return createValue(val);
}

Value nil_val() {
    return createValue(std::monostate());
}

double asNumber(Value value) {
    return std::get<double>(value);
}

bool asBool(Value value) {
    return std::get<bool>(value);
}

bool isNumber(Value val) {
    return std::holds_alternative<double>(val);
}

bool isBool(Value val) {
    return std::holds_alternative<bool>(val);
}

bool isNil(Value val) {
    return std::holds_alternative<std::monostate>(val);
}

bool isFalsey(Value val) {
//    return isNil(val) || (isBool(val) && !asBool(val));
    return std::visit(overload{
        [](double) {return false;},
        [](bool val) {return !val;},
        [](std::monostate) {return true;}
    }, val);
}

//bool valuesEqual(Value a, Value b) {
//    return a == b;
//}


void printValue(Value value) {
    std::visit(overload{
            [](double val)     { fmt::print("{}", asNumber(val)); },
            [](bool val)       { fmt::print("{}", asBool(val)); },
            [](std::monostate) { fmt::print("nil"); }
    }, value);
}
