// example function
#include "Registries.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Variable Registry: Registration", "[registry]") {
    SECTION("Subint registration") {
        Registry::get().clear();
        int my_int = 10;
        static AutoRegister reg("my_int", &my_int, 0, 100);
        
        auto &entries = Registry::get().getAll();
        REQUIRE(entries.size() == 1);

        const auto &entry = entries[0];
        REQUIRE(entry.type == VarType::SUBINT);
        CHECK(entry.subintData.min == 0);
        CHECK(entry.subintData.max == 100);
        CHECK(entry.subintData.data == &my_int);
    }
    SECTION("Enum registration") {
        Registry::get().clear();
        enum class my_enum {A, B, C, D};

        static constexpr std::array<std::pair<std::string_view, int>, 4> enumTable{{
            {"A", 0},
            {"B", 1},
            {"C", 2},
            {"D", 3}
        }};

        AutoRegister reg("my_enum", &enumTable);

        auto& entries = Registry::get().getAll();
        REQUIRE(entries.size() == 1);

        const auto& entry = entries[0];
        REQUIRE(entry.type == VarType::ENUM);
        
        CHECK(std::string(entry.enumData.name) == "my_enum");
        CHECK(entry.enumData.size == 4);

        REQUIRE(entry.enumData.data != nullptr);
        
        CHECK(entry.enumData.data[0].first == "A");
        CHECK(entry.enumData.data[0].second == 0);
        CHECK(entry.enumData.data[3].first == "D");
        CHECK(entry.enumData.data[3].second == 3);
    }
    SECTION("Bool registration"){
        Registry::get().clear();
        static bool my_bool = false;

        AutoRegister reg("my_bool", false);
        
        auto& entries = Registry::get().getAll();
        REQUIRE(entries.size() == 1);

        const auto& entry = entries[0];
        REQUIRE(entry.type == VarType::BOOL);

        CHECK(entry.boolData.initialValue == false);
        CHECK(std::string(entry.boolData.name) == "my_bool");

    }
}


TEST_CASE("Statement Registry Tracking", "[StmtRegistry]") {
    SECTION("Clearing Registry") {
        ScopeAutoReg{"func"};
        REQUIRE(StmtRegistry::get().getAll().size() == 1);
        
        StmtRegistry::get().clear();
        REQUIRE(StmtRegistry::get().getAll().empty());
    }
    SECTION("Function Declaration Registration") {
        StmtRegistry::get().clear();
        ScopeAutoReg{"void myFunc()"};

        const auto& stream = StmtRegistry::get().getAll();
        
        REQUIRE(stream.size() == 1);
        CHECK(stream[0].type == ScopeTokenType::FUNC_DECL);
        CHECK(stream[0].stmt == "void myFunc()");
    }

    SECTION("Manual Scope Control") {
        StmtRegistry::get().clear();
        // Simulate: { int x = 10; }
        ScopeAutoReg{ScopeTokenType::BEGIN_SCOPE};
        ScopeAutoReg{ScopeTokenType::VAR_DECL, "int x = 10"};
        ScopeAutoReg{ScopeTokenType::END_SCOPE};

        const auto& stream = StmtRegistry::get().getAll();

        REQUIRE(stream.size() == 3);
        
        CHECK(stream[0].type == ScopeTokenType::BEGIN_SCOPE);
        
        CHECK(stream[1].type == ScopeTokenType::VAR_DECL);
        CHECK(stream[1].stmt == "int x = 10");
        
        CHECK(stream[2].type == ScopeTokenType::END_SCOPE);
    }

    SECTION("Control Flow Tokens") {
        StmtRegistry::get().clear();
        // Simulate: if (cond) return;
        ScopeAutoReg{ScopeTokenType::IF_STMT, "cond"};
        ScopeAutoReg{ScopeTokenType::BEGIN_SCOPE};
        ScopeAutoReg{ScopeTokenType::VAR_DECL, "bool a = false;"}; // simulating return as stmt
        ScopeAutoReg{ScopeTokenType::END_SCOPE};

        const auto& stream = StmtRegistry::get().getAll();

        REQUIRE(stream.size() == 4);
        CHECK(stream[0].type == ScopeTokenType::IF_STMT);
        CHECK(stream[0].stmt == "cond");
        CHECK(stream[1].type == ScopeTokenType::BEGIN_SCOPE);
        CHECK(stream[3].type == ScopeTokenType::END_SCOPE);
    }
}