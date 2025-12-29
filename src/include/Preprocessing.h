#pragma once

#include <lexy/dsl/brackets.hpp>
#include <lexy/dsl/digit.hpp>
#include <stdexcept>
#include <string_view>
#include <array>

#include "lexy/dsl/expression.hpp"
#include <lexy/action/parse.hpp>
#include <lexy/action/trace.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

#include "Registries.h"

namespace grammar {
    namespace dsl = lexy::dsl;
    
    // TERMINALS
    static constexpr auto ws = dsl::whitespace(dsl::ascii::space);
    static constexpr auto identifier = dsl::identifier(
    dsl::ascii::alpha_underscore,
    dsl::ascii::alpha_digit_underscore
    );
    
    static constexpr auto kw_true  = LEXY_KEYWORD("true",  dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_false = LEXY_KEYWORD("false", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_subint = LEXY_KEYWORD("DSL_SUBINT", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_bool   = LEXY_KEYWORD("DSL_BOOL", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_guard = LEXY_KEYWORD("DSL_GUARD", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_begin  = LEXY_KEYWORD("BEGIN", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_end    = LEXY_KEYWORD("END", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_beh    = LEXY_KEYWORD("BEHAVIOR", dsl::identifier(dsl::ascii::alpha));
    static constexpr auto kw_endbeh = LEXY_KEYWORD("END_BEHAVIOR", dsl::identifier(dsl::ascii::alpha));


    static constexpr auto digits = dsl::digits<>.sep(dsl::digit_sep_tick).no_leading_zero();
    static constexpr auto integer_literal = dsl::integer<int>(digits);
    
    static constexpr auto comma = dsl::lit_c<','>;
    
    
    // operators
    static constexpr auto op_not  = dsl::op(dsl::lit_c<'!'>);
    static constexpr auto op_eq   = dsl::op(dsl::lit_c<'='> >> dsl::lit_c<'='>);
    static constexpr auto op_neq  = dsl::op(dsl::lit_c<'!'> >> dsl::lit_c<'='>);
    static constexpr auto op_and  = dsl::op(dsl::lit_c<'&'> >> dsl::lit_c<'&'>);
    static constexpr auto op_or   = dsl::op(dsl::lit_c<'|'> >> dsl::lit_c<'|'>);
    static constexpr auto op_impl = dsl::op(dsl::lit_c<'-'> >> dsl::lit_c<'>'>);

    // 1. The Main Production
    struct bool_expr : lexy::expression_production {
        

        // 3. Define Atom (Nested so it can recurse)
        static constexpr auto atom = kw_true | kw_false | identifier |
                dsl::parenthesized(dsl::recurse<bool_expr>);
                
        // Define operators wrt precedences
        struct unary : dsl::prefix_op {
            static constexpr auto op    = op_not;
            using operand               = dsl::atom;
        };

        struct and_expr : dsl::infix_op_left {
            static constexpr auto op    = op_and;
            using operand               = unary;
        };

        struct or_expr : dsl::infix_op_left {
            static constexpr auto op    = op_or;
            using operand               = and_expr;
        };

        // TODO implication/esitlik precedence check
        struct impl_expr : dsl::infix_op_right {
            static constexpr auto op    = op_impl;
            using operand               = or_expr;
        };

        struct eq_expr : dsl::infix_op_left {
            static constexpr auto op    = op_eq / op_neq;
            using operand               = impl_expr;
        };

        using operation                 = eq_expr;
        
    };

    struct subint_stmt {
        static constexpr auto rule = 
            kw_subint >> dsl::parenthesized(
                identifier +
                (comma + integer_literal) +
                (comma + integer_literal)
            );
    };

    struct bool_stmt {
        static constexpr auto rule = 
        kw_bool >> dsl::parenthesized(
            identifier + comma + integer_literal
        );
    };

    struct enum_item {
        static constexpr auto rule =
            (identifier + dsl::if_(dsl::op(dsl::lit_c<'='>) >> integer_literal));

    };

    struct enum_stmt {
        static constexpr auto rule = 
        kw_bool >> dsl::parenthesized(
            identifier + comma + dsl::list(dsl::p<enum_item>, dsl::sep(comma))
        );
    };

    // TODO burasi tam identifier degil
    struct bhv_begin {
        static constexpr auto rule = 
        kw_beh >> dsl::parenthesized(
            identifier
        );
    };

    struct bhv_end {
        static constexpr auto rule = kw_endbeh;
    };

    struct guard_stmt {
        static constexpr auto rule = 
        kw_guard >> dsl::parenthesized(dsl::p<bool_expr>);
    };

    struct begin_stmt {
        static constexpr auto rule = kw_begin;
    };

    struct end_stmt {
        static constexpr auto rule = kw_end;
    };

    struct cpp_noise {
        static constexpr auto rule = 
        (LEXY_LIT("//") >> dsl::until(dsl::newline)) 
        | (LEXY_LIT("/*") >> dsl::until(LEXY_LIT("*/")))
        | dsl::quoted(dsl::ascii::character)
        | dsl::code_point
        ;
    };
    

    struct block_parse_entry {
        static constexpr auto rule = dsl::list(
            dsl::p<subint_stmt>
            | dsl::p<bool_stmt>
            | dsl::p<enum_stmt>
            | dsl::p<bhv_begin>
            | dsl::p<bhv_end>
            | dsl::p<guard_stmt>
            | dsl::p<begin_stmt>
            | dsl::p<end_stmt>
            | dsl::p<cpp_noise>
        );
    };


}

namespace proc {

    constexpr std::string_view trim(std::string_view sv){
        using namespace std::literals;
        auto start = sv.find_first_not_of(" \t"sv);
        if (start == std::string_view::npos) return {};
        sv.remove_prefix(start);

        auto end = sv.find_last_not_of(" \t"sv);
        if (end != std::string_view::npos)
            sv.remove_suffix(sv.size() - (end + 1));
        return sv;
    }

    // TODO should return invalid argument on 'foo 99 bar'
    constexpr int parse_int(std::string_view str){
        str = trim(str);
        using namespace std::literals;
        const auto numbers = "0123456789"sv;
        const auto start = str.find_first_of(numbers);
        
        if (start > 1U)
            throw std::invalid_argument{"parse_int"};

        const auto sign = start != 0U && str[start - 1U] == '-' ? -1 : 1;
        str.remove_prefix(start);

        const auto end = str.find_first_not_of(numbers);
        if (end != std::string_view::npos)
            str.remove_suffix(std::size(str) - end);

        long long result = 0;
        auto multiplier = sign;
        auto len = str.size();
        for (std::size_t idx = 0; idx < len; ++idx){
            auto i = len - 1 - idx;
            auto number = str[i] - '0';
            result += number * multiplier;
            multiplier *= 10U;
        }
        return result;
    }

    constexpr std::size_t count_enums(const char* str){
        std::size_t count = (*str == '\0') ? 0 : 1;
        for (const char* p = str; *p; ++p){
            if (*p == ',') count++;
        }
        return count;
    }

    template<std::size_t N>
    constexpr std::array<std::pair<std::string_view, int>, N> parse_enum_data(std::string_view str){
        
        using namespace std::literals;
        std::array<std::pair<std::string_view, int>, N> result{};
        
        str = trim(str);

        size_t index = 0;
        int next_enum_value = 0;

        while (index < N){
            // extract token
            auto comma_pos = str.find(',');
            auto token = (comma_pos == std::string_view::npos) 
                ? str
                : str.substr(0, comma_pos);
            // check for assignment
            auto eq_pos = token.find('=');
            std::string_view name;
            if (eq_pos != std::string_view::npos) {
                name = trim(token.substr(0,  eq_pos));
                auto value = trim(token.substr(eq_pos + 1));
                next_enum_value = parse_int(value);
            }
            else {
                name = trim(token);
            }  
            result[index++] = std::pair(name, next_enum_value++);

            if (comma_pos == std::string_view::npos) break;
            str = str.substr(comma_pos + 1);
        }

        return result;
    }
}

consteval bool validate(const char* str){
    return lexy::match<grammar::bool_expr>(lexy::zstring_input(str));
}
