%{
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <functional>
#include <fstream>

extern int yylineno;
extern int yylex();
extern char* yytext;

extern FILE* yyin;
extern void yyrestart(FILE*);

std::string current_filename;

std::unordered_set<std::string> symbolTable;
std::vector<std::string> behaviors;

std::map<std::string, std::string> publishers;
std::map<std::string, std::string> subscribers;

std::hash<std::string> strHasher;

std::string topic_to_event(std::string topicName){
    // Remove double quotes
    topicName = topicName.substr(1, topicName.size() - 2);
    // Replace all '/' with '_'
    std::replace(topicName.begin(), topicName.end(), '/', '_');
    return topicName;
}


void yyerror(const char *s) {
    std::cerr << "Parser Error at line " << yylineno << ": " << s << " (Token: " << yytext << ")" << std::endl;
}

void log_match(const std::string& type) {
    std::cout << "[Line " << yylineno << "] Found " << type << std::endl;
}
%}

%code requires {
    #include "Statement.h"
}

%union {
    std::string* sval;
    SingleStmt* sstmt;
    CompoundStmt* cstmt;
    GuardStmt* gstmt;
}

%type <sstmt> dsl_macro
%type <cstmt> statements invalid_statement valid_statement scoped_block

%type <sval> expression assignment enum_args enum_arg

/* Macros */
%token TOK_DECLARE_ENUM TOK_DECLARE_SUB TOK_DECLARE_PUB
%token TOK_ENUM_VAR TOK_SUBINT TOK_BOOL TOK_GUARD TOK_SET TOK_PUT

/* Keywords */
%token KW_TRUE KW_FALSE

/* Operators */
%token PLUS MINUS
%token EQ NEQ GT LT LEQ GEQ OR AND NOT
%token ASGN DCOL

/* Structure */
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET SEMICOLON COMMA TOK_BEGIN TOK_END

/* Values */
%token <sval> IDENT TOK_TEXT STR_LIT
%token <sval> NUMBER

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
    {
        size_t hashedFilename = strHasher(current_filename);
        // TODO output dir etc.
        std::string out_filename = "../generate/file_" + std::to_string(hashedFilename) + ".dzn";
        std::ofstream outfile(out_filename);
        if (!outfile.is_open()) {
            std::cerr << "Error: Could not open output file: " << out_filename << std::endl;
            YYABORT;
        }
        // print the beginning
        outfile << "interface file_" << hashedFilename << "{" << std::endl;
        // print the in/out events
        for (const auto &[key, val] : publishers) {
            outfile << "out void " + val + "();" << std::endl;
        }
        for (const auto& [key, val] : subscribers) {
            outfile << "in void " + val + "();" << std::endl;
        }
        outfile << std::endl; 
        // print the statements
        for (auto stmt : $1->_stmts) 
            if (stmt != "")
                outfile << stmt; 
        delete $1;
        outfile << std::endl;
        outfile << "behavior {" << std::endl;
        for (auto beh : behaviors) outfile << beh;
        outfile << "}" << std::endl;
        // close the interface
        outfile << "}";
        outfile.close();
    }
    ;


statements:
    /* Empty */
    { $$ = new CompoundStmt(); }
    | statements dsl_macro
    { $$ = $1; $$->addStmt(*$2); }
    | statements invalid_statement
    { $$ = $1; }
    ;

// statement:
//     valid_statement
//     { $$ = $1; }
//     | invalid_statement
//     { $$ = $1; }

valid_statement:
    dsl_macro
    { $$ = new CompoundStmt(*$1); delete $1; }
    | scoped_block
    { $$ = $1; }

