#ifndef TYPE_HPP
#define TYPE_HPP

#include <string>
#include <vector>
#include "ErrorPrinter.hpp"

using std::string;
using std::vector;

enum Category
{
    INT_VAL,
    FLOAT_VAL,
    CHAR_VAL,
    INT_VAR,
    FLOAT_VAR,
    CHAR_VAR,
    ARRAY,
    STRUCTURE_DEF,
    STRUCTURE_VAL,
    FUNCTION,
    ERROR_VAL
};

class Array;

class Type
{
public:
    string name;
    Category category;

    string primitive_value;

    Array *array;

    vector<Type *> varlist;

    Type *structType;

    Type *returnType;

    Type() = default;

    Type(Category _category, string _name = "");

    string getSigniture();
};

class Array
{
public:
    Type *type;
    int size;

    Array(Type *_type, int _size);
};

Type::Type(Category _category, string _name)
{
    category = _category;
    name = _name;
}

string Type::getSigniture()
{
    string base = "";
    Type *t = this;
    switch (category)
    {
    case Category::INT_VAL:
    case Category::INT_VAR:
        return "int";

    case Category::FLOAT_VAL:
    case Category::FLOAT_VAR:
        return "float";

    case Category::CHAR_VAL:
    case Category::CHAR_VAR:
        return "char";

    case Category::ARRAY:
        while (t->category == Category::ARRAY)
        {
            t = t->array->type;
            base += "*";
        }

        return t->getSigniture() + base;

    case Category::STRUCTURE_VAL:
        return "struct " + structType->name;

    case Category::FUNCTION:
        return "Func " + name;

    default:
        return "NULL";
    }
}

Array::Array(Type *_type, int _size)
{
    type = _type;
    size = _size;
}

#endif