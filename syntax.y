%{
    #include "lex.yy.c"
    #include <string.h>
    #define YYSTYPE TreeNode *
    #include "AssemblyGenerator.hpp"
    void yyerror(const char*);
%}

%token INT FLOAT CHAR ID
%token TYPE
%token STRUCT
%token IF ELSE
%token WHILE
%token RETURN
%token DOT SEMI COMMA
%token ASSIGN
%token LT LE GT GE NE EQ
%token PLUS MINUS MUL DIV
%token AND OR NOT
%token LP RP LB RB LC RC

%right ASSIGN
%left OR
%left AND
%left GT GE LT LE EQ NE
%left PLUS MINUS
%left MUL DIV
%right NOT
%left LC RC LB RB DOT

%nonassoc LOWER_ELSE
%nonassoc ELSE

%%
Program: 
      ExtDefList                            { $$ = create_child_node("Program", @$.first_line, {$1}); root = $$; }
    ;
ExtDefList: 
      ExtDef ExtDefList                     { $$ = create_child_node("ExtDefList", @$.first_line, {$1, $2}); }
    | %empty                                { $$ = create_child_node("ExtDefList", @$.first_line, {}); }
    ;   
ExtDef: 
      Specifier ExtDecList SEMI             { $$ = create_child_node("ExtDef", @$.first_line, {$1, $2, $3}); }
    | Specifier SEMI                        { $$ = create_child_node("ExtDef", @$.first_line, {$1, $2}); }
    | Specifier FunDec CompSt               { $$ = create_child_node("ExtDef", @$.first_line, {$1, $2, $3}); }
    | Specifier ExtDecList error            { fprintf(output_file, "Error type B at Line %d: Missing semicolon ';'\n", @$.first_line); has_error = 1; }
    | Specifier error                       { fprintf(output_file, "Error type B at Line %d: Missing semicolon ';'\n", @$.first_line); has_error = 1; }
    | error ExtDecList SEMI                 { fprintf(output_file, "Error type B at Line %d: Missing specifier\n", @$.first_line); has_error = 1; }
    | error SEMI                            { fprintf(output_file, "Error type B at Line %d: Missing specifier\n", @$.first_line); has_error = 1; }
    ;
ExtDecList: 
      VarDec                                { $$ = create_child_node("ExtDecList", @$.first_line, {$1}); }
    | VarDec COMMA ExtDecList               { $$ = create_child_node("ExtDecList", @$.first_line, {$1, $2, $3}); }
    | VarDec ExtDecList error               { fprintf(output_file, "Error type B at Line %d: Missing comma ','\n", @$.first_line); has_error = 1; }
    ;

Specifier: 
      TYPE                                  { $$ = create_child_node("Specifier", @$.first_line, {$1}); }
    | StructSpecifier                       { $$ = create_child_node("Specifier", @$.first_line, {$1}); }
    ;
