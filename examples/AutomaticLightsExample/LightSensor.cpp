#include <thread>

#include "zenoh.hxx"        // IWYU pragma: keep
#include "Language.h"   

using namespace std::chrono_literals;

bool highLight = false;

int main(){

    zenoh::Config config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    ZENOH_DECLARE_PUBLISHER(session, lightSensorLow, "autoLights/lightSensor/lowLight");
    ZENOH_DECLARE_PUBLISHER(session, lightSensorHigh, "autoLights/lightSensor/highLight");

    auto ctlPub = session.declare_publisher("autoLights/ctl/lightSensor/reply");
    auto ctlSub = session.declare_subscriber(
        "autoLights/ctl/lightSensor",
        [&](const zenoh::Sample &sample) {
            auto command = sample.get_payload().as_string();
            if (command == "lowLight") lightSensorLow.put("lowLight");
            else if(command == "highLight") lightSensorHigh.put("highLight");
        }, zenoh::closures::none);

    while(true) {
        std::this_thread::sleep_for(20s);
        // lightSensorSession.put("Low light");
        // std::cout << "Low light" << std::endl;
        // std::this_thread::sleep_for(20s);
        // lightSensorSession.put("High light") ;
        // std::cout << "High light" << std::endl;
    }
    return 0;
}