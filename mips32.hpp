#ifndef MIPS_HPP
#define MIPS_HPP

#include "tac.h"
#include <string>
#include <set>

using std::set;
using std::string;

typedef enum
{
    zero,
    at,
    v0,
    v1,
    a0,
    a1,
    a2,
    a3,
    t0,
    t1,
    t2,
    t3,
    t4,
    t5,
    t6,
    t7,
    s0,
    s1,
    s2,
    s3,
    s4,
    s5,
    s6,
    s7,
    t8,
    t9,
    k0,
    k1,
    gp,
    sp,
    fp,
    ra,
    NUM_REGS
} Register;

struct RegDesc
{ // the register descriptor
    string name;
    string var;
    bool dirty; // value updated but not stored
};

extern struct RegDesc regs[NUM_REGS];

void mips32_gen(tac *head, FILE *_fd);

extern set<string> vars_set;

#endif // MIPS_HPP