invalid_statement:
    /* 3. Ignored C++ "Water" - Explicitly listing tokens to skip */
    TOK_TEXT { $$ = nullptr; }
    | STR_LIT { $$ = nullptr; }
    | IDENT { $$ = nullptr; }
    | NUMBER { $$ = nullptr; }
    | KW_TRUE { $$ = nullptr; }
    | KW_FALSE { $$ = nullptr; }
    | SEMICOLON { $$ = nullptr; }
    | COMMA { $$ = nullptr; }
    | LPAREN { $$ = nullptr; }
    | RPAREN { $$ = nullptr; }
    | LBRACE { $$ = nullptr; }
    | RBRACE { $$ = nullptr; }
    | LBRACKET { $$ = nullptr; }
    | RBRACKET { $$ = nullptr; }
    | PLUS { $$ = nullptr; }
    | MINUS { $$ = nullptr; }
    | EQ { $$ = nullptr; }
    | NEQ { $$ = nullptr; }
    | GT { $$ = nullptr; }
    | LT { $$ = nullptr; }
    | LEQ { $$ = nullptr; }
    | GEQ { $$ = nullptr; }
    | OR { $$ = nullptr; }
    | AND { $$ = nullptr; }
    | NOT { $$ = nullptr; }
    | DCOL { $$ = nullptr; }
    | ASGN { $$ = nullptr; }
    ;

// TODO add braces to this.
scoped_block:
    TOK_BEGIN statements TOK_END
    { 
        $$ = new CompoundStmt();
        $$->_stmts.push_back("{");
        $$->_stmts.push_back($2->to_string());
        $$->_stmts.push_back("}");
        delete $2; 
    }
    ;

dsl_macro:
    TOK_DECLARE_ENUM LPAREN IDENT COMMA enum_args RPAREN
    { 
        $$ = new SingleStmt(); 
        $$->_code = std::string("enum " + *$3 + " {" + *$5 + "};\n"); 
        delete $3; 
        delete $5;
    }

    | TOK_ENUM_VAR LPAREN IDENT COMMA IDENT COMMA IDENT RPAREN
    { 
        $$ = new SingleStmt();
        $$->_code = std::string(*$3 + " " + *$5 + " = " + *$3 + "." + *$7 + ";\n");
        delete $3; delete $5; delete $7;
    }

    | TOK_SUBINT LPAREN IDENT COMMA NUMBER COMMA NUMBER RPAREN
    { $$ =  new SingleStmt(*$3, *$5, *$7); }

    | TOK_BOOL LPAREN IDENT COMMA IDENT RPAREN
    { $$ = new SingleStmt(*$3, $5); }

    // TODO ignore subint assignments
    // TODO subint is actually type definition; there should be a separate
    // int statement etc.
    // TODO variable definitions must be inside behavior
    | TOK_SET LPAREN assignment RPAREN
    {$$ = new SingleStmt(); $$->_code = std::move(*$3); }

    // FIXME Assumption: only one publisher per topic exists in each node
    | TOK_DECLARE_PUB LPAREN IDENT COMMA IDENT COMMA STR_LIT args RPAREN
    { 
        $$ = new SingleStmt();
        auto res = publishers.insert({*$5, topic_to_event(*$7)});
        if (res.second == false) std::cerr << "Duplicate publisher" << std::endl;
        delete $5; delete $7; 
    }
    // FIXME No idea how to handle put() outside behavioral blocks
    | TOK_PUT LPAREN IDENT COMMA args RPAREN
    { 
        $$ = new SingleStmt();
        $$->_code = std::string(publishers.at(*$3) + "();");
    }

    // TODO variable checking yapilmiyor.
    | TOK_GUARD LPAREN expression RPAREN valid_statement
    { 
        if ($5 != nullptr) {
            $$ = new GuardStmt(*$3, *$5);
        } 
        else $$ = new GuardStmt(*$3);
    }

    // FIXME Assumption: topic name is provided directly (not through a variable)
    // FIXME Assumption: only one subscriber is subscribed to a topic in a node
    // FIXME Assumption: only one session is active in a node
    // If multiple are active, one may attempt to create multiple models per session? 
    | TOK_DECLARE_SUB LPAREN IDENT COMMA IDENT COMMA STR_LIT COMMA args RPAREN skip_lambda scoped_block
    { 
        $$ = new SingleStmt();
        std::string *str = new std::string(); 
        *str += "on " + topic_to_event(*$7) + ": ";
        if ($12 != nullptr)
        *str += $12->to_string();
        behaviors.push_back(*str);
        subscribers.insert({*$5, topic_to_event(*$7)});
    }
    ;

enum_args:
    enum_arg
    { $$ = $1; }
    | enum_args COMMA enum_arg
    { $$ = new std::string(*$1 + "," + *$3); delete $1; delete $3; }
    ;

