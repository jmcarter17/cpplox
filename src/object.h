
#ifndef CPPLOX_OBJECT_H
#define CPPLOX_OBJECT_H


#include <string>

enum class ObjType{
    STRING
};

struct Obj {
    ObjType type;
    Obj* next;
};

struct ObjString : public Obj {
    std::string str;

    ObjString(ObjType type, Obj* obj, std::string_view view);
};

void printObject(Obj* obj);


#endif //CPPLOX_OBJECT_H
