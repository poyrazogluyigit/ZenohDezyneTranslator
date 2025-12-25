#include <tuple>
#include <stdexcept>
#include <string_view>
#include <stdint.h>
#include <array>

template<size_t N>
struct const_string : std::array<char, N> {
    constexpr auto str() const -> std::string_view {
        return std::string_view(std::array<char, N>::data(), N);
    }
};
template<size_t N>
const_string(const char(&chars)[N]) -> const_string<N>;
const_string(auto... chars) -> const_string<sizeof...(chars)>;

template<typename T, size_t Capacity>
struct static_vector {
    constexpr auto operator[](size_t i) -> T& {
        return raw[i];
    }

    constexpr auto operator[](size_t i) const -> T const& {
        return raw[i];
    }

    constexpr void emplace_back(auto&& expr) {
        raw[count] = static_cast<decltype(expr)>(expr);
        count += 1;
    }

    constexpr auto size() const -> size_t {
        return count;
    }

    constexpr auto data() const -> const T* {
        return raw.data();
    }

    std::array<T, Capacity> raw{};
    size_t count = 0;
};

enum class TokenType {
    End,
    Unknown,
    Identifier,
    Argument,
    Operator,
    LeftParen,
    RightParen,
    Comma,
    Arrow,

    Plus,
    Minus,
    Asterisk,
    Slash,
    Tilde,
    Not
};

struct Token {
    TokenType type;
    size_t from = 0;
    size_t to = 0;
    size_t id = 0;

    constexpr auto is_end() const {
        return type == TokenType::End;
    }
};

struct SourceStream {
    std::string_view s;
    size_t i;

    constexpr auto token() -> Token {
        while (i < s.size()) {
            if (s[i] != ' ') {
                break;
            }
            i++;
        }

        auto from = i;
        if (i >= s.size()) {
            return Token{TokenType::End, i, i};
        } else if (is_identifier_char(s[i])) {
            while (i < s.size() && is_identifier_char(s[i])) {
                ++i;
            }
            return Token{TokenType::Identifier, from, i, fnv1a(s.substr(from, i - from))};
        } else if (s[i] == '+') {
            i++;
            return Token{TokenType::Plus, from , i};
        } else if (s[i] == '-') {
            i++;
            if (s[i] == '>') {
                i++;
                return Token{TokenType::Arrow, from, i};
            }
            return Token{TokenType::Minus, from , i};
        } else if (s[i] == '*') {
            i++;
            return Token{TokenType::Asterisk, from, i};
        } else if (s[i] == '/') {
            i++;
            return Token{TokenType::Slash, from, i};
        } else if (s[i] == '!') {
            i++;
            return Token{TokenType::Not, from, i};
        } else if (s[i] == '~') {
            i++;
            return Token{TokenType::Tilde, from, i};
        } else if (s[i] == '(') {
            i++;
            return Token{TokenType::LeftParen, from, i};
        } else if (s[i] == ')') {
            i++;
            return Token{TokenType::RightParen, from, i};
        } else if (s[i] == ',') {
            i++;
            return Token{TokenType::Comma, from, i};
        } else {
            i++;
            return Token{TokenType::Unknown, from, i};
        }
    }

    static constexpr auto fnv1a(std::string_view s) -> uint32_t {
        uint32_t hash = 0;
        for (char i : s) {
            hash *= 0x811C9DC5;
            hash ^= static_cast<uint32_t>(i);
        }
        return hash;
    }

    static constexpr auto is_identifier_char(char c) -> bool {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    }
};

struct TokenStream {
    static_vector<Token, 1024> tokens;
    size_t position;

    constexpr auto next() const {
        return TokenStream{tokens, position + 1};
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
        static_vector<Token, 1024> tokens{};
        auto tk = source_stream.token();
        while (!tk.is_end()) {
            tokens.emplace_back(tk);
            tk = source_stream.token();
        }
        return TokenStream{tokens, 0};
    }
};

namespace meta {
    struct Void {};

    template<typename T, typename It>
    struct Result {
        T expr;
        It it;

        constexpr auto has_value() const {
            return !std::is_same_v<T, Void>;
        }
    };

    template<typename T, typename It>
    Result(T, It) -> Result<T, It>;