enum_arg:
    IDENT
    { $$ = $1; }
    | IDENT ASGN NUMBER
    { $$ = new std::string(*$1 + "=" + *$3); delete $1; delete $3; }
    ;

assignment:
    IDENT ASGN expression
    { $$ = new std::string(*$1 + " = " + *$3 + ";"); delete $1; delete $3;}
    | IDENT PLUS PLUS
    { $$ = new std::string(*$1 + " = " + *$1 + "+1;"); delete $1; }
    | IDENT MINUS MINUS
    { $$ = new std::string(*$1 + " = " + *$1 + "-1;"); delete $1; }
    ;

expression:
      expression OR expression
      { $$ = new std::string(*$1 + "||" + *$3); delete $1; delete $3; }
    | expression AND expression
    { $$ = new std::string(*$1 + "&&" + *$3); delete $1; delete $3; }
    | expression EQ expression
    { $$ = new std::string(*$1 + "==" + *$3); delete $1; delete $3; }
    | expression NEQ expression
    { $$ = new std::string(*$1 + "!=" + *$3); delete $1; delete $3; }
    | expression LT expression
    { $$ = new std::string(*$1 + "<" + *$3); delete $1; delete $3; }
    | expression GT expression
    { $$ = new std::string(*$1 + ">" + *$3); delete $1; delete $3; }
    | expression LEQ expression
    { $$ = new std::string(*$1 + "<=" + *$3); delete $1; delete $3; }
    | expression GEQ expression
    { $$ = new std::string(*$1 + ">=" + *$3); delete $1; delete $3; }
    | expression PLUS expression
    { $$ = new std::string(*$1 + "+" + *$3); delete $1; delete $3; }
    | expression MINUS expression
    { $$ = new std::string(*$1 + "-" + *$3); delete $1; delete $3; }
    /* Unary */
    | NOT expression
    { $$ = new std::string("!" + *$2); delete $2; }
    /* Grouping */
    | LPAREN expression RPAREN
    { $$ = new std::string("(" + *$2 + ")"); delete $2; }
    /* Terminals */
    | IDENT
    { $$ = $1; } /* Passed up from lexer */
    | NUMBER
    { $$ = $1; }
    | KW_TRUE
    { $$ = new std::string("true"); }
    | KW_FALSE
    { $$ = new std::string("false"); }
    /* Complex IDs */
    | IDENT DCOL IDENT
    { $$ = new std::string(*$1 + "." + *$3); delete $1; delete $3; }
    ;

/* --- SKIP LAMBDA LOGIC --- */
skip_lambda:
    LBRACKET junk_tokens RBRACKET LPAREN junk_tokens RPAREN
    ;

junk_tokens:

    | junk_tokens junk_token
    ;

junk_token:
    TOK_TEXT | STR_LIT | IDENT | NUMBER | KW_TRUE | KW_FALSE
    | SEMICOLON | COMMA
    | PLUS | MINUS | EQ | NEQ | GT | LT | LEQ | GEQ | OR | AND | NOT | DCOL | ASGN
    ;

// paren_junk_tokens:
//     | paren_junk_tokens paren_junk_token
// 
// paren_junk_token:
//     TOK_TEXT | STR_LIT | IDENT | NUMBER | KW_TRUE | KW_FALSE
//     | SEMICOLON | COMMA
//     | PLUS | MINUS | EQ | NEQ | GT | LT | LEQ | GEQ | OR | AND | NOT | DCOL | ASGN
//     ;

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
    TOK_TEXT | STR_LIT | IDENT | NUMBER | KW_TRUE | KW_FALSE
    | PLUS | MINUS | EQ | NEQ | GT | LT | LEQ | GEQ | OR | AND | NOT 
    | ASGN 
    | DCOL                 /* <--- ALLOW :: FREELY HERE */
    | COMMA
    | LPAREN args RPAREN   /* Handle nested parens */
    ;

%%

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2...]" << std::endl;
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (!file) {
            std::perror(argv[i]);
            return 1;
        }
        current_filename = argv[i];
        yyin = file;

        yyrestart(file);

        yyparse();

        fclose(file);
    }
    return 0;
}