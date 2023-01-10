#include "value.h"

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
