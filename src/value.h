#ifndef CPPLOX_VALUE_H
#define CPPLOX_VALUE_H

#include <variant>

using Value = std::variant<double, bool, std::monostate>;
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

template<typename T>
Value createValue(T value);
Value number_val(double val);
Value bool_val(bool val);
Value nil_val();
double asNumber(Value value);
bool asBool(Value value);
bool isNumber(Value val);
bool isBool(Value val);
bool isNil(Value val);

bool isFalsey(Value val);

//bool valuesEqual(Value a, Value b);

void printValue(Value value);


#endif //CPPLOX_VALUE_H
