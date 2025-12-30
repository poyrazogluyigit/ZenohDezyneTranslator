%{
#include <iostream>
#include <string>
#include <vector>

extern int yylineno;
extern int yylex();
extern char* yytext;

void yyerror(const char *s) {
    std::cerr << "Parser Error at line " << yylineno << ": " << s << " (Token: " << yytext << ")" << std::endl;
}

void log_match(const std::string& type) {
    std::cout << "[Line " << yylineno << "] Found " << type << std::endl;
}
%}

%union {
    char *sval;
}

/* Macros */
%token <sval> TOK_DECLARE_ENUM TOK_DECLARE_SUB TOK_DECLARE_PUB
%token <sval> TOK_ENUM_VAR TOK_SUBINT TOK_GUARD TOK_SET TOK_PUT

/* Keywords */
%token KW_TRUE KW_FALSE

/* Arithmetic Operators */
%token PLUS MINUS

/* Boolean Operators */
%token EQ NEQ GT LT LEQ GEQ OR AND
%token NOT

%left OR
%left AND
%left EQ NEQ
%left LT GT LEQ GEQ

/* Structure */
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA

/* Text related tokens */
%token <sval> IDENT
%token <sval> TOK_TEXT
%token NUMBER
%token DCOL

%%

program:
    statements
    ;

statements:
    /* Empty */
    | statements statement
    ;

statement:
    /* The specific Macros we care about */
    dsl_macro
    /* Structural blocks (to handle recursion) */
    | block
    /* Standard C++ code we want to skip */
    | TOK_TEXT       { /* Ignore */ }
    | SEMICOLON      { /* Ignore */ }
    | COMMA          { /* Ignore */ }
    | LPAREN         { /* Ignore loose parens in C++ code */ }
    | RPAREN         { /* Ignore */ }
    ;


block:
    LBRACE statements RBRACE
    ;

/* --- MACRO DEFINITIONS --- */

dsl_macro:
    TOK_DECLARE_ENUM LPAREN args RPAREN opt_semicolon
    { log_match("Enum Declaration"); }

    | TOK_ENUM_VAR LPAREN args RPAREN opt_semicolon
    { log_match("Enum Variable"); }

    | TOK_SUBINT LPAREN args RPAREN opt_semicolon
    { log_match("SubInt"); }

    | TOK_SET LPAREN args RPAREN opt_semicolon
    { log_match("Set Operation"); }

    | TOK_DECLARE_PUB LPAREN args RPAREN opt_semicolon
    { log_match("Publisher Declaration"); }
    
    | TOK_PUT LPAREN args RPAREN opt_semicolon
    { log_match("Zenoh Put"); }

    /* Guard: followed by a statement (which could be a block or single line) */
    | TOK_GUARD LPAREN bool_expression RPAREN statement
    { log_match("Guard"); }

    /* Subscriber: Macro -> (Ignored Lambda Capture/Args) -> Body Block */
    | TOK_DECLARE_SUB LPAREN args RPAREN ignored_cpp block opt_semicolon
    { log_match("Subscriber with Body"); }
    ;

/* GUARD PARSING */
bool_expression: 
    bool_literal
    | variable
    | enum_comparison
    | NOT bool_expression
    | LPAREN bool_expression RPAREN
    | bool_expression bool_operator bool_expression
    | int_expression comparison_operator int_expression
    ;

int_expression:
    NUMBER
    | variable
    | MINUS int_expression
    | PLUS int_expression
    | int_expression int_operator int_expression

int_operator:
    PLUS | MINUS
    ;

enum_comparison:
    variable EQ IDENT DCOL IDENT
    | variable NEQ IDENT DCOL IDENT
    ;

variable:
    IDENT
    ;

bool_literal:
    KW_FALSE | KW_TRUE
    ;

bool_operator:
    EQ | NEQ | OR | AND
    ;

comparison_operator:
    EQ | NEQ | GT | LT | GEQ | LEQ
    ;



/* --- ARGUMENTS --- */
/* We just gobble up anything balanced inside parens */
args:
    /* Empty args allowed */
    | args_list
    ;

args_list:
    arg_item
    | args_list COMMA arg_item
    ;

arg_item:
    /* An argument is any sequence of Text or Nested Parentheses */
    complex_text
    ;

complex_text:
    TOK_TEXT
    | complex_text TOK_TEXT
    | LPAREN args RPAREN  /* Handle nested parens (x + (y*z)) */
    | complex_text LPAREN args RPAREN
    ;

/* --- IGNORING C++ --- */
/* This rule consumes everything that isn't a DSL macro or a block delimiter */
ignored_cpp:
    TOK_TEXT
    | SEMICOLON
    | COMMA
    | LPAREN args RPAREN /* Skip function calls/definitions in C++ */
    | ignored_cpp TOK_TEXT
    | ignored_cpp SEMICOLON
    | ignored_cpp COMMA
    | ignored_cpp LPAREN args RPAREN
    ;

opt_semicolon:
    /* empty */
    | SEMICOLON
    ;

%%

int main() {
    yyparse();
    return 0;
}