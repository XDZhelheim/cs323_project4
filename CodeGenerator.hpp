#ifndef CODE_GENERTOR_HPP
#define CODE_GENERTOR_HPP
#define DEBUG 0
#define DISABLE_SIMPLIFY 0

#include "NodeAnalyser.hpp"
#include <deque>
#include <sstream>

int vCounter = 0;
int tCounter = 0;
int lCounter = 0;

using std::deque;
using std::endl;
using std::istringstream;
using std::ostream;
using std::string;

class DNode
{
public:
    string name;
    deque<DNode *> adj;

    bool visited;

    DNode(string name)
    {
        this->name = name;
        this->visited = false;
    }
};

map<string, DNode *> dmap;
deque<DNode *> graphRoots;

void function_init()
{
    Type *int_type = new Type(Category::INT_VAL);

    Type *read = new Type(Category::FUNCTION, "read");
    read->returnType = int_type;
    symbolTable["read"] = read;

    Type *write = new Type(Category::FUNCTION, "write");
    write->returnType = int_type;
    write->varlist.push_back(int_type);
    symbolTable["write"] = write;
}

map<string, string> vmap;

string createVar()
{
    string varName = "v" + to_string(vCounter++);
    dmap[varName] = new DNode(varName);
    return varName;
}

string createTemp()
{
    string tempName = "t" + to_string(tCounter++);
    dmap[tempName] = new DNode(tempName);
    return tempName;
}

string createLabel()
{
    return "label" + to_string(lCounter++);
}

ostream &operator<<(ostream &out, vector<string> &v)
{
    for (auto line : v)
    {
        out << line << endl;
    }
    return out;
}

class Generator
{
private:
    TreeNode *root;

    string code;

public:
    Generator(TreeNode *node, string path)
    {
        root = node;
        out = ofstream(path);
    }

    void generate()
    {
        translateProgram(root);
        if (DISABLE_SIMPLIFY)
        {
            out << code;
        }
        else
        {
            vector<string> result = simplify();
            out << result;
        }
    }

    /*
    Program:
      ExtDefList
    */
    void translateProgram(TreeNode *node)
    {
        if (DEBUG)
            cout << "Program" << endl;

        translateExtDefList(node->child[0]);
    }

    /*
    ExtDefList:
      ExtDef ExtDefList
    | %empty
    */
    void translateExtDefList(TreeNode *node)
    {
        if (DEBUG)
            cout << "ExtDefList" << endl;

        if (node->child.empty())
        {
            return;
        }
        translateExtDef(node->child[0]);
        translateExtDefList(node->child[1]);
    }

    /*
    ExtDef:
      Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    */
    void translateExtDef(TreeNode *node)
    {
        if (DEBUG)
            cout << "ExtDef" << endl;

        if (node->child.size() == 2)
        {
            // Specifier SEMI
            translateSpecifier(node->child[0]);
        }
        else
        {
            if (node->child[2]->child.empty())
            {
                // Specifier ExtDecList SEMI
                translateExtDecList(node->child[1]);
            }
            else
            {
                translateFunDec(node->child[1]);
                code += translateCompSt(node->child[2]);
            }
        }
    }

    /*
    ExtDecList:
      VarDec
    | VarDec COMMA ExtDecList
    */
    void translateExtDecList(TreeNode *node)
    {
        if (DEBUG)
            cout << "ExtDecList" << endl;

        translateVarDec(node->child[0]);

        if (node->child.size() == 3)
        {
            // VarDec COMMA ExtDecList
            translateExtDecList(node->child[2]);
        }
    }

    /*
    Specifier:
      TYPE
    | StructSpecifier
    */
    void translateSpecifier(TreeNode *node)
    {
        if (DEBUG)
            cout << "Specifier" << endl;

        if (node->child[0]->child.empty())
        {
            return;
        }
        else
        {
            // StructSpecifier
            translateStructSpecifier(node->child[0]);
        }
    }

    /*
    StructSpecifier:
      STRUCT ID LC DefList RC
    | STRUCT ID
    */
    void translateStructSpecifier(TreeNode *node)
    {
        if (DEBUG)
            cout << "StructSpecifier" << endl;

        //! not implemented
        if (node->child.size() == 5)
        {
            // STRUCT ID LC DefList RC
            translateDefList(node->child[3]);
        }
    }

