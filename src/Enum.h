#pragma once
#include <array>
#include <cstddef>
#include <string>

namespace reflect {

    constexpr std::size_t count_enums(const char* str){
        std::size_t count = (*str == '\0') ? 0 : 1;
        for (const char* p = str; *p; ++p){
            if (*p == ',') count++;
        }
        return count;
    }
    
    template<std::size_t N>
    constexpr std::array<std::string_view, N> parse_enum_data(const char* str){
        std::array<std::string_view, N> result{};
        const char* start = str;
        std::size_t index = 0;
        // skip leading whitespace
        while (*start == ' ' || *start == '\t') ++start;
        for (const char *p = start; ; ++p){
            if (*p == ',' || *p == '\0') {
                const char* end = p;
                // trim leading whitespace
                while (start < end && (*start == ' ' || *start == '\t')) ++start;
                // trim trailing whitespace
                while (start < end && (*end == ' ' || *end == '\t')) --end;
                // TODO extend this for manual value initialization
                // TODO this is very easy if using std::map, benchmark
                result[index++] = std::string_view(start, end-start);
                start = p + 1;
            }
            if (*p == '\0') break;
        }
        return result;
    }
}

template<typename T>
struct EnumInfo;

#define CUSTOM_ENUM(EnumName, ...)                                                                              \
    enum class EnumName { __VA_ARGS__ };                                                                        \
    template<> struct EnumInfo<EnumName> {                                                                      \
        static constexpr auto data =                                                                            \
            reflect::parse_enum_data<reflect::count_enums(#__VA_ARGS__)>(#__VA_ARGS__);                         \
    } ;                                                                                                         \
    namespace reflect {                                                                                         \
        constexpr std::string_view to_string(EnumName enumVal){                                                 \
            return EnumInfo<EnumName>::data[static_cast<std::size_t>(enumVal)];                                 \
    }                                                                                                           \
}