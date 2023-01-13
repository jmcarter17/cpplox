#ifndef CPPLOX_VALUE_H
#define CPPLOX_VALUE_H

#include <variant>
#include "object.h"

using Value = std::variant<double, bool, std::monostate, Obj*>;
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

template<typename T>
Value createValue(T value);
Value number_val(double val);
Value bool_val(bool val);
Value nil_val();
Value obj_val(Obj* obj);

double asNumber(Value value);
bool asBool(Value value);
Obj* asObject(Value value);
ObjString* asString(Value value);
const char* asCString(Value value);
bool isNumber(Value val);
bool isBool(Value val);
bool isNil(Value val);
bool isObj(Value val);
bool isObjType(Value value, ObjType type);
bool isString(Value value);


bool isFalsey(Value val);

bool valuesEqual(Value a, Value b);

void printValue(Value value);


#endif //CPPLOX_VALUE_H