    /*
    VarDec:
      ID
    | VarDec LB INT RB
    */
    string translateVarDec(TreeNode *node)
    {
        if (DEBUG)
            cout << "VarDec" << endl;

        if (node->child.size() == 4)
        {
            //! not implemented
            translateVarDec(node->child[0]);
            return "";
        }

        string varName = node->child[0]->data;
        if (vmap.count(varName) == 0)
        {
            vmap[varName] = createVar();
        }
        return vmap[varName];
    }

    /*
    FunDec:
      ID LP VarList RP
    | ID LP RP
    */
    void translateFunDec(TreeNode *node)
    {
        if (DEBUG)
            cout << "FunDec" << endl;

        code += "FUNCTION " + node->child[0]->data + " :" + "\n";

        if (node->child.size() == 4)
        {
            // ID LP VarList RP
            code += translateVarList(node->child[2]);
        }
    }

    /*
    VarList:
      ParamDec COMMA VarList
    | ParamDec
    */
    string translateVarList(TreeNode *node)
    {
        if (DEBUG)
            cout << "VarList" << endl;

        if (node->child.size() == 3)
        {
            // ParamDec COMMA VarList
            string code1 = translateParamDec(node->child[0]);
            string code2 = translateVarList(node->child[2]);
            return code1 + code2;
        }
        return translateParamDec(node->child[0]);
    }

    /*
    ParamDec:
      Specifier VarDec
    */
    string translateParamDec(TreeNode *node)
    {
        if (DEBUG)
            cout << "ParamDec" << endl;

        translateSpecifier(node->child[0]);

        string v = translateVarDec(node->child[1]);

        // dmap
        DNode *root = new DNode("param " + v);
        root->adj.push_back(dmap[v]);
        graphRoots.push_back(root);

        return "PARAM " + v + "\n";
    }

    /*
    CompSt:
      LC DefList StmtList RC
    */
    string translateCompSt(TreeNode *node)
    {
        if (DEBUG)
            cout << "Compst" << endl;

        string code1 = translateDefList(node->child[1]);
        string code2 = translateStmtList(node->child[2]);
        return code1 + code2;
    }

