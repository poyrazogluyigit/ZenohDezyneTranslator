#include "zenoh.hxx"    // IWYU pragma: keep



int main(){
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    // declare publishers/subscribers corresponding to the components
    auto lightSensorPub = session.declare_publisher("autoLights/ctl/lightSensor");
    auto frontCameraPub = session.declare_publisher("autoLights/ctl/frontCamera");
    auto mainPub = session.declare_publisher("autoLights/ctl/AutoLights");

    // declare subscribers corresponding to each component under .../reply
    // the callback should simply prepend the component name and print the response
    auto lightSensorSub = session.declare_subscriber(
        "autoLights/ctl/lightSensor/reply",
        [&](const zenoh::Sample &sample) {
            std::cout << "lightSensor: " << sample.get_payload().as_string() << std::endl;
        }, zenoh::closures::none);
    
    auto frontCameraSub = session.declare_subscriber(
        "autoLights/ctl/frontCamera/reply",
        [&](const zenoh::Sample &sample) {
            std::cout << "frontCamera: " << sample.get_payload().as_string() << std::endl;
        }, zenoh::closures::none);

    auto highBeamsSub = session.declare_subscriber(
        "autoLights/ctl/highBeams/reply",
        [&](const zenoh::Sample &sample) {
            std::cout << "highBeams: " << sample.get_payload().as_string() << std::endl;
        }, zenoh::closures::none);         
    
    auto lightsTimerSub = session.declare_subscriber(
        "autoLights/ctl/lightsTimer/reply",
        [&](const zenoh::Sample &sample) {
            std::cout << "lightsTimer: " << sample.get_payload().as_string() << std::endl;
        }, zenoh::closures::none);
    
    auto mainSub = session.declare_subscriber(
        "autoLights/ctl/AutoLights/reply",  
        [&](const zenoh::Sample &sample) {
            std::cout << "AutoLights: " << sample.get_payload().as_string() << std::endl;
        }, zenoh::closures::none);  

    while(true) {
        std::string input;
        std::getline(std::cin, input);
        // parse input based on <file>/<command>
        auto delimPos = input.find('/');
        auto component = input.substr(0, delimPos);
        auto command = input.substr(delimPos + 1);
        // send the command to the appropriate component
        if (component == "lightSensor") {
            lightSensorPub.put(command);
        } else if (component == "frontCamera") {
            frontCameraPub.put(command);
        } else if (component == "AutoLights") {
            mainPub.put(command);
        } else {
            std::cout << "Component either unknown or does not accept input: " << component << std::endl;
        }
    }
    return 0;
}