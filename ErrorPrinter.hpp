#ifndef ERROR_PRINTER_HPP
#define ERROR_PRINTER_HPP

#include <fstream>

using std::endl;
using std::ofstream;

ofstream out;

void print_type_1(int lineno)
{
    has_error = true;
    out << "Error type 1 at Line " << lineno << ": a variable is used without a definition" << endl;
}

void print_type_2(int lineno)
{
    has_error = true;
    out << "Error type 2 at Line " << lineno << ": a function is invoked without a definition" << endl;
}

void print_type_3(int lineno)
{
    has_error = true;
    out << "Error type 3 at Line " << lineno << ": a variable is redefined in the same scope" << endl;
}

void print_type_4(int lineno)
{
    has_error = true;
    out << "Error type 4 at Line " << lineno << ": a function is redefined" << endl;
}

void print_type_5(int lineno)
{
    has_error = true;
    out << "Error type 5 at Line " << lineno << ": unmatching types appear at both sides of the assignment operator" << endl;
}

void print_type_6(int lineno)
{
    has_error = true;
    out << "Error type 6 at Line " << lineno << ": rvalue appears on the left-hand side of the assignment operator" << endl;
}

void print_type_7(int lineno)
{
    has_error = true;
    out << "Error type 7 at Line " << lineno << ": unmatching operands" << endl;
}

void print_type_8(int lineno)
{
    has_error = true;
    out << "Error type 8 at Line " << lineno << ": a function’s return value type mismatches the declared type" << endl;
}

void print_type_9(int lineno)
{
    has_error = true;
    out << "Error type 9 at Line " << lineno << ": a function’s arguments mismatch the declared parameters" << endl;
}

void print_type_10(int lineno)
{
    has_error = true;
    out << "Error type 10 at Line " << lineno << ": applying indexing operator on non-array type variables" << endl;
}

void print_type_11(int lineno)
{
    has_error = true;
    out << "Error type 11 at Line " << lineno << ": applying function invocation operator on non-function names" << endl;
}

void print_type_12(int lineno)
{
    has_error = true;
    out << "Error type 12 at Line " << lineno << ": array indexing with a non-integer type expression" << endl;
}

void print_type_13(int lineno)
{
    has_error = true;
    out << "Error type 13 at Line " << lineno << ": accessing members of a non-structure variable" << endl;
}

void print_type_14(int lineno)
{
    has_error = true;
    out << "Error type 14 at Line " << lineno << ": accessing an undefined structure member" << endl;
}

void print_type_15(int lineno)
{
    has_error = true;
    out << "Error type 15 at Line " << lineno << ": redefine the same structure type" << endl;
}

#endif