    /*
    StmtList:
      Stmt StmtList
    | %empty
    */
    string translateStmtList(TreeNode *node)
    {
        if (DEBUG)
            cout << "StmtList" << endl;

        if (node->child.empty())
        {
            return "";
        }
        // Stmt StmtList
        string code1 = translateStmt(node->child[0]);
        string code2 = translateStmtList(node->child[1]);
        return code1 + code2;
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
    string translateStmt(TreeNode *node)
    {
        if (DEBUG)
        {
            cout << "Stmt" << endl;
            cout << "  Stmt childs = " << node->child.size() << endl;
        }

        if (node->child.size() == 2)
        {
            // Exp SEMI
            return translateExp(node->child[0], "");
        }
        else if (node->child.size() == 1)
        {
            // CompSt
            return translateCompSt(node->child[0]);
        }
        else if (node->child.size() == 3)
        {
            // RETURN Exp SEMI
            if (DEBUG)
                cout << "    Return" << endl;
            if (node->child[1]->child.size() == 1)
            {
                string v = translateExp(node->child[1], "");

                // dmap
                if (v[0] != '#')
                {
                    DNode *root = new DNode("return " + v);
                    root->adj.push_back(dmap[v]);
                    graphRoots.push_back(root);
                }

                return "RETURN " + v + "\n";
            }

            string tp = createTemp();

            // dmap
            DNode *root = new DNode("return " + tp);
            root->adj.push_back(dmap[tp]);
            graphRoots.push_back(root);

            return translateExp(node->child[1], tp) + "RETURN " + tp + "\n";
        }
        else if (node->child.size() == 5)
        {
            // IF LP Exp RP Stmt
            if (node->child[0]->name == "IF")
            {
                string lb1 = createLabel();
                string lb2 = createLabel();
                string code1 = translateCondExp(node->child[2], lb1, lb2) + "LABEL " + lb1 + " :\n";
                string code2 = translateStmt(node->child[4]) + "LABEL " + lb2 + " :\n";
                return code1 + code2;
            }
            else // WHILE LP Exp RP Stmt
            {
                string lb1 = createLabel();
                string lb2 = createLabel();
                string lb3 = createLabel();
                string code1 = "LABEL " + lb1 + " :\n" + translateCondExp(node->child[2], lb2, lb3);
                string code2 = "LABEL " + lb2 + " :\n" + translateStmt(node->child[4]) + "GOTO " + lb1 + "\n";
                return code1 + code2 + "LABEL " + lb3 + " :\n";
            }
        }
        else
        {
            // IF LP Exp RP Stmt ELSE Stmt
            string lb1 = createLabel();
            string lb2 = createLabel();
            string lb3 = createLabel();
            string code1 = translateCondExp(node->child[2], lb1, lb2) + "LABEL " + lb1 + " :\n";
            string code2 = translateStmt(node->child[4]) + "GOTO " + lb3 + "\nLABEL " + lb2 + " :\n";
            string code3 = translateStmt(node->child[6]) + "LABEL " + lb3 + " :\n";
            return code1 + code2 + code3;
        }
    }

    /*
    DefList:
      Def DefList
    | %empty
    */
    string translateDefList(TreeNode *node)
    {
        if (DEBUG)
            cout << "DefList" << endl;

        if (node->child.size() == 2)
        {
            // Def DefList
            string code1 = translateDef(node->child[0]);
            string code2 = translateDefList(node->child[1]);
            return code1 + code2;
        }
        return "";
    }

    /*
    Def:
      Specifier DecList SEMI
    */
    string translateDef(TreeNode *node)
    {
        if (DEBUG)
            cout << "Def" << endl;

        translateSpecifier(node->child[0]);
        return translateDecList(node->child[1]);
    }

    /*
    DecList:
      Dec
    | Dec COMMA DecList
    */
    string translateDecList(TreeNode *node)
    {
        if (DEBUG)
            cout << "DecList" << endl;

        if (node->child.size() == 3)
        {
            string code1 = translateDec(node->child[0]);
            string code2 = translateDecList(node->child[2]);
            return code1 + code2;
        }
        return translateDec(node->child[0]);
    }

    /*
    Dec:
      VarDec
    | VarDec ASSIGN Exp
    */
    string translateDec(TreeNode *node)
    {
        if (DEBUG)
            cout << "Dec" << endl;

        string v = translateVarDec(node->child[0]);
        if (node->child.size() == 3)
        {
            // string tp = createTemp();
            return translateExp(node->child[2], v);
        }
        return "";
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
    string translateExp(TreeNode *node, string place)
    {
        if (DEBUG)
        {
            cout << "Exp" << endl;
            cout << "  Exp childs = " << node->child.size() << endl;
        }

        if (node->child.size() == 1)
        {
            if (node->child[0]->type == DataType::INT_TYPE)
            {
                if (DEBUG)
                {
                    cout << place + " := #" + to_string(strtol(node->child[0]->data.c_str(), NULL, 0)) + "\n";
                }

                if (place.length() == 0)
                {
                    return "#" + to_string(strtol(node->child[0]->data.c_str(), NULL, 0));
                }

                return place + " := #" + to_string(strtol(node->child[0]->data.c_str(), NULL, 0)) + "\n";
            }
            else
            { // ID
                if (place.length())
                {
                    // dmap
                    dmap[place]->adj.push_back(dmap[vmap[node->child[0]->data]]);
                }

                if (DEBUG)
                {
                    cout << place + " := " + vmap[node->child[0]->data] + "\n";
                }
                if (place.length())
                {
                    return place + " := " + vmap[node->child[0]->data] + "\n";
                }
                else
                {
                    return vmap[node->child[0]->data];
                }
            }
        }
        else if (node->child.size() == 2)
        {
            if (node->child[0]->name == "MINUS")
            {
                string name = createTemp();
                string code1 = translateExp(node->child[1], name);
                string code2 = place + " := #0 - " + name + "\n";
                if (DEBUG)
                {
                    cout << code1 + code2;
                }

                // dmap
                dmap[place]->adj.push_back(dmap[name]);

                return code1 + code2;
            }
        }
        else if (node->child.size() == 3)
        {
            if (node->child[1]->child.size() == 0 && node->child[2]->child.size() == 0)
            {
                if (node->child[0]->data == "read")
                {
                    if (DEBUG)
                    {
                        cout << "READ " + place + "\n";
                    }

                    return "READ " + place + "\n";
                }
                else
                {
                    if (DEBUG)
                    {
                        cout << place + " := CALL " + node->child[0]->data + "\n";
                    }

                    // dmap
                    graphRoots.push_back(dmap[place]);

                    return place + " := CALL " + node->child[0]->data + "\n";
                }
            }
            else if (node->child[1]->name == "ASSIGN")
            {
                string name = createTemp();
                string code1 = translateExp(node->child[2], name);
                string code2 = vmap[node->child[0]->child[0]->data] + " := " + name + "\n";
                string code3;
                if (place.length())
                {
                    code3 = place + " := " + vmap[node->child[0]->child[0]->data] + "\n";
                }
                if (DEBUG)
                {
                    cout << code1 + code2 + code3;
                }

                // dmap
                dmap[vmap[node->child[0]->child[0]->data]]->adj.push_back(dmap[name]);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "PLUS")
            {
                string name1 = createTemp();
                string name2 = createTemp();
                string code1 = translateExp(node->child[0], name1);
                string code2 = translateExp(node->child[2], name2);
                string code3 = place + " := " + name1 + " + " + name2 + "\n";
                if (DEBUG)
                {
                    cout << code1 + code2 + code3;
                }

                // dmap
                dmap[place]->adj.push_back(dmap[name1]);
                dmap[place]->adj.push_back(dmap[name2]);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "MINUS")
            {
                string name1 = createTemp();
                string name2 = createTemp();
                string code1 = translateExp(node->child[0], name1);
                string code2 = translateExp(node->child[2], name2);
                string code3 = place + " := " + name1 + " - " + name2 + "\n";
                if (DEBUG)
                {
                    cout << code1 + code2 + code3;
                }

                // dmap
                dmap[place]->adj.push_back(dmap[name1]);
                dmap[place]->adj.push_back(dmap[name2]);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "MUL")
            {
                string name1 = createTemp();
                string name2 = createTemp();
                string code1 = translateExp(node->child[0], name1);
                string code2 = translateExp(node->child[2], name2);
                string code3 = place + " := " + name1 + " * " + name2 + "\n";
                if (DEBUG)
                {
                    cout << code1 + code2 + code3;
                }

                // dmap
                dmap[place]->adj.push_back(dmap[name1]);
                dmap[place]->adj.push_back(dmap[name2]);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "DIV")
            {
                string name1 = createTemp();
                string name2 = createTemp();
                string code1 = translateExp(node->child[0], name1);
                string code2 = translateExp(node->child[2], name2);
                string code3 = place + " := " + name1 + " / " + name2 + "\n";
                if (DEBUG)
                {
                    cout << code1 + code2 + code3;
                }

                // dmap
                dmap[place]->adj.push_back(dmap[name1]);
                dmap[place]->adj.push_back(dmap[name2]);

                return code1 + code2 + code3;
            }
            else if (node->child[0]->name == "LP")
            {
                return translateExp(node->child[1], place);
            }
        }
        else
        {
            if (node->child[0]->data == "write")
            {
                string name = createTemp();
                if (DEBUG)
                {
                    cout << translateExp(node->child[2]->child[0], name) + "WRITE " + name + "\n";
                }

                // dmap
                DNode *root = new DNode("write " + name);
                root->adj.push_back(dmap[name]);
                graphRoots.push_back(root);

                return translateExp(node->child[2]->child[0], name) + "WRITE " + name + "\n";
            }
            else
            {
                deque<string> args;
                string code1 = translateArgs(node->child[2], args);
                string code2;
                for (auto arg : args)
                {
                    code2 += "ARG " + arg + "\n";
                }
                if (place.length() == 0)
                {
                    place = createTemp();
                }

                if (DEBUG)
                {
                    cout << code1 + code2 + place + " := CALL " + node->child[0]->data + "\n";
                }

                // dmap
                graphRoots.push_back(dmap[place]);

                return code1 + code2 + place + " := CALL " + node->child[0]->data + "\n";
            }
        }

        string label1 = createLabel();
        string label2 = createLabel();
        string code0 = place + " := #0\n";
        string code1 = translateCondExp(node, label1, label2);
        string code2 = "LABEL " + label1 + " :\n" + place + " := #1\nLABEL " + label2 + " :\n";
        if (DEBUG)
        {
            cout << code0 + code1 + code2;
        }
        return code0 + code1 + code2;
    }

    string translateCondExp(TreeNode *node, string label_true, string label_false)
    {
        if (DEBUG)
        {
            cout << "CondExp" << endl;
            cout << "  CondExp childs = " << node->child.size() << endl;
        }
        if (node->child.size() == 2)
        {
            return translateCondExp(node->child[1], label_false, label_true);
        }
        else
        {
            if (node->child[1]->name == "AND")
            {
                string label = createLabel();
                string code1 = translateCondExp(node->child[0], label, label_false) + "LABEL " + label + " :\n";
                string code2 = translateCondExp(node->child[2], label_true, label_false);
                return code1 + code2;
            }
            else if (node->child[1]->name == "OR")
            {
                string label = createLabel();
                string code1 = translateCondExp(node->child[0], label_true, label) + "LABEL " + label + " :\n";
                string code2 = translateCondExp(node->child[2], label_true, label_false);
                return code1 + code2;
            }
            else if (node->child[1]->name == "EQ")
            {
                string name1;
                string code1;
                if (node->child[0]->child.size() == 1)
                {
                    name1 = translateExp(node->child[0], "");
                }
                else
                {
                    name1 = createTemp();
                    code1 = translateExp(node->child[0], name1);
                }

                string name2;
                string code2;
                if (node->child[2]->child.size() == 1)
                {
                    name2 = translateExp(node->child[2], "");
                }
                else
                {
                    name2 = createTemp();
                    code2 = translateExp(node->child[2], name2);
                }

                string code3 = "IF " + name1 + " == " + name2 + " GOTO " + label_true + "\nGOTO " + label_false + "\n";

                // dmap
                DNode *root = new DNode("if " + name1 + node->child[1]->name + name2);
                if (name1[0] != '#')
                {
                    root->adj.push_back(dmap[name1]);
                }
                if (name2[0] != '#')
                {
                    root->adj.push_back(dmap[name2]);
                }
                graphRoots.push_back(root);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "NE")
            {
                string name1;
                string code1;
                if (node->child[0]->child.size() == 1)
                {
                    name1 = translateExp(node->child[0], "");
                }
                else
                {
                    name1 = createTemp();
                    code1 = translateExp(node->child[0], name1);
                }

                string name2;
                string code2;
                if (node->child[2]->child.size() == 1)
                {
                    name2 = translateExp(node->child[2], "");
                }
                else
                {
                    name2 = createTemp();
                    code2 = translateExp(node->child[2], name2);
                }

                string code3 = "IF " + name1 + " != " + name2 + " GOTO " + label_true + "\nGOTO " + label_false + "\n";

                // dmap
                DNode *root = new DNode("if " + name1 + node->child[1]->name + name2);
                if (name1[0] != '#')
                {
                    root->adj.push_back(dmap[name1]);
                }
                if (name2[0] != '#')
                {
                    root->adj.push_back(dmap[name2]);
                }
                graphRoots.push_back(root);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "GT")
            {
                string name1;
                string code1;
                if (node->child[0]->child.size() == 1)
                {
                    name1 = translateExp(node->child[0], "");
                }
                else
                {
                    name1 = createTemp();
                    code1 = translateExp(node->child[0], name1);
                }

                string name2;
                string code2;
                if (node->child[2]->child.size() == 1)
                {
                    name2 = translateExp(node->child[2], "");
                }
                else
                {
                    name2 = createTemp();
                    code2 = translateExp(node->child[2], name2);
                }

                string code3 = "IF " + name1 + " > " + name2 + " GOTO " + label_true + "\nGOTO " + label_false + "\n";

                // dmap
                DNode *root = new DNode("if " + name1 + node->child[1]->name + name2);
                if (name1[0] != '#')
                {
                    root->adj.push_back(dmap[name1]);
                }
                if (name2[0] != '#')
                {
                    root->adj.push_back(dmap[name2]);
                }
                graphRoots.push_back(root);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "GE")
            {
                string name1;
                string code1;
                if (node->child[0]->child.size() == 1)
                {
                    name1 = translateExp(node->child[0], "");
                }
                else
                {
                    name1 = createTemp();
                    code1 = translateExp(node->child[0], name1);
                }

                string name2;
                string code2;
                if (node->child[2]->child.size() == 1)
                {
                    name2 = translateExp(node->child[2], "");
                }
                else
                {
                    name2 = createTemp();
                    code2 = translateExp(node->child[2], name2);
                }

                string code3 = "IF " + name1 + " >= " + name2 + " GOTO " + label_true + "\nGOTO " + label_false + "\n";

                // dmap
                DNode *root = new DNode("if " + name1 + node->child[1]->name + name2);
                if (name1[0] != '#')
                {
                    root->adj.push_back(dmap[name1]);
                }
                if (name2[0] != '#')
                {
                    root->adj.push_back(dmap[name2]);
                }
                graphRoots.push_back(root);

                return code1 + code2 + code3;
            }
            else if (node->child[1]->name == "LT")
            {
                string name1;
                string code1;
                if (node->child[0]->child.size() == 1)
                {
                    name1 = translateExp(node->child[0], "");
                }
                else
                {
                    name1 = createTemp();
                    code1 = translateExp(node->child[0], name1);
                }

                string name2;
                string code2;
                if (node->child[2]->child.size() == 1)
                {
                    name2 = translateExp(node->child[2], "");
                }
                else
                {
                    name2 = createTemp();
                    code2 = translateExp(node->child[2], name2);
                }

                string code3 = "IF " + name1 + " < " + name2 + " GOTO " + label_true + "\nGOTO " + label_false + "\n";

                // dmap
                DNode *root = new DNode("if " + name1 + node->child[1]->name + name2);
                if (name1[0] != '#')
                {
                    root->adj.push_back(dmap[name1]);
                }
                if (name2[0] != '#')
                {
                    root->adj.push_back(dmap[name2]);
                }
                graphRoots.push_back(root);

                return code1 + code2 + code3;
            }
            else
            {
                string name1;
                string code1;
                if (node->child[0]->child.size() == 1)
                {
                    name1 = translateExp(node->child[0], "");
                }
                else
                {
                    name1 = createTemp();
                    code1 = translateExp(node->child[0], name1);
                }

                string name2;
                string code2;
                if (node->child[2]->child.size() == 1)
                {
                    name2 = translateExp(node->child[2], "");
                }
                else
                {
                    name2 = createTemp();
                    code2 = translateExp(node->child[2], name2);
                }

                string code3 = "IF " + name1 + " <= " + name2 + " GOTO " + label_true + "\nGOTO " + label_false + "\n";

                // dmap
                DNode *root = new DNode("if " + name1 + node->child[1]->name + name2);
                if (name1[0] != '#')
                {
                    root->adj.push_back(dmap[name1]);
                }
                if (name2[0] != '#')
                {
                    root->adj.push_back(dmap[name2]);
                }
                graphRoots.push_back(root);

                return code1 + code2 + code3;
            }
        }
    }

    /*
    Args:
      Exp COMMA Args
    | Exp
    */
    string translateArgs(TreeNode *node, deque<string> &argList)
    {
        if (DEBUG)
            cout << "Args" << endl;

        if (node->child.size() == 1)
        {
            string v = createVar();
            argList.push_front(v);

            // dmap
            DNode *root = new DNode("arg " + v);
            root->adj.push_back(dmap[v]);
            graphRoots.push_back(root);

            return translateExp(node->child[0], v);
        }
        else
        {
            // Exp COMMA Args
            string v = createVar();
            argList.push_front(v);

            // dmap
            DNode *root = new DNode("arg " + v);
            root->adj.push_back(dmap[v]);
            graphRoots.push_back(root);

            string code1 = translateExp(node->child[0], v);
            string code2 = translateArgs(node->child[2], argList);
            return code1 + code2;
        }
    }

    vector<string> simplify()
    {
        while (graphRoots.size() > 0)
        {
            DNode *curr = graphRoots.front();
            graphRoots.pop_front();
            if (curr->visited)
            {
                continue;
            }
            curr->visited = true;
            for (auto subnode : curr->adj)
            {
                if (!subnode->visited)
                {
                    graphRoots.push_back(subnode);
                }
            }
        }

        vector<string> uselessNode;

        for (auto item : dmap)
        {
            if (!item.second->visited)
            {
                uselessNode.push_back(item.first);
            }
        }

        vector<string> resultLines;
        istringstream in(code);
        string line;
        while (getline(in, line))
        {
            if (!contains(line, uselessNode))
            {
                resultLines.push_back(line);
            }
        }

        return resultLines;
    }

    bool contains(string line, vector<string> &uselessNode)
    {
        istringstream strin(line);
        string subs;
        while ((strin >> subs))
        {
            for (auto item : uselessNode)
            {
                if (subs == item)
                {
                    return true;
                }
            }
        }
        return false;
    }
};

string generateIntermidiateCode(char *file_path)
{
    function_init();
    analysisTreeNode(file_path);
    if (!has_error)
    {
        string path = file_path;
        string out_path = "/dev/stdout";
        if (path.substr(path.length() - 4) == ".spl")
        {
            out_path = path.substr(0, path.length() - 4) + ".ir";
        }
        Generator(root, out_path).generate();
        return out_path;
    }
    return "";
}

#endif