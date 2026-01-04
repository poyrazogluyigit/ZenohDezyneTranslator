// #include <functional>
// #include <chrono>
#include <thread>

#include "zenoh.hxx"    // IWYU pragma: keep
#include "Language.h"

using namespace std::chrono_literals;

int carDetected = 0;
int carPassed = 0;

int main(){

    zenoh::Config config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    ZENOH_DECLARE_PUBLISHER(session, carDetectedPub, "autoLights/frontCamera/carDetected");
    ZENOH_DECLARE_PUBLISHER(session, carPassedPub, "autoLights/frontCamera/carPassed");

    auto ctlPub = session.declare_publisher("autoLights/ctl/frontCamera/reply");

    auto ctlSub = session.declare_subscriber(
        "autoLights/ctl/frontCamera",
        [&](const zenoh::Sample &sample) {
            auto command = sample.get_payload().as_string();
            if (command == "carDetected") carDetectedPub.put("carDetected");
            else if(command == "carPassed") carPassedPub.put("carPassed");
        }, zenoh::closures::none);


    while(true) {
        std::this_thread::sleep_for(30s);
        // timerSession.put("Car detected");
        // std::cout << "Car detected" << std::endl;
        // std::this_thread::sleep_for(5s);
        // timerSession.put("Car passed");
        // std::cout << "Car passed" << std::endl;
    }
    return 0;
}