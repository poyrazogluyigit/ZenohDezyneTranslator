#pragma once


#include <tuple>
#include <utility>
#include <string>

// Namespace to keep the global scope clean
namespace sub_macro {

    template <typename Session, typename... Args>
    struct SubscriberBuilder {
        Session& session;
        std::string topic;
        std::tuple<Args...> extra_args;

        SubscriberBuilder(Session& s, std::string t, Args&&... args)
            : session(s), topic(std::move(t)), extra_args(std::forward<Args>(args)...) {}

        template <typename Callback>
        auto operator+(Callback&& callback) {
            // Unpack the stored arguments (closure, etc.) and call the real function
            return std::apply([&](auto&&... unpacked_args) {
                return session.declare_subscriber(
                    topic, 
                    std::forward<Callback>(callback), 
                    std::forward<decltype(unpacked_args)>(unpacked_args)...
                );
            }, extra_args);
        }
    };

    template <typename Session, typename... Args>
    SubscriberBuilder<Session, Args...> make_builder(Session& s, std::string t, Args&&... args) {
        return SubscriberBuilder<Session, Args...>(s, std::move(t), std::forward<Args>(args)...);
    }
}


#define ZENOH_DECLARE_PUBLISHER(session, publisher, ...) \
    auto publisher = session.declare_publisher(__VA_ARGS__)

#define ZENOH_DECLARE_SUBSCRIBER(session, subscriber, topic, ...)       \
    auto subscriber = sub_macro::make_builder(session, topic, ##__VA_ARGS__) +

#define BEGIN {

#define END }

#define ZENOH_PUT(pub, ...) \
    pub.put(__VA_ARGS__)


#define DSL_DECLARE_ENUM(EnumName, ...)                                       \
    enum class EnumName {__VA_ARGS__};                                                               
    
#define DSL_ENUM_VAR(EnumName, VarName, Enum)       \
    EnumName VarName = EnumName::Enum

#define DSL_SUBINT(VarName, LBound, UBound)                      \
    int VarName = LBound;                                                                              


#define DSL_BOOL(BoolName, InitialValue)                            \
    bool BoolName = InitialValue;
    
#define DSL_SET(expr) expr

// FIXME These guards don't have any mutex in c++
#define DSL_GUARD(expr) if (expr)

#define DSL_IF(expr) if (expr)

#define DSL_ELSE_IF(expr) else if (expr)