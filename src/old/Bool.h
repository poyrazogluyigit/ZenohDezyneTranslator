#define DSL_BOOL(BoolName, InitialValue) \
    constexpr std::string var_name = #BoolName; \
    bool #BoolName = #InitialValue;