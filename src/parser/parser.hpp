/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 46 "parser.y"

    #include "Statement.h"

#line 53 "parser.hpp"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOK_DECLARE_ENUM = 258,        /* TOK_DECLARE_ENUM  */
    TOK_DECLARE_SUB = 259,         /* TOK_DECLARE_SUB  */
    TOK_DECLARE_PUB = 260,         /* TOK_DECLARE_PUB  */
    TOK_ENUM_VAR = 261,            /* TOK_ENUM_VAR  */
    TOK_SUBINT = 262,              /* TOK_SUBINT  */
    TOK_BOOL = 263,                /* TOK_BOOL  */
    TOK_GUARD = 264,               /* TOK_GUARD  */
    TOK_SET = 265,                 /* TOK_SET  */
    TOK_PUT = 266,                 /* TOK_PUT  */
    KW_TRUE = 267,                 /* KW_TRUE  */
    KW_FALSE = 268,                /* KW_FALSE  */
    PLUS = 269,                    /* PLUS  */
    MINUS = 270,                   /* MINUS  */
    EQ = 271,                      /* EQ  */
    NEQ = 272,                     /* NEQ  */
    GT = 273,                      /* GT  */
    LT = 274,                      /* LT  */
    LEQ = 275,                     /* LEQ  */
    GEQ = 276,                     /* GEQ  */
    OR = 277,                      /* OR  */
    AND = 278,                     /* AND  */
    NOT = 279,                     /* NOT  */
    ASGN = 280,                    /* ASGN  */
    DCOL = 281,                    /* DCOL  */
    LPAREN = 282,                  /* LPAREN  */
    RPAREN = 283,                  /* RPAREN  */
    LBRACE = 284,                  /* LBRACE  */
    RBRACE = 285,                  /* RBRACE  */
    LBRACKET = 286,                /* LBRACKET  */
    RBRACKET = 287,                /* RBRACKET  */
    SEMICOLON = 288,               /* SEMICOLON  */
    COMMA = 289,                   /* COMMA  */
    TOK_BEGIN = 290,               /* TOK_BEGIN  */
    TOK_END = 291,                 /* TOK_END  */
    IDENT = 292,                   /* IDENT  */
    TOK_TEXT = 293,                /* TOK_TEXT  */
    STR_LIT = 294,                 /* STR_LIT  */
    NUMBER = 295                   /* NUMBER  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 50 "parser.y"

    std::string* sval;
    SingleStmt* sstmt;
    CompoundStmt* cstmt;
    GuardStmt* gstmt;

#line 117 "parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