StructSpecifier: 
      STRUCT ID LC DefList RC               { $$ = create_child_node("StructSpecifier", @$.first_line, {$1, $2, $3, $4, $5}); }
    | STRUCT ID                             { $$ = create_child_node("StructSpecifier", @$.first_line, {$1, $2}); }
    | STRUCT ID LC DefList error            { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis '}'\n", @$.first_line); has_error = 1; }
    ;

VarDec:
      ID                                    { $$ = create_child_node("VarDec", @$.first_line, {$1}); }
    | VarDec LB INT RB                      { $$ = create_child_node("VarDec", @$.first_line, {$1, $2, $3, $4}); }
    | VarDec LB INT error                   { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ']'\n", @$.first_line); has_error = 1; }
    ;
FunDec: 
      ID LP VarList RP                      { $$ = create_child_node("FunDec", @$.first_line, {$1, $2, $3, $4}); }
    | ID LP RP                              { $$ = create_child_node("FunDec", @$.first_line, {$1, $2, $3}); }
    | ID LP VarList error                   { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    | ID LP error                           { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    ;
VarList: 
      ParamDec COMMA VarList                { $$ = create_child_node("VarList", @$.first_line, {$1, $2, $3}); }
    | ParamDec VarList error                { fprintf(output_file, "Error type B at Line %d: Missing comma ','\n", @$.first_line); has_error = 1; }
    | ParamDec                              { $$ = create_child_node("VarList", @$.first_line, {$1}); }
    ;
ParamDec: 
      Specifier VarDec                      { $$ = create_child_node("ParamDec", @$.first_line, {$1, $2}); }
    | error VarDec                          { fprintf(output_file, "Error type B at Line %d: Missing specifier\n", @$.first_line); has_error = 1; }
    ;

CompSt: 
      LC DefList StmtList RC                { $$ = create_child_node("CompSt", @$.first_line, {$1, $2, $3, $4}); }
    ;
StmtList: 
      Stmt StmtList                         { $$ = create_child_node("StmtList", @$.first_line, {$1, $2}); }
    | %empty                                { $$ = create_child_node("StmtList", @$.first_line, {}); }
    ;
Stmt: 
      Exp SEMI                              { $$ = create_child_node("Stmt", @$.first_line, {$1, $2}); }
    | CompSt                                { $$ = create_child_node("Stmt", @$.first_line, {$1}); }
    | RETURN Exp SEMI                       { $$ = create_child_node("Stmt", @$.first_line, {$1, $2, $3}); }
    | IF LP Exp RP Stmt %prec LOWER_ELSE    { $$ = create_child_node("Stmt", @$.first_line, {$1, $2, $3, $4, $5}); }
    | IF LP Exp RP Stmt ELSE Stmt           { $$ = create_child_node("Stmt", @$.first_line, {$1, $2, $3, $4, $5, $6, $7}); }
    | WHILE LP Exp RP Stmt                  { $$ = create_child_node("Stmt", @$.first_line, {$1, $2, $3, $4, $5}); }
    | Exp error                             { fprintf(output_file, "Error type B at Line %d: Missing semicolon ';'\n", @$.first_line); has_error = 1; }
    | RETURN Exp error                      { fprintf(output_file, "Error type B at Line %d: Missing semicolon ';'\n", @$.first_line); has_error = 1; }
    | IF LP Exp error Stmt %prec LOWER_ELSE { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    | IF LP Exp error Stmt ELSE Stmt        { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    | WHILE LP Exp error Stmt               { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    ;
DefList: 
      Def DefList                           { $$ = create_child_node("DefList", @$.first_line, {$1, $2}); }
    | %empty                                { $$ = create_child_node("DefList", @$.first_line, {}); }
    ;
Def: 
      Specifier DecList SEMI                { $$ = create_child_node("Def", @$.first_line, {$1, $2, $3}); }
    | Specifier DecList error               { fprintf(output_file, "Error type B at Line %d: Missing semicolon ';'\n", @$.first_line); has_error = 1; }
    | error DecList SEMI                    { fprintf(output_file, "Error type B at Line %d: Missing specifier\n", @$.first_line); has_error = 1; }
    ;
DecList: 
      Dec                                   { $$ = create_child_node("DecList", @$.first_line, {$1}); }
    | Dec COMMA DecList                     { $$ = create_child_node("DecList", @$.first_line, {$1, $2, $3}); }
    | Dec DecList error                     { fprintf(output_file, "Error type B at Line %d: Missing comma ','\n", @$.first_line); has_error = 1; }
    ;
Dec: 
      VarDec                                { $$ = create_child_node("Dec", @$.first_line, {$1}); }
    | VarDec ASSIGN Exp                     { $$ = create_child_node("Dec", @$.first_line, {$1, $2, $3}); }
    ;

Exp: 
      Exp ASSIGN Exp                        { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp AND Exp                           { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp OR Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp LT Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp LE Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp GT Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp GE Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp NE Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp EQ Exp                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp PLUS Exp                          { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp MINUS Exp                         { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp MUL Exp                           { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp DIV Exp                           { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | LP Exp RP                             { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | MINUS Exp                             { $$ = create_child_node("Exp", @$.first_line, {$1, $2}); }
    | NOT Exp                               { $$ = create_child_node("Exp", @$.first_line, {$1, $2}); }
    | ID LP Args RP                         { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3, $4}); }
    | ID LP RP                              { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | Exp LB Exp RB                         { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3, $4}); }
    | Exp DOT ID                            { $$ = create_child_node("Exp", @$.first_line, {$1, $2, $3}); }
    | ID                                    { $$ = create_child_node("Exp", @$.first_line, {$1}); }
    | INT                                   { $$ = create_child_node("Exp", @$.first_line, {$1}); }
    | FLOAT                                 { $$ = create_child_node("Exp", @$.first_line, {$1}); }
    | CHAR                                  { $$ = create_child_node("Exp", @$.first_line, {$1}); }
    | LP Exp error                          { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    | ID LP Args error                      { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    | ID LP error                           { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ')'\n", @$.first_line); has_error = 1; }
    | Exp LB Exp error                      { fprintf(output_file, "Error type B at Line %d: Missing closing parenthesis ']'\n", @$.first_line); has_error = 1; }
    ;
Args: 
      Exp COMMA Args                        { $$ = create_child_node("Args", @$.first_line, {$1, $2, $3}); }
    | Exp                                   { $$ = create_child_node("Args", @$.first_line, {$1}); }
    ;

%%
void yyerror(const char *s) {}

int main(int argc, char **argv){
    has_error = 0;

    char *file_path;
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAIL;
    } else if(argc == 2){
        file_path = argv[1];
        if(!(yyin = fopen(file_path, "r"))){
            perror(argv[1]);
            return EXIT_FAIL;
        }
        int len = strlen(file_path);
        if (strcmp(file_path+len-4, ".spl")==0)
        {
            char out[100];
            strcpy(out, file_path);
            out[len-3]='o';
            out[len-2]='u';
            out[len-1]='t';
            output_file = fopen(out, "w+");
        }
        yyparse();
        if (has_error) {
            fflush(output_file);
            fclose(output_file);
        }
        else
        {
            fclose(output_file);
            generateAssemblyCode(file_path);
        }
        return 0;
    } else{
        fputs("Too many arguments! Expected: 2.\n", stderr);
        return 1;
    }
}