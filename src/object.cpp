
#include <fmt/core.h>
#include "object.h"

ObjString::ObjString(ObjType type, Obj* next, std::string_view view) : Obj{type, next}, str{view} {
}


void printObject(Obj* obj) {
    switch (obj->type) {
        case ObjType::STRING:
            fmt::print("{}", static_cast<ObjString*>(obj)->str);
            break;
        default:
            break; // unreachable
    }
}