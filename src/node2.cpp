#include "Wrapper.h"
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

void c_on_sample(const zenoh::Sample &sample){
    std::cout << "Received " << sample.get_payload().as_string() << std::endl;
}

int main(){

    Wrapper wrapper;

    zenoh::Config conf = zenoh::Config::create_default();
    wrapper.open(conf);

    auto pub = wrapper.declare_publisher("demo/wrapper/2");
    auto sub = wrapper.declare_subscriber("demo/wrapper/1", &c_on_sample, zenoh::closures::none);

    while (1){
        pub.put("Sent from 2");
        std::this_thread::sleep_for(200ms);
    }

    return 0;
}