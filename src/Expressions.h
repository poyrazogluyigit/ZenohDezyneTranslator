#pragma once
#include <array>
#include <string>
#include <string_view>

#include "zenoh.h"

template<size_t N>
struct const_string : std::array<char, N> {
    constexpr auto str() const -> std::string_view {
        return std::string_view(std::array<char, N>::data(), N);
    }
};
template<size_t N>
const_string(const char(&chars)[N]) -> const_string<N>;
const_string(auto... chars) -> const_string<sizeof...(chars)>;

enum class TokenType {
    End,
    Unknown, 
    Identifier,
    Literal,
    LeftParen,
    RightParen,
    LeftBrac,
    RightBrac,

    NotOp,
    EqOp,
    NeqOp,
    AndOp,
    OrOp
};

struct Token {
    TokenType type;
    size_t from = 0;
    size_t to = 0;
    std::string_view id;

    constexpr auto is_end() const {
        return type == TokenType::End;
    }
};

struct SourceStream {
    std::string_view s;
    size_t i;

    constexpr auto token() -> Token {
        // eliminate white space
        while (i < s.size()) {
            if (s[i] != ' ') break;
            i++;
        }

        auto from = i;
        if (i > s.size())
            return Token{TokenType::End, i, i};
        else if (is_identifier_char(s[i])) {
            while(i < s.size() && is_identifier_char(s[i])) i++;
            return Token{TokenType::Identifier, from, i, s.substr(from, i)};
        }
        else if (s[i] == '[') {
            i++;
            return Token{TokenType::LeftBrac, from, i};
        }
        else if (s[i] == ']') {
            i++;
            return Token{TokenType::RightBrac, from, i};
        }
        else if (s[i] == '(') {
            i++;
            return Token{TokenType::LeftParen, from, i};
        }
        else if (s[i] == ')') {
            i++;
            return Token{TokenType::RightParen, from, i};
        }
        else if (s[i] == '!') {
            i++;
            if (i < s.size() && s[i] == '=') {
                i++;
                return Token{TokenType::NeqOp, from, i};
            }
            else return Token{TokenType::NotOp, from, i};
        }
        else if (s[i] == '='){
            i++;
            if (i< s.size() && s[i] == '=') {
                i++;
                return Token{TokenType::EqOp, from, i};
            }
            else return Token{TokenType::Unknown, from, i};
        }
        else if (s[i] == '&'){
            i++;
            if (i< s.size() && s[i] == '&') {
                i++;
                return Token{TokenType::AndOp, from, i};
            }
            else return Token{TokenType::Unknown, from, i};
        }
        else if (s[i] == '|'){
            i++;
            if (i< s.size() && s[i] == '|') {
                i++;
                return Token{TokenType::OrOp, from, i};
            }
            else return Token{TokenType::Unknown, from, i};
        }
        else return Token{TokenType::Unknown, from, i};
    }

    static constexpr auto is_identifier_char(char c) -> bool {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    }
};

#define MAX_TOKENS 1024
// TODO ornek move semantics kullaniyor, ben su anda
// sadece copy yapiyorum. improve 
// genel value semantics lazim gibi
struct TokenStream {
    std::array<Token, MAX_TOKENS> tokens;
    size_t position;
    bool capacity_exceeded = false;

    constexpr auto next() const {
        return TokenStream{tokens, position+1};
    }

    constexpr auto operator++() -> TokenStream& {
        position += 1;
        return *this;
    }

    constexpr auto operator*() const {
        return tokens[position];
    }

    static constexpr auto of(std::string_view s) -> TokenStream {
        SourceStream source_stream{s, 0};
        std::array<Token, MAX_TOKENS> tokens{};
        size_t i = 0;
        auto tk = source_stream.token();
        bool capacity_exceeded = false;
        while (!tk.is_end()){
            tokens[i] = tk;
            capacity_exceeded = ++i == MAX_TOKENS;
            if (capacity_exceeded) break;
            tk = source_stream.token();
        }
        return TokenStream{tokens, 0, capacity_exceeded};
    }
};

namespace parse {

    struct Void {};

    template<typename T, typename It>
    struct Result{
        T expr;
        It it;

        constexpr auto has_value() const {
            return !std::is_same_v<T, Void>;
        }
    };

    template<typename T, typename It>
    Result(T, It) -> Result<T, It>;

    struct Equal {};
    struct NotEqual {};
    struct LogicalAnd {};
    struct LogicalOr {};
    struct LogicalNot {};

    template<auto lhs, auto next>
    constexpr auto chain(){
        constexpr auto e = next.template operator()<lhs.it>(lhs.expr);
        if constexpr (e.has_value()){
            return chain<e, next>();
        }
        else {
            return lhs;
        }
    }

    template<auto it>
    constexpr auto bool_expression(){
        return logical_or<it>();
    }

    template<auto it>
    constexpr auto logical_or(){
        constexpr auto lhs = logical_and<it>();
        return parse_chain<lhs, []<auto it2>(auto lhs) {
            if constexpr ((*it2).type == TokenType::OrOp) {
                auto rhs = logical_or<it2.next()>();
                return Result{LogicalAnd{}, rhs.it};
            }
        }
    }

    template<auto it>
    constexpr auto logical_and(){

    }
    
    template<auto it>
    constexpr auto equality(){

    }

    template<auto it>
    constexpr auto unary(){

    }

    template<auto it>
    constexpr auto primary(){

    }


}



// #define GUARD(expr) parse<#expr>();