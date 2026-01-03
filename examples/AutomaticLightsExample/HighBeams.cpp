#include <thread>

#include "Language.h"
#include "zenoh.hxx"    // IWYU pragma: keep

using namespace std::chrono_literals;

int main(){

    bool highBeams = false;

    zenoh::Config config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    ZENOH_DECLARE_SUBSCRIBER(session, highBeamsTurnOn,
        "autoLights/highBeams/turnOn", zenoh::closures::none)
        [&](zenoh::Sample &sample) {
            std::cout << "High beams turned on" << std::endl;
        };
    
        ZENOH_DECLARE_SUBSCRIBER(session, highBeamsTurnOff,
        "autoLights/highBeams/turnOff", zenoh::closures::none)  
        [&](zenoh::Sample &sample) {
            std::cout << "High beams turned off" << std::endl;
        };

    while(true) {
        std::this_thread::sleep_for(100ms);
    }
    return 0;
}