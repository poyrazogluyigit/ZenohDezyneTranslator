#include "Wrapper.h"
#include "Types.h"
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

DSL_ENUM(State, A, B);

State state = State::A;

int main(){

    Wrapper wrapper;

    zenoh::Config conf = zenoh::Config::create_default();
    wrapper.open(conf);

    // bir alttaki tamamen farazi bir ornek
    wrapper.register_subint("x", 0, 3);
    wrapper.register_enum(EnumInfo<State>::get_enum_data(), EnumInfo<State>::get_enum_size());
    wrapper.add_transition("demo/wrapper/1: x < 3", reflect::to_string(State::A), reflect::to_string(State::B));
    wrapper.add_transition("demo/wrapper/1", reflect::to_string(State::B), reflect::to_string(State::A));


    auto pub = wrapper.declare_publisher("demo/wrapper/2");

    auto c_on_sample = [&pub](const zenoh::Sample &sample){
            std::cout << sample.get_payload().as_string() << std::endl;
            pub.put(reflect::to_string(state));
            if (state == State::A) {
                state = State::B;
            }
            else state = State::A;
    };

    auto sub = wrapper.declare_subscriber("demo/wrapper/1", c_on_sample, zenoh::closures::none);

    while (1){
        // pub.put("Sent from 2");
        std::this_thread::sleep_for(200ms);
    }

    return 0;
}