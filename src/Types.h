#pragma once
#include <array>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <cstddef>
#include <string>

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
        for (std::ptrdiff_t i = std::size(str) - 1U; i>=0; --i){
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

#define DSL_ENUM(EnumName, ...)                                                                     \
    enum class EnumName {__VA_ARGS__};                                                              \
    static AutoRegister _reg_enum_##EnumName(                                                       \
        #EnumName,                                                                                  \
        [](){                                                                                       \
            constexpr size_t N = proc::count_enums(#__VA_ARGS__);                                   \
            static constexpr auto data = proc::parse_enum_data<N>(#__VA_ARGS__);                    \
            return &data;                                                                           \
        }())                                                                                        \


#define DSL_SUBINT(VarName, LBound, UBound)                                                         \
    int VarName = LBound;                                                                           \
    static AutoRegister _reg_subint_##VarName(#VarName, &VarName, LBound, UBound);                  \


#define DSL_BOOL(BoolName, InitialValue)                                                            \
    bool BoolName = InitialValue;                                                                   \
    static AutoRegister _reg_subint_##BoolName(#BoolName, InitialValue);                            \
