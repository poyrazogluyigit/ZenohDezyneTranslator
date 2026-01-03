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

    std::this_thread::sleep_for(50s);
    ZENOH_PUT(carDetectedPub, "Car detected");
    std::cout << "Car detected" << std::endl;


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