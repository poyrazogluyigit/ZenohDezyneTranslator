%{
#include <iostream>
#include <string>
#include "parser.hpp"
%}

%option yylineno
%option noyywrap

%%

[ \t\r\n]+  { /* Skip whitespace */ }

"DSL_DECLARE_ENUM"          { return TOK_DECLARE_ENUM; }
"DSL_ENUM_VAR"              { return TOK_ENUM_VAR; }
"DSL_SUBINT"                { return TOK_SUBINT; }
"DSL_BOOL"                  { return TOK_BOOL; }
"DSL_IF"                    { return TOK_GUARD; }
"DSL_ELSE_IF"               { return TOK_GUARD; }
"DSL_SET"                   { return TOK_SET; }
"ZENOH_DECLARE_SUBSCRIBER"  { return TOK_DECLARE_SUB; }
"ZENOH_DECLARE_PUBLISHER"   { return TOK_DECLARE_PUB; }
"ZENOH_PUT"                 { return TOK_PUT; }

"BEGIN"                     { return TOK_BEGIN; }
"END"                       { return TOK_END; }

"true"                      { return KW_TRUE; }
"false"                     { return KW_FALSE; }

    /* --- 2. STRUCTURE (We need these to track nesting) --- */
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
"["         { return LBRACKET; }
"]"         { return RBRACKET; }
";"         { return SEMICOLON; }
","         { return COMMA; }

"=="        { return EQ; }        
"!="        { return NEQ; }
"||"        { return OR; }
"&&"        { return AND; }        
"<"         { return LT; }    
"<="        { return LEQ; }
">"         { return GT; }
">="        { return GEQ; }        
"!"         { return NOT; }

"="         { return ASGN; }
"::"        { return DCOL; }
"+"         { return PLUS; }
"-"         { return MINUS; }

    /* Matches string literals */
\"[^"]*\"   { yylval.sval = new std::string(yytext, yyleng); return STR_LIT; }

[a-zA-Z_][a-zA-Z0-9_]*          { yylval.sval = new std::string(yytext, yyleng); return IDENT; }
[0-9]+                          { yylval.sval = new std::string(yytext, yyleng); return NUMBER; }

    /* Matches any single character that isn't one of the above.
       This automatically handles ::, <, >, &, *, [, ], =, etc. 
       We treat them all as generic text chunks. */
.           { yylval.sval = new std::string(yytext, yyleng); return TOK_TEXT; }
[a-zA-Z0-9_]+ { yylval.sval = new std::string(yytext, yyleng); return TOK_TEXT; }

%%