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
        FILE *fp, *output;
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
        fp = tmpfile();
        head = tac_from_buffer(buf);
        mips32_gen(head, fp);

        fflush(fp);
        fseek(fp, 0, SEEK_SET);

        char *line = (char *)malloc(1024);
        size_t s;

        output = fopen(out_path.c_str(), "w");

        while (getline(&line, &s, fp) != EOF)
        {
            if (strcmp(line, "  pushvars\n") == 0)
            {
                fprintf(output, "  addi $sp, $sp, -%lu\n", vars_set.size() * 4);
                int i = 0;
                for (auto var : vars_set)
                {
                    fprintf(output, "  lw $t3, _%s\n", var.c_str());
                    fprintf(output, "  sw $t3, %d($sp)\n", (i++) * 4);
                }
            }
            else if (strcmp(line, "  popvars\n") == 0)
            {
                int i = 0;
                for (auto var : vars_set)
                {
                    fprintf(output, "  lw $t3, %d($sp)\n", (i++) * 4);
                    fprintf(output, "  sw $t3, _%s\n", var.c_str());
                }
                fprintf(output, "  addi $sp, $sp, %lu\n", vars_set.size() * 4);
            }
            else
            {
                fprintf(output, "%s", line);
                if (strcmp(line, ".data\n") == 0)
                {
                    for (auto s : vars_set)
                    {
                        fprintf(output, "_%s: .word 0\n", s.c_str());
                    }
                }
            }
        }

        delete[] line;

        fflush(output);
        fclose(output);
        fclose(fp);
    }
}

#endif