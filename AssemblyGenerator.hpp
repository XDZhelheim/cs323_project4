#ifndef ASM_GENERTOR_HPP
#define ASM_GENERTOR_HPP

#include "CodeGenerator.hpp"
#include "tac.h"
#include "mips32.hpp"

#define BUF_SIZE 0x10000

using std::string;

char buf[BUF_SIZE];

void generateAssemblyCode(char *file_path)
{
    string ir_path = generateIntermidiateCode(file_path);
    if (!has_error)
    {
        string path = file_path;
        string out_path = "/dev/stdout";
        if (path.substr(path.length() - 4) == ".spl")
        {
            out_path = path.substr(0, path.length() - 4) + ".s";
        }
        FILE *fp;
        tac *head;
        char c;
        int size, len;

        // read the IR code
        size = 0;
        fp = fopen(ir_path.c_str(), "r");
        while ((c = getc(fp)) != EOF)
            buf[size++] = c;
        buf[size] = '\x7f';
        fclose(fp);

        // write the target code
        fp = fopen(out_path.c_str(), "w");
        head = tac_from_buffer(buf);
        mips32_gen(head, fp);
        fclose(fp);
    }
}

#endif