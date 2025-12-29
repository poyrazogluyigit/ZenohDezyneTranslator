#include <catch2/catch_test_macros.hpp>
#include "../src/include/Preprocessing.h"

#define STR_HELPER(x) #x
#define STRINGIZE(x) STR_HELPER(x)

TEST_CASE("String trimming", "[trim]") {
    STATIC_REQUIRE(proc::trim("  hello  ") == "hello");
    STATIC_REQUIRE(proc::trim("abc") == "abc");
    STATIC_REQUIRE(proc::trim("\t abc \t") == "abc");
    STATIC_REQUIRE(proc::trim("   ") == "");
    STATIC_REQUIRE(proc::trim("") == "");
}

TEST_CASE("constexpr integer parsing", "[parse_int]") {
        STATIC_REQUIRE(proc::parse_int("123") == 123);
        STATIC_REQUIRE(proc::parse_int("-123") == -123);
        STATIC_REQUIRE(proc::parse_int("+123") == +123);
        STATIC_REQUIRE(proc::parse_int("0") == 0);
        STATIC_REQUIRE(proc::parse_int("2147483647") == 2147483647);
        STATIC_REQUIRE(proc::parse_int("-2147483648") == -2147483648);
        // Handling surrounding text
        STATIC_REQUIRE(proc::parse_int("31337 with words") == 31337);
        // Edge cases
        REQUIRE_THROWS_AS(proc::parse_int("foo -99 bar"), std::invalid_argument);
        REQUIRE_THROWS_AS(proc::parse_int("abc"), std::invalid_argument);
        REQUIRE_THROWS_AS(proc::parse_int("val=50"), std::invalid_argument);
        // REQUIRE_THROWS_AS(proc::parse_int("12345678901"), std::out_of_range);
}

TEST_CASE("Enum counter", "[count_enums]"){
    STATIC_REQUIRE(proc::count_enums("a, b, c") == 3);
    STATIC_REQUIRE(proc::count_enums("a=0, b, c=1, d") == 4);
    STATIC_REQUIRE(proc::count_enums("") == 0);
    // empty strings should not occur in an enum declaration anyways
    // STATIC_REQUIRE(proc::count_enums(" ") == 0);
}

TEST_CASE("Enum parser", "parse_enum_data") {
    SECTION("No custom values") {
        constexpr auto res1 = proc::parse_enum_data<4>("Alpha, Bravo, Charlie, Kirk");
        STATIC_REQUIRE(res1[0].first == "Alpha"); STATIC_REQUIRE(res1[0].second == 0);
        STATIC_REQUIRE(res1[1].first == "Bravo"); STATIC_REQUIRE(res1[1].second == 1);
        STATIC_REQUIRE(res1[2].first == "Charlie"); STATIC_REQUIRE(res1[2].second == 2);
        STATIC_REQUIRE(res1[3].first == "Kirk"); STATIC_REQUIRE(res1[3].second == 3);
    }
    SECTION("Custom values") {
        constexpr auto res1 = proc::parse_enum_data<4>("Alpha = 10, Bravo, Charlie = 15, Kirk");
        STATIC_REQUIRE(res1[0].first == "Alpha"); STATIC_REQUIRE(res1[0].second == 10);
        STATIC_REQUIRE(res1[1].first == "Bravo"); STATIC_REQUIRE(res1[1].second == 11);
        STATIC_REQUIRE(res1[2].first == "Charlie"); STATIC_REQUIRE(res1[2].second == 15);
        STATIC_REQUIRE(res1[3].first == "Kirk"); STATIC_REQUIRE(res1[3].second == 16);
    }
        SECTION("Whitespace") {
        constexpr auto res1 = proc::parse_enum_data<4>("  Alpha , Bravo    ,    Charlie   , Kirk    ");
        STATIC_REQUIRE(res1[0].first == "Alpha"); STATIC_REQUIRE(res1[0].second == 0);
        STATIC_REQUIRE(res1[1].first == "Bravo"); STATIC_REQUIRE(res1[1].second == 1);
        STATIC_REQUIRE(res1[2].first == "Charlie"); STATIC_REQUIRE(res1[2].second == 2);
        STATIC_REQUIRE(res1[3].first == "Kirk"); STATIC_REQUIRE(res1[3].second == 3);
    }
}

TEST_CASE("Expression parsing", "guard_expr_grammar"){
    SECTION("Atomics") {
        STATIC_REQUIRE(validate("true")); STATIC_REQUIRE(validate("false")); 
        STATIC_REQUIRE(validate("abc")); STATIC_REQUIRE(validate("sdfsdf")); 
        STATIC_REQUIRE(validate("(true)"));
    }
    SECTION("Operations") {
        STATIC_REQUIRE(validate("true || false")); STATIC_REQUIRE(validate("false && !false")); 
        STATIC_REQUIRE(validate("a || b && (c || d == (e && f))"));
    }
}