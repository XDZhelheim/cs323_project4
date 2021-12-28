#ifndef NODE_ANALYSER_HPP
#define NODE_ANALYSER_HPP

#include "TreeNode.hpp"
#include "Type.hpp"
#include <map>
#include <iostream>

using std::cout;
using std::map;
using std::to_string;

map<string, Type *> symbolTable;

bool isArithmatic(string name)
{
    return name == "PLUS" || name == "MINUS" || name == "MUL" || name == "DIV";
}

bool check_func_signature(vector<Type *> &func, vector<Type *> &varList)
{
    if (func.size() != varList.size())
    {
        return false;
    }
    for (int i = 0; i < func.size(); i++)
    {
        if (func[i]->getSigniture() != varList[i]->getSigniture())
        {
            return false;
        }
    }
    return true;
}

Type *find_structure_mem(vector<Type *> &varList, string name)
{
    for (auto var : varList)
    {
        if (var->name == name)
        {
            return var;
        }
    }
    return new Type(Category::ERROR_VAL);
}

class Analyser
{
private:
    TreeNode *root;

public:
    Analyser(TreeNode *node, string path)
    {
        root = node;
        out = ofstream(path);
    }

    void analyze()
    {
        analyzeProgram(root);
    }

    /*
    Program: 
      ExtDefList 
    */
    void analyzeProgram(TreeNode *node)
    {
        analyzeExtDefList(node->child[0]);
    }

    /*
    ExtDefList: 
      ExtDef ExtDefList
    | %empty
    */
    void analyzeExtDefList(TreeNode *node)
    {
        if (node->child.empty())
        {
            // empty
            return;
        }
        else
        {
            // ExtDef ExtDefList
            analyzeExtDef(node->child[0]);
            analyzeExtDefList(node->child[1]);
        }
    }

    /*
    ExtDef: 
      Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    */
    void analyzeExtDef(TreeNode *node)
    {
        if (node->child.size() == 2)
        {
            // Specifier SEMI
            analyzeSpecifier(node->child[0]);
        }
        else
        {
            Type *specifier = analyzeSpecifier(node->child[0]);
            if (node->child[2]->child.empty())
            {
                // Specifier ExtDecList SEMI
                analyzeExtDecList(node->child[1], specifier);
            }
            else
            {
                // Specifier FunDec CompSt
                Type *funcType = new Type(Category::FUNCTION);
                funcType->returnType = specifier;
                analyzeFunDec(node->child[1], funcType);
                analyzeCompSt(node->child[2], funcType);
            }
        }
    }

    /*
    ExtDecList: 
      VarDec
    | VarDec COMMA ExtDecList
    */
    void analyzeExtDecList(TreeNode *node, Type *specifier)
    {
        analyzeVarDec(node->child[0], specifier);

        if (node->child.size() == 3)
        {
            // VarDec COMMA ExtDecList
            analyzeExtDecList(node->child[2], specifier);
        }
    }

    /*
    Specifier: 
      TYPE
    | StructSpecifier
    */
    Type *analyzeSpecifier(TreeNode *node)
    {
        if (node->child[0]->child.empty())
        {
            // TYPE
            Category category;
            if (node->child[0]->data == "int")
            {
                category = Category::INT_VAR;
            }
            else if (node->child[0]->data == "float")
            {
                category = Category::FLOAT_VAR;
            }
            else if (node->child[0]->data == "char")
            {
                category = Category::CHAR_VAR;
            }
            return new Type(category);
        }
        else
        {
            // StructSpecifier
            return analyzeStructSpecifier(node->child[0]);
        }
    }

    /*
    StructSpecifier: 
      STRUCT ID LC DefList RC
    | STRUCT ID
    */
    Type *analyzeStructSpecifier(TreeNode *node)
    {
        TreeNode *id = node->child[1];
        if (node->child.size() == 5)
        {
            // STRUCT ID LC DefList RC
            Type *structSpecifier = new Type(Category::STRUCTURE_DEF);

            if (symbolTable.count(id->data) > 0)
            {
                print_type_15(id->pos);
                return new Type(Category::ERROR_VAL);
            }
            structSpecifier->name = id->data;
            symbolTable[id->data] = structSpecifier;
            analyzeDefList(node->child[3], structSpecifier);
            return structSpecifier;
        }
        else
        {
            return symbolTable[id->data];
        }
    }

