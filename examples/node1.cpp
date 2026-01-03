#include <iostream>
#include <thread>
#include <zenoh/api/closures.hxx>

#include "zenoh.hxx"    // IWYU pragma: keep
#include "Language.h"

using namespace std::literals::chrono_literals;

int main(){

    zenoh::Config conf = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(conf));

    ZENOH_DECLARE_PUBLISHER(session, pub,"demo/wrapper/1");
    ZENOH_DECLARE_SUBSCRIBER(session, sub, "demo/wrapper/2", zenoh::closures::none)
    [](const zenoh::Sample &sample){
        std::cout << "Received " << sample.get_payload().as_string() << std::endl;
    };

    while (1){
        std::string input;
        std::cin >> input;
        ZENOH_PUT(pub, input);
        std::this_thread::sleep_for(200ms);
    }

    return 0;
}