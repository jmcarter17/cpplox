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

Value obj_val(Obj* obj) {
    return createValue(obj);
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

bool isObj(Value val) {
    return std::holds_alternative<Obj*>(val);;
}

bool isObjType(Value value, ObjType type) {
    return isObj(value) && asObject(value)->type == type;
}


bool isString(Value value) {
    return isObjType(value, ObjType::STRING);
}

Obj* asObject(Value value) {
    return std::get<Obj*>(value);
}

ObjString *asString(Value value) {
    return static_cast<ObjString*>(asObject(value));
}

//const char* asCString(Value value) {
//    return static_cast<ObjString*>(asObject(value))->str.data();
//}

bool isFalsey(Value val) {
//    return isNil(val) || (isBool(val) && !asBool(val));
    return std::visit(overload{
        [](double)         {return false;},
        [](bool val)       {return !val;},
        [](std::monostate) {return true;},
        [](Obj* obj)       {return false;}
    }, val);
}

void printValue(Value value) {
    std::visit(overload{
            [](double val)     { fmt::print("{}", asNumber(val)); },
            [](bool val)       { fmt::print("{}", asBool(val)); },
            [](std::monostate) { fmt::print("nil"); },
            [](Obj* obj)       { printObject(obj); }
    }, value);
}

bool valuesEqual(Value a, Value b) {
    if (a.index() != b.index()) return false;

    if (std::holds_alternative<Obj*>(a)) {
        return asString(a)->str.data() == asString(b)->str.data();
    }

    return a == b;
}
