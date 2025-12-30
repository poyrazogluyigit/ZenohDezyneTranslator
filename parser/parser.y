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

/* Operators */
%token PLUS MINUS
%token EQ NEQ GT LT LEQ GEQ OR AND NOT
%token ASGN DCOL

/* Structure */
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA

/* Values */
%token <sval> IDENT TOK_TEXT
%token NUMBER

/* Precedence (Resolves Operator Shift/Reduce) */
%left OR
%left AND
%left EQ NEQ
%left LT GT LEQ GEQ
%left PLUS MINUS
%right NOT

%%

program:
    statements
    ;

statements:
    /* Empty */
    | statements statement
    ;

statement:
    /* 1. Macros */
    dsl_macro
    /* 2. Blocks */
    | block
    /* 3. Ignored C++ "Water" - Explicitly listing tokens to skip */
    | TOK_TEXT | IDENT | NUMBER | KW_TRUE | KW_FALSE
    | SEMICOLON | COMMA | LPAREN | RPAREN 
    | PLUS | MINUS | EQ | NEQ | GT | LT | LEQ | GEQ | OR | AND | NOT | DCOL | ASGN
    ;

block:
    LBRACE statements RBRACE
    ;

dsl_macro:
    /* SEMICOLON CONFLICT FIX: 
       We removed 'opt_semicolon' from all these rules. 
       The parser finishes the macro at ')'. 
       The semicolon that follows is caught by the 'statement: SEMICOLON' rule above. */

    TOK_DECLARE_ENUM LPAREN args RPAREN
    { log_match("Enum Declaration"); }

    | TOK_ENUM_VAR LPAREN args RPAREN
    { log_match("Enum Variable"); }

    | TOK_SUBINT LPAREN args RPAREN
    { log_match("SubInt"); }

    | TOK_SET LPAREN args RPAREN
    { log_match("Set Operation"); }

    | TOK_DECLARE_PUB LPAREN args RPAREN
    { log_match("Publisher Declaration"); }
    
    | TOK_PUT LPAREN args RPAREN
    { log_match("Zenoh Put"); }

    /* Guard: Uses unified expression */
    | TOK_GUARD LPAREN expression RPAREN statement
    { log_match("Guard"); }

    /* Subscriber: Uses skip_lambda */
    | TOK_DECLARE_SUB LPAREN args RPAREN skip_lambda block
    { log_match("Subscriber with Body"); }
    ;

/* --- UNIFIED EXPRESSION PARSING (Fixes Reduce/Reduce & Operator Conflicts) --- */
expression:
    /* Binary Operations */
      expression OR expression
    | expression AND expression
    | expression EQ expression
    | expression NEQ expression
    | expression LT expression
    | expression GT expression
    | expression LEQ expression
    | expression GEQ expression
    | expression PLUS expression
    | expression MINUS expression
    /* Unary */
    | NOT expression
    /* Grouping */
    | LPAREN expression RPAREN
    /* Terminals */
    | IDENT
    | NUMBER
    | KW_TRUE | KW_FALSE
    /* Complex IDs (State::A) */
    | IDENT DCOL IDENT
    /* Assignment (x = 5) */
    | IDENT ASGN expression
    /* Increment (x++) handled as generic text or specialized rule */
    | IDENT PLUS PLUS 
    ;

/* --- SKIP LAMBDA LOGIC --- */
skip_lambda:
    /* empty */
    | skip_lambda junk_token
    ;

junk_token:
    TOK_TEXT | IDENT | NUMBER | KW_TRUE | KW_FALSE
    | LPAREN | RPAREN | SEMICOLON | COMMA
    | PLUS | MINUS | EQ | NEQ | GT | LT | LEQ | GEQ | OR | AND | NOT | DCOL | ASGN
    ;

/* --- ARGUMENTS --- */
args:
    /* empty */
    | args_list
    ;

args_list:
    arg_item
    | args_list COMMA arg_item
    ;

arg_item:
    arg_token 
    | arg_item arg_token
    ;

arg_token:
    TOK_TEXT | IDENT | NUMBER | KW_TRUE | KW_FALSE
    | PLUS | MINUS | EQ | NEQ | GT | LT | LEQ | GEQ | OR | AND | NOT 
    | ASGN 
    | DCOL                 /* <--- ALLOW :: FREELY HERE */
    | LPAREN args RPAREN   /* Handle nested parens */
    ;

%%

int main() {
    yyparse();
    return 0;
}