    template<typename O, typename Arg0, typename Arg1>
    struct Binary {
        O op;
        Arg0 arg0;
        Arg1 arg1;
    };
    Binary(auto op, auto arg0, auto arg1) -> Binary<decltype(op), decltype(arg0), decltype(arg1)>;

    struct Plus {};
    struct Minus {};
    struct Multiply {};
    struct Divide {};
    struct Equal {};
    struct NotEqual {};
    struct Less {};
    struct LessEqual {};
    struct Greater {};
    struct GreaterEqual {};
    struct LogicalAnd {};
    struct LogicalOr {};
    struct LogicalNot {};
    struct BitwiseAnd {};
    struct BitwiseOr {};
    struct BitwiseXor {};
    struct BitwiseNot {};
    struct BitwiseLeftShift {};
    struct BitwiseRightShift {};

    template<typename O, typename Arg> struct Unary {
        O op;
        Arg arg;
    };
    Unary(auto op, auto arg) -> Unary<decltype(op), decltype(arg)>;

    template<size_t> struct Argument {};

    struct Identifier {
        size_t id;
    };

    struct Parameter {
        Identifier name;
    };

    struct Signature {
        static_vector<Parameter, 10> params;
    };

    constexpr auto get_argument_index(const Signature& fn, const Identifier& id) -> size_t {
        for (size_t i = 0; i < fn.params.size(); ++i) {
            if (fn.params[i].name.id == id.id) {
                return i;
            }
        }
        throw std::runtime_error("Unknown parameter");
    }

    constexpr void expect(const auto& it, TokenType type, const char* msg) {
        if ((*it).type != type) {
            throw std::runtime_error(msg);
        }
    }

    constexpr void consume(auto& it, TokenType type, const char* msg) {
        expect(it, type, msg);
        ++it;
    }

    template<auto lhs, auto next>
    constexpr auto parse_chain() {
        constexpr auto e = next.template operator()<lhs.it>(lhs.expr);
        if constexpr (e.has_value()) {
            return parse_chain<e, next>();
        } else {
            return lhs;
        }
    }

    constexpr auto parse_identifier(auto& it) {
        auto id = Identifier{(*it).id};
        consume(it, TokenType::Identifier, "Expected identifier");
        return id;
    }

    template<auto it, auto fn>
    constexpr auto parse_expression();

    template<auto it, auto fn>
    constexpr auto parse_primitive() {
        if constexpr ((*it).type == TokenType::LeftParen) {
            auto expr = parse_expression<it.next(), fn>();
            consume(expr.it, TokenType::RightParen, "Expected )");
            return expr;
        } else {
            constexpr auto id = Identifier{(*it).id};
            expect(it, TokenType::Identifier, "Expected identifier");
            constexpr auto arg = get_argument_index(fn, id);
            return Result{Argument<arg>{}, it.next()};
        }
    }

    template<auto it, auto fn>
    constexpr auto parse_unary() {
        if constexpr ((*it).type == TokenType::Plus) {
            auto expr = parse_unary<it.next(), fn>();
            return Result{Unary{Plus{}, expr.expr}, expr.it};
        } else if constexpr ((*it).type == TokenType::Minus) {
            auto expr = parse_unary<it.next(), fn>();
            return Result{Unary{Minus{}, expr.expr}, expr.it};
        } else {
            return parse_primitive<it, fn>();
        }
    }

    template<auto it, auto fn>
    constexpr auto parse_not() {
        if constexpr((*it).type == TokenType::Not) {
            auto expr = parse_not<it.next(), fn>();
            return Result{Unary{LogicalNot{}, expr.expr}, expr.it};
        } else if constexpr ((*it).type == TokenType::Tilde) {
            auto expr = parse_not<it.next(), fn>();
            return Result{Unary{BitwiseNot{}, expr.expr}, expr.it};
        } else {
            return parse_unary<it, fn>();
        }
    }

    template<auto it, auto fn>
    constexpr auto parse_multiplication() {
        constexpr auto lhs = parse_not<it, fn>();
        return parse_chain<lhs, []<auto it2>(auto lhs) {
            if constexpr((*it2).type == TokenType::Asterisk) {
                auto rhs = parse_not<it2.next(), fn>();
                return Result{Binary{Multiply{}, lhs, rhs.expr}, rhs.it};
            } else if constexpr((*it2).type == TokenType::Slash) {
                auto rhs = parse_not<it2.next(), fn>();
                return Result{Binary{Divide{}, lhs, rhs.expr}, rhs.it};
            } else {
                return Result{Void{}, it2.next()};
            }
        }>();
    }

    template<auto it, auto fn>
    constexpr auto parse_addition() {
        constexpr auto lhs = parse_multiplication<it, fn>();
        return parse_chain<lhs, []<auto it2>(auto lhs) {
            if constexpr((*it2).type == TokenType::Plus) {
                auto rhs = parse_multiplication<it2.next(), fn>();
                return Result{Binary{Plus{}, lhs, rhs.expr}, rhs.it};
            } else if constexpr((*it2).type == TokenType::Minus) {
                auto rhs = parse_multiplication<it2.next(), fn>();
                return Result{Binary{Minus{}, lhs, rhs.expr}, rhs.it};
            } else {
                return Result{Void{}, it2.next()};
            }
        }>();
    }

    template<auto it, auto fn>
    constexpr auto parse_expression() {
        return parse_addition<it, fn>();
    }

    constexpr auto parse_param(auto& it) {
        return Parameter{parse_identifier(it)};
    }

    constexpr auto parse_params(auto& it) {
        consume(it, TokenType::LeftParen, "Expected (");
        static_vector<Parameter, 10> params{};
        if ((*it).type != TokenType::RightParen) {
            while (true) {
                params.emplace_back(parse_param(it));
                if ((*it).type == TokenType::RightParen) {
                    break;
                }
                consume(it, TokenType::Comma, "Expected ,");
            }
        }
        consume(it, TokenType::RightParen, "Expected )");
        return params;
    }

    constexpr auto parse_signature(auto it) {
        auto params = parse_params(it);
        consume(it, TokenType::Arrow, "Expected ->");
        return Result{Signature{params}, it};
    }

    template<size_t I, typename... Args>
    inline constexpr auto apply(const Argument<I>& argument, std::tuple<Args...> args) {
        return std::get<I>(args);
    }

    template<typename O, typename Arg0, typename... Args>
    inline constexpr auto apply(const Unary<O, Arg0>& exp, std::tuple<Args...> args) {
        if constexpr (std::is_same_v<O, Plus>) {
            return +apply(exp.arg, args);
        } else if constexpr (std::is_same_v<O, Minus>) {
            return -apply(exp.arg, args);
        } else if constexpr (std::is_same_v<O, LogicalNot>) {
            return !apply(exp.arg, args);
        } else if constexpr (std::is_same_v<O, BitwiseNot>) {
            return ~apply(exp.arg, args);
        } else {
            static_assert(!std::is_same_v<O, O>);
        }
    }

    template<typename O, typename Arg0, typename Arg1, typename... Args>
    inline constexpr auto apply(const Binary<O, Arg0, Arg1>& exp, std::tuple<Args...> args) {
        if constexpr (std::is_same_v<O, Plus>) {
            return apply(exp.arg0, args) + apply(exp.arg1, args);
        } else if constexpr (std::is_same_v<O, Minus>) {
            return apply(exp.arg0, args) - apply(exp.arg1, args);
        } else if constexpr (std::is_same_v<O, Multiply>) {
            return apply(exp.arg0, args) * apply(exp.arg1, args);
        } else if constexpr (std::is_same_v<O, Divide>) {
            return apply(exp.arg0, args) / apply(exp.arg1, args);
        } else {
            static_assert(!std::is_same_v<O, O>);
        }
    }

    template<
        const_string chars,
        auto signature = parse_signature(TokenStream::of(chars.str())),
        auto expression = parse_expression<signature.it, signature.expr>().expr
    >
    inline constexpr auto function = [](auto... args) constexpr {
        return meta::apply(expression, std::forward_as_tuple(args...));
    };
}

#define $fn(fn) meta::function<const_string{#fn}>

int main() {
    constexpr auto fn1 = $fn((a, b, c, d) -> d - ((a - b) + c));
    constexpr auto fn2 = [](int a, int b, int c, int d) {return d - ((a - b) + c); };

    static_assert(fn1(12, 97, 11, 121) == fn2(12, 97, 11, 121));

    constexpr auto p_fn = static_cast<int(*)(int, int, int, int)>(fn1);

    static_assert(195 == p_fn(12, 97, 11, 121));
    return 0;
}
