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
    const char* name;
    int* data;
    int min;
    int max;
};

struct BoolData {
    const char * name;
    bool initialValue;
};

struct EnumData {
    const char * name;
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


class Registry {
public:
    static Registry& get(){
        static Registry instance;
        return instance;
    }
    void registerSubint(const char* name, int* ptr, int min, int max){
        entries.push_back(SubintData{name, ptr, min, max});
    }
    void registerEnum(const char * name, size_t size, const std::pair<std::string_view, int> * table) {
        entries.push_back(EnumData{name, size, table});
    }
    void registerBool(const char * name, bool initialValue) {
        entries.push_back(BoolData{name, initialValue});
    }

    void clear(){entries.clear();}

    std::vector<Entry>& getAll() {return entries; }

    Registry(Registry&) = delete;
    Registry(Registry&&) = delete;
    void operator=(const Registry&) = delete;
protected:
    Registry() = default;
private:
    std::vector<Entry> entries;
};

struct AutoRegister {
    AutoRegister(const char * name, int * ptr, int min, int max){
        Registry::get().registerSubint(name, ptr, min, max);
    }

    template <std::size_t N>
    AutoRegister(const char * name, const std::array<std::pair<std::string_view, int>, N>* arrayPtr) {
        Registry::get().registerEnum(name, N, arrayPtr->data());
    }

    AutoRegister(const char * name, bool initialValue) {
        Registry::get().registerBool(name, initialValue);
    }
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

struct StmtRegistry {
    static StmtRegistry& get() {
        static StmtRegistry instance;
        return instance;
    }
    void beginScope() {
        this->stream.push_back(ScopeToken{ScopeTokenType::BEGIN_SCOPE, ""});
    }
    void endScope() {
        this->stream.push_back(ScopeToken{ScopeTokenType::END_SCOPE, ""});
    }
    void declareFunction(const char* name) {
        this->stream.push_back(ScopeToken{ScopeTokenType::FUNC_DECL, std::string_view(name)});
    }
    void push_stmt(ScopeTokenType type, const char* stmt) {
        this->stream.push_back(ScopeToken{type, std::string_view(stmt)});
    }
    void clear(){stream.clear();}
    std::vector<ScopeToken>& getAll() {return stream;}

    std::vector<ScopeToken> stream;

};

/*
struct ScopeAutoTracker {
    ScopeAutoTracker() {
        StmtRegistry::get().beginScope();
    }
    ~ScopeAutoTracker() {
        StmtRegistry::get().endScope();
    }

    ScopeAutoTracker(const ScopeAutoTracker&) = delete;
    ScopeAutoTracker& operator=(const ScopeAutoTracker&) = delete;

    ScopeAutoTracker(ScopeAutoTracker&&) = delete;
    ScopeAutoTracker& operator=(ScopeAutoTracker&&) = delete;

};
*/

struct ScopeAutoReg {
    ScopeAutoReg(ScopeTokenType type){
        if (type == ScopeTokenType::BEGIN_SCOPE) StmtRegistry::get().beginScope();
        else if (type == ScopeTokenType::END_SCOPE) StmtRegistry::get().endScope();
    }
    ScopeAutoReg(const char* name) {
        StmtRegistry::get().declareFunction(name);
    }
    ScopeAutoReg(ScopeTokenType type, const char* stmt) {
        StmtRegistry::get().push_stmt(type, stmt);
    }
};

struct ParseContext {
    std::vector<Entry> variables;
    std::vector<ScopeToken> scopes;
    void registerSubint(const char* name, int* ptr, int min, int max){
        variables.push_back(SubintData{name, ptr, min, max});
    }
    void registerEnum(const char * name, size_t size, const std::pair<std::string_view, int> * table) {
        variables.push_back(EnumData{name, size, table});
    }
    void registerBool(const char * name, bool initialValue) {
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