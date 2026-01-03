#include <iostream>
#include <thread>

#include "zenoh.hxx"    // IWYU pragma: keep
#include "Language.h"

using namespace std::literals::chrono_literals;

DSL_DECLARE_ENUM(State, A, B);

DSL_ENUM_VAR(State, state, A);

int main(){

    zenoh::Config conf = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(conf));

    ZENOH_DECLARE_PUBLISHER(session, pub, "demo/wrapper/2");

    // bir alttaki tamamen farazi bir ornek
    DSL_SUBINT(x, 0, 5);

    ZENOH_DECLARE_SUBSCRIBER(session, sub, 
        "demo/wrapper/1", zenoh::closures::none) 
        [&pub, &x](const zenoh::Sample& sample) BEGIN
        std::cout << sample.get_payload().as_string() << std::endl;
        DSL_GUARD(x < 3) BEGIN
            DSL_SET(state = State::B);
        END
        DSL_GUARD(x >= 3) DSL_SET(state = State::A);
        
        DSL_SET(x++);
        DSL_GUARD(x == 5) 
            DSL_SET(x = 0);

        ZENOH_PUT(pub, std::to_string(static_cast<int>(state)));
    END;

    while (1){
        // pub.put("Sent from 2");
        std::this_thread::sleep_for(200ms);
    }

    return 0;
}