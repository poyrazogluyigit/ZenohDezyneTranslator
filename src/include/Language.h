#pragma once
#include "Registries.h"
#include "Preprocessing.h"


#define DSL_ENUM(EnumName, ...)                                                                     \
    enum class EnumName {__VA_ARGS__};                                                              \
    static AutoRegister _reg_enum_##EnumName(                                                       \
        #EnumName,                                                                                  \
        [](){                                                                                       \
            constexpr size_t N = proc::count_enums(#__VA_ARGS__);                                   \
            static constexpr auto data = proc::parse_enum_data<N>(#__VA_ARGS__);                    \
            return &data;                                                                           \
        }())                                                                                        \
    static ScopeAutoReg _reg_subint_stmt_##VarName(ScopeTokenType::VAR_DECL, #EnumName);                



#define DSL_SUBINT(VarName, LBound, UBound)                                                         \
    int VarName = LBound;                                                                           \
    static AutoRegister _reg_subint_##VarName(#VarName, &VarName, LBound, UBound);                  \
    static ScopeAutoReg _reg_subint_stmt_##VarName(ScopeTokenType::VAR_DECL, #VarName);                


#define DSL_BOOL(BoolName, InitialValue)                                                            \
    bool BoolName = InitialValue;                                                                   \
    static AutoRegister _reg_bool_##BoolName(#BoolName, InitialValue);                            \
    static ScopeAutoReg _reg_bool_stmt_##VarName(ScopeTokenType::VAR_DECL, #BoolName);                


// 1. Function Definition
// Creates a local vector, sets up the context, and returns the vector at the end.
#define BEHAVIOR(funcName)                                                      \
    static ScopeAutoReg _begin_fn_##funcName(#funcName);                        \
    static ScopeAutoReg _begin_scope_##funcName(ScopeTokenType::BEGIN_SCOPE);   \
    constexpr void funcName()  {                                                \
        /* User code follows */

#define END_BEHAVIOR                                                        \
    }                                                                       \
    static ScopeAutoReg(ScopeTokenType::END_SCOPE);

// TODO isimsiz degiskenler hata verecek

#define DSL_BEGIN_GUARD(expr)                                   \
    static_assert(validate(#expr), "Valid");                        \
    static ScopeAutoReg(ScopeTokenType::IF_STMT, #expr);            \
    static ScopeAutoReg(ScopeTokenType::BEGIN_SCOPE);               \
    if (expr)

#define DSL_END_GUARD \
    static ScopeAutoReg(ScopeTokenType::END_SCOPE);