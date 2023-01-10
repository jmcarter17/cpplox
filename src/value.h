#ifndef CPPLOX_VALUE_H
#define CPPLOX_VALUE_H

#include <variant>

using Value = std::variant<double, bool>;

template<typename T>
Value createValue(T value);
Value number_val(double val);
Value bool_val(bool val);
double asNumber(Value value);
bool asBool(Value value);
bool isNumber(Value val);
bool isBool(Value val);

#endif //CPPLOX_VALUE_H
