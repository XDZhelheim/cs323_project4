%{
    extern "C" int yylex();
    #include "TreeNode.hpp"
    #define YYSTYPE TreeNode *
    #include "syntax.tab.h"

    #define EXIT_OK 0
    #define EXIT_FAIL 1

    int yycolno = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylineno; \
        yylloc.first_column = yycolno; \
        yylloc.last_line = yylineno; \
        yylloc.last_column = yycolno + yyleng; \
        yycolno += yyleng;
%}

%option yylineno

letter [a-zA-Z]
letter_ {letter}|_
digit [0-9]
hexdigit [0-9a-fA-F]

%%
0x{hexdigit}+|{digit}+    { yylval = create_node("INT", yylloc.first_line, DataType::INT_TYPE, yytext); return INT; }
{digit}+\.{digit}+               { yylval = create_node("FLOAT", yylloc.first_line, DataType::FLOAT_TYPE, yytext); return FLOAT; }
\'([\x20-\x7E]|\\x{hexdigit}{1,2})\'    { yylval = create_node("CHAR", yylloc.first_line, DataType::CHAR_TYPE, yytext); return CHAR; }

int|float|char  { yylval = create_node("TYPE", yylloc.first_line, DataType::TYPE_TYPE, yytext); return TYPE; }
struct          { yylval = create_node("STRUCT", yylloc.first_line); return STRUCT; }
if              { yylval = create_node("IF", yylloc.first_line); return IF; }
else            { yylval = create_node("ELSE", yylloc.first_line); return ELSE; }
while           { yylval = create_node("WHILE", yylloc.first_line); return WHILE; }
return          { yylval = create_node("RETURN", yylloc.first_line); return RETURN; }

{letter_}({letter_}|{digit})*           { yylval = create_node("ID", yylloc.first_line, DataType::ID_TYPE, yytext); return ID; }

"."     { yylval = create_node("DOT", yylloc.first_line); return DOT; }
";"     { yylval = create_node("SEMI", yylloc.first_line); return SEMI; }
","     { yylval = create_node("COMMA", yylloc.first_line); return COMMA; }
"="     { yylval = create_node("ASSIGN", yylloc.first_line); return ASSIGN; }
"<"     { yylval = create_node("LT", yylloc.first_line); return LT; }
"<="    { yylval = create_node("LE", yylloc.first_line); return LE; }
">"     { yylval = create_node("GT", yylloc.first_line); return GT; }
">="    { yylval = create_node("GE", yylloc.first_line); return GE; }
"!="    { yylval = create_node("NE", yylloc.first_line); return NE; }
"=="    { yylval = create_node("EQ", yylloc.first_line); return EQ; }
"+"     { yylval = create_node("PLUS", yylloc.first_line); return PLUS; }
"-"     { yylval = create_node("MINUS", yylloc.first_line); return MINUS; }
"*"     { yylval = create_node("MUL", yylloc.first_line); return MUL; }
"/"     { yylval = create_node("DIV", yylloc.first_line); return DIV; }
"&&"    { yylval = create_node("AND", yylloc.first_line); return AND; }
"||"    { yylval = create_node("OR", yylloc.first_line); return OR; }
"!"     { yylval = create_node("NOT", yylloc.first_line); return NOT; }
"("     { yylval = create_node("LP", yylloc.first_line); return LP; }
")"     { yylval = create_node("RP", yylloc.first_line); return RP; }
"["     { yylval = create_node("LB", yylloc.first_line); return LB; }
"]"     { yylval = create_node("RB", yylloc.first_line); return RB; }
"{"     { yylval = create_node("LC", yylloc.first_line); return LC; }
"}"     { yylval = create_node("RC"); return RC; }

"&" { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return AND; 
}
"|" { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return OR; 
}

0x{hexdigit}*[g-zG-Z]+{hexdigit}* { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return INT; 
}

\'\\x{hexdigit}*[g-zG-Z]+{hexdigit}*\' { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return CHAR; 
}

\'\\x{hexdigit}{3,}\' { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return CHAR; 
}

[0-9]+{letter_}({letter_}|{digit})* { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return ID; 
}

"//" {
    while(yyinput() != '\n');
}

"/*" {
    char c = 0;
    char buffer;
    while ((buffer = yyinput())) {
        if (c == '*' && buffer == '/') {
            yycolno += 2;
            break;
        }
        if (c == '\n') {
            yycolno = 1; // yylineno will still auto-increase
        } else {
            yycolno++;
        }
        c = buffer;
    }
    if (buffer == 0) {
        fprintf(output_file, "Error type B at Line %d: Missing closing symbol \"*/\"\n", yylineno);
        has_error = 1;
    }
}

"\n" { yycolno = 1; }
[ \r\t] {}

. { 
    fprintf(output_file, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); 
    has_error = 1; 
    yylval = create_node("ERR"); 
    return CHAR; 
}

%%