#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

enum class VarType {
    BOOL, 
    ENUM, 
    SUBINT
};

struct SubintData {
    std::string_view name;
    int* data;
    int min;
    int max;
};

struct BoolData {
    std::string_view name;
    bool initialValue;
};

struct EnumData {
    std::string_view name;
    size_t size;
    const std::pair<std::string_view, int> * data;
};

struct Entry {
    VarType type;
    union {
        SubintData subintData;
        EnumData enumData;
        BoolData boolData;
    };

    Entry(SubintData&& data) : type(VarType::SUBINT), subintData(data) {}
    Entry(EnumData&& data) : type(VarType::ENUM), enumData(data) {}
    Entry(BoolData && data) : type(VarType::BOOL), boolData(data) {}

};

enum class ScopeTokenType {
    BEGIN_SCOPE,
    END_SCOPE,
    VAR_DECL,
    IF_STMT,
    FUNC_DECL
};

struct ScopeToken {
    ScopeTokenType type;
    std::string_view stmt;
};


struct ParseContext {
    std::vector<Entry> variables;
    std::vector<ScopeToken> scopes;
    void registerSubint(std::string_view name, int* ptr, int min, int max){
        variables.push_back(SubintData{name, ptr, min, max});
    }
    void registerEnum(std::string_view name, size_t size, const std::pair<std::string_view, int> * table) {
        variables.push_back(EnumData{name, size, table});
    }
    void registerBool(std::string_view name, bool initialValue) {
        variables.push_back(BoolData{name, initialValue});
    }
    void beginScope() {
        this->scopes.push_back(ScopeToken{ScopeTokenType::BEGIN_SCOPE, ""});
    }
    void endScope() {
        this->scopes.push_back(ScopeToken{ScopeTokenType::END_SCOPE, ""});
    }
    void declareFunction(const char* name) {
        this->scopes.push_back(ScopeToken{ScopeTokenType::FUNC_DECL, std::string_view(name)});
    }
    void push_stmt(ScopeTokenType type, const char* stmt) {
        this->scopes.push_back(ScopeToken{type, std::string_view(stmt)});
    }
    void clearScope(){scopes.clear();}
    void clearVariables(){variables.clear();}
};