    /*
    VarDec:
      ID
    | VarDec LB INT RB
    */
    Type *analyzeVarDec(TreeNode *node, Type *specifier)
    {
        if (node->child.size() == 1)
        {
            // ID
            TreeNode *id = node->child[0];
            if (symbolTable.count(id->data) > 0)
            {
                print_type_3(id->pos);
                return new Type(Category::ERROR_VAL);
            }
            if (specifier->category == Category::STRUCTURE_VAL)
            {
                Type *spec = new Type(Category::STRUCTURE_VAL);
                spec->name = id->data;
                spec->structType = specifier->structType;
                symbolTable[id->data] = spec;
                return spec;
            }
            else if (specifier->category == Category::ARRAY)
            {
                specifier->name = id->data;
                symbolTable[id->data] = specifier;
                return specifier;
            }
            else
            {
                Type *spec = new Type;
                spec->category = specifier->category;
                spec->name = id->data;
                symbolTable[id->data] = spec;
                return spec;
            }
        }
        else
        {
            // VarDec LB INT RB
            Type *parrSpecifier = new Type(Category::ARRAY);
            parrSpecifier->array = new Array(specifier, strtol(node->child[2]->data.c_str(), NULL, 0));
            return analyzeVarDec(node->child[0], parrSpecifier);
        }
    }

    /*
    FunDec: 
      ID LP VarList RP
    | ID LP RP
    */
    void analyzeFunDec(TreeNode *node, Type *funcType)
    {
        TreeNode *id = node->child[0];
        if (symbolTable.count(id->data) > 0)
        {
            print_type_4(id->pos);
            // return; // @test4
        }
        else
        { // @test4 redefine func, keep the first definition
            funcType->name = id->data;
            symbolTable[id->data] = funcType;
        }
        if (node->child.size() == 4)
        {
            // ID LP VarList RP
            analyzeVarList(node->child[2], funcType);
        }
    }

    /*
    VarList: 
      ParamDec COMMA VarList
    | ParamDec
    */
    void analyzeVarList(TreeNode *node, Type *spec)
    {
        analyzeParamDec(node->child[0], spec);
        if (node->child.size() == 3)
        {
            // ParamDec COMMA VarList
            analyzeVarList(node->child[2], spec);
        }
    }

    /*
    ParamDec: 
      Specifier VarDec
    */
    void analyzeParamDec(TreeNode *node, Type *spec)
    {
        Type *specifier = analyzeSpecifier(node->child[0]);
        spec->varlist.push_back(analyzeVarDec(node->child[1], specifier));
    }

    /*
    CompSt: 
      LC DefList StmtList RC
    */
    void analyzeCompSt(TreeNode *node, Type *funcType)
    {
        Type trash;
        analyzeDefList(node->child[1], &trash);
        analyzeStmtList(node->child[2], funcType);
    }

    /*
    StmtList: 
      Stmt StmtList
    | %empty
    */
    void analyzeStmtList(TreeNode *node, Type *funcType)
    {
        if (node->child.empty())
        {
            // empty
            return;
        }
        else
        {
            // Stmt StmtList
            analyzeStmt(node->child[0], funcType);
            analyzeStmtList(node->child[1], funcType);
        }
    }

    /*
    Stmt: 
      Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    */
    void analyzeStmt(TreeNode *node, Type *specifier)
    {
        if (node->child.size() == 2)
        {
            // Exp SEMI
            analyzeExp(node->child[0]);
        }
        else if (node->child.size() == 1)
        {
            // CompSt
            analyzeCompSt(node->child[0], specifier);
        }
        else if (node->child.size() == 3)
        {
            // RETURN Exp SEMI
            Type *exp = analyzeExp(node->child[1]);
            if (exp->getSigniture() != "NULL" && specifier->returnType->getSigniture() != exp->getSigniture())
            {
                print_type_8(node->pos);
                return;
            }
        }
        else if (node->child.size() == 5)
        {
            // IF LP Exp RP Stmt & WHILE LP Exp RP Stmt
            analyzeExp(node->child[2]);
            analyzeStmt(node->child[4], specifier);
        }
        else
        {
            // IF LP Exp RP Stmt ELSE Stmt
            analyzeExp(node->child[2]);
            analyzeStmt(node->child[4], specifier);
            analyzeStmt(node->child[6], specifier);
        }
    }

    /*
    DefList: 
      Def DefList
    | %empty
    */
    void analyzeDefList(TreeNode *node, Type *spec)
    {
        if (node->child.size() == 2)
        {
            // Def DefList
            analyzeDef(node->child[0], spec);
            analyzeDefList(node->child[1], spec);
        }
    }

    /*
    Def: 
      Specifier DecList SEMI 
    */
    void analyzeDef(TreeNode *node, Type *spec)
    {
        Type *specifier = analyzeSpecifier(node->child[0]);
        if (specifier->category == Category::STRUCTURE_DEF)
        {
            Type *sp = specifier;
            specifier = new Type(Category::STRUCTURE_VAL);
            specifier->structType = sp;
        }
        analyzeDecList(node->child[1], specifier, spec);
    }

    /*
    DecList: 
      Dec
    | Dec COMMA DecList
    */
    void analyzeDecList(TreeNode *node, Type *specifier, Type *func)
    {
        Type *dec = analyzeDec(node->child[0], specifier);
        func->varlist.push_back(dec);
        symbolTable[dec->name] = dec;
        if (node->child.size() == 3)
        {
            // Dec COMMA DecList
            analyzeDecList(node->child[2], specifier, func);
        }
    }

    /*
    Dec: 
      VarDec
    | VarDec ASSIGN Exp
    */
    Type *analyzeDec(TreeNode *node, Type *specifier)
    {
        Type *var = analyzeVarDec(node->child[0], specifier);
        if (node->child.size() == 3)
        {
            // VarDec ASSIGN Exp
            analyzeExp(node->child[2]);
        }
        return var;
    }

    /*
    Exp: 
      Exp ASSIGN Exp                type check
    | Exp AND Exp                   int, int -> int
    | Exp OR Exp                    int, int -> int
    | Exp LT Exp                    int, int -> int
    | Exp LE Exp                    int, int -> int
    | Exp GT Exp                    int, int -> int
    | Exp GE Exp                    int, int -> int
    | Exp NE Exp                    int, int -> int
    | Exp EQ Exp                    int, int -> int
    | Exp PLUS Exp                  T, T -> T
    | Exp MINUS Exp                 T, T -> T
    | Exp MUL Exp                   T, T -> T
    | Exp DIV Exp                   T, T -> T
    | LP Exp RP                     T -> T
    | MINUS Exp                     T -> -T
    | NOT Exp                       int -> int
    | ID LP Args RP                 func
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT
    | FLOAT
    | CHAR
    */
    Type *analyzeExp(TreeNode *node)
    {
        if (node->child.size() == 3)
        {
            // yidadui
            if (node->child[0]->child.empty() && node->child[1]->child.empty() && node->child[2]->child.empty())
            {
                // ID LP RP
                if (!symbolTable.count(node->child[0]->data))
                {
                    print_type_2(node->pos);
                    return new Type(Category::ERROR_VAL);
                }
                Type *exp = symbolTable[node->child[0]->data];
                if (exp->category != Category::FUNCTION)
                {
                    print_type_11(node->pos);
                    return new Type(Category::ERROR_VAL);
                }
                if (exp->varlist.size() != 0)
                {
                    print_type_9(node->pos);
                    return new Type(ERROR_VAL);
                }
                return exp->returnType;
            }
            else if (node->child[0]->child.empty() && !node->child[1]->child.empty() && node->child[2]->child.empty())
            {
                // LP Exp RP
                return analyzeExp(node->child[1]);
            }
            else if (!node->child[0]->child.empty() && node->child[1]->child.empty() && node->child[2]->child.empty())
            {
                // Exp DOT ID
                Type *exp = analyzeExp(node->child[0]);
                if (exp->category != Category::STRUCTURE_VAL)
                {
                    print_type_13(node->pos);
                    return new Type(ERROR_VAL);
                }
                Type *t = find_structure_mem(exp->structType->varlist, node->child[2]->data);
                if (t->category == Category::ERROR_VAL)
                {
                    print_type_14(node->pos);
                    return new Type(ERROR_VAL);
                }
                return t;
            }
            else
            {
                // Exp op Exp
                Type *exp1 = analyzeExp(node->child[0]);
                Type *exp2 = analyzeExp(node->child[2]);
                if (exp1->getSigniture() != exp2->getSigniture())
                {
                    if (node->child[1]->name == "ASSIGN")
                    {
                        print_type_5(node->pos);
                    }
                    else
                    {
                        if (exp1->category != Category::ERROR_VAL && exp2->category != Category::ERROR_VAL)
                            print_type_7(node->pos);
                    }
                    return new Type(Category::ERROR_VAL);
                }
                if (node->child[1]->name == "ASSIGN" && (exp1->category == Category::INT_VAL || exp1->category == Category::FLOAT_VAL || exp1->category == CHAR_VAL))
                {
                    print_type_6(node->pos);
                    return new Type(Category::ERROR_VAL);
                }
                if (isArithmatic(node->child[1]->name))
                {
                    return new Type(exp1->category, exp1->name);
                }
                return new Type(Category::INT_VAL);
            }
        }
        else if (node->child.size() == 2)
        {
            return analyzeExp(node->child[1]);
        }
        else if (node->child.size() == 4)
        {
            if (node->child[0]->child.empty())
            {
                // ID LP Args RP
                vector<Type *> varList;
                analyzeArgs(node->child[2], varList);
                if (!symbolTable.count(node->child[0]->data))
                {
                    print_type_2(node->pos);
                    return new Type(Category::ERROR_VAL);
                }
                Type *exp = symbolTable[node->child[0]->data];
                if (exp->category != Category::FUNCTION)
                {
                    print_type_11(node->pos);
                    return new Type(Category::ERROR_VAL);
                }
                if (!check_func_signature(exp->varlist, varList))
                {
                    print_type_9(node->pos);
                    return new Type(ERROR_VAL);
                }
                return exp->returnType;
            }
            else
            {
                // Exp LB Exp RB
                Type *exp1 = analyzeExp(node->child[0]);
                Type *exp2 = analyzeExp(node->child[2]);
                int flag = 0;
                if (exp1->category != Category::ARRAY)
                {
                    print_type_10(node->pos);
                    flag = 1;
                }
                if (exp2->getSigniture() != "int")
                {
                    print_type_12(node->pos);
                    flag = 1;
                }
                if (flag)
                {
                    return new Type(Category::ERROR_VAL);
                }

                return exp1->array->type;
            }
        }
        else
        {
            // INT FLOAT CHAR ID
            Type *t;
            switch (node->child[0]->type)
            {
            case DataType::INT_TYPE:
                t = new Type(Category::INT_VAL);
                t->primitive_value = node->child[0]->data;
                return t;

            case DataType::FLOAT_TYPE:
                t = new Type(Category::FLOAT_VAL);
                t->primitive_value = node->child[0]->data;
                return t;

            case DataType::CHAR_TYPE:
                t = new Type(Category::CHAR_VAL);
                t->primitive_value = node->child[0]->data;
                return t;

            default:
                if (symbolTable.count(node->child[0]->data))
                {
                    Type *t = symbolTable[node->child[0]->data];
                    if (t->category == Category::STRUCTURE_DEF)
                    {
                        return new Type(Category::ERROR_VAL);
                    }
                    return t;
                }
                print_type_1(node->pos);
                return new Type(Category::ERROR_VAL);
            }
        }
    }

    /*
    Args: 
      Exp COMMA Args
    | Exp
    */
    void analyzeArgs(TreeNode *node, vector<Type *> &varList)
    {
        varList.push_back(analyzeExp(node->child[0]));
        if (node->child.size() == 3)
        {
            // Exp COMMA Args
            analyzeArgs(node->child[2], varList);
        }
    }
};

void analysisTreeNode(char *file_path)
{
    string path = file_path;
    string out_path = "/dev/stdout";
    if (path.substr(path.length() - 4) == ".spl")
    {
        out_path = path.substr(0, path.length() - 4) + ".out";
    }
    Analyser(root, out_path).analyze();
}

#endif