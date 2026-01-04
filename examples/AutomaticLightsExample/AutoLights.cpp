// #include <iostream>
#include <chrono>
#include <thread>

#include "zenoh.hxx"    // IWYU pragma: keep
#include "Language.h"

DSL_DECLARE_ENUM(SystemState, Uninitialized, Operational);
DSL_DECLARE_ENUM(BeamsMode, Manual, Automatic);
DSL_DECLARE_ENUM(HighBeams, On, Off);
DSL_DECLARE_ENUM(TimerState, Armed, Disarmed);


int main(){

    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    DSL_ENUM_VAR(SystemState, state, Uninitialized);
    DSL_ENUM_VAR(BeamsMode, mode, Manual);
    DSL_ENUM_VAR(HighBeams, beams, Off);
    DSL_ENUM_VAR(TimerState, timerState, Disarmed);

    DSL_BOOL(lowLight, false);

    // out events
    ZENOH_DECLARE_PUBLISHER(session, tCreate, "autoLights/lightsTimer/create");
    ZENOH_DECLARE_PUBLISHER(session, tCancel, "autoLights/lightsTimer/cancel");
    ZENOH_DECLARE_PUBLISHER(session, hBeamsTurnOn, "autoLights/highBeams/turnOn");
    ZENOH_DECLARE_PUBLISHER(session, hBeamsTurnOff, "autoLights/highBeams/turnOff");
    
    // module.Initialize()
    ZENOH_DECLARE_SUBSCRIBER(session, mInitialize,
        "autoLights/module/Initialize", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Uninitialized) {
                DSL_SET(state = SystemState::Operational);
            }
        };    
    // module.Terminate()
    ZENOH_DECLARE_SUBSCRIBER(session, mTerminate,
        "autoLights/system/Terminate", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Operational) {
                DSL_SET(state = SystemState::Uninitialized);
                DSL_SET(mode = BeamsMode::Manual);
                DSL_SET(beams = HighBeams::Off);
                DSL_SET(timerState = TimerState::Disarmed);
                ZENOH_PUT(tCancel, "cancel");
                ZENOH_PUT(hBeamsTurnOff, "turnOff");
            }
        };  
    // lightShifter.toggle()
    ZENOH_DECLARE_SUBSCRIBER(session, lShifterToggle, 
        "autoLights/lightShifter/toggle", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Uninitialized) {
                ZENOH_PUT(tCancel, "cancel");
                DSL_SET(timerState = TimerState::Disarmed);
                // toggleHighBeams()
                DSL_IF(beams == HighBeams::On) DSL_SET(beams = HighBeams::Off);
                DSL_ELSE_IF(beams == HighBeams::Off) DSL_SET(beams = HighBeams::On);

            }
            DSL_ELSE_IF(state == SystemState::Operational) {
                DSL_IF(mode == BeamsMode::Manual) {
                    ZENOH_PUT(tCancel, "cancel");
                    DSL_SET(timerState = TimerState::Disarmed);
                    // toggleHighBeams();
                    DSL_IF(beams == HighBeams::On) DSL_SET(beams = HighBeams::Off);
                    DSL_ELSE_IF(beams == HighBeams::Off) DSL_SET(beams = HighBeams::On);
                }
                DSL_ELSE_IF(mode == BeamsMode::Automatic) {
                    DSL_IF(beams == HighBeams::On) {
                        ZENOH_PUT(hBeamsTurnOff, "turnOff");
                        DSL_SET(beams = HighBeams::Off);
                        DSL_SET(mode = BeamsMode::Manual);
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                    DSL_ELSE_IF(beams == HighBeams::Off) {
                        ZENOH_PUT(hBeamsTurnOn, "turnOn");
                        DSL_SET(beams = HighBeams::On);
                        DSL_SET(mode = BeamsMode::Manual);
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                }
            }
        };
    // lightSensor.lowLight()
    ZENOH_DECLARE_SUBSCRIBER(session, lSensorLowLight,
        "autoLights/lightSensor/lowLight", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Operational) {
                DSL_IF(mode == BeamsMode::Manual) {
                    DSL_IF(timerState == TimerState::Disarmed) {
                        DSL_IF(beams == HighBeams::Off) {
                            ZENOH_PUT(tCreate, "5000");
                            DSL_SET(timerState = TimerState::Armed);
                        }
                    }
                }
                DSL_ELSE_IF(mode == BeamsMode::Automatic) {
                    DSL_IF(beams == HighBeams::On) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                    DSL_ELSE_IF(beams == HighBeams::Off) {
                        DSL_GUARD(timerState == TimerState::Disarmed) {
                            ZENOH_PUT(tCreate, "5000");
                            DSL_SET(timerState = TimerState::Armed);
                        }       
                    }
                }
            }
        };
    // lightSensor.highLight()
    ZENOH_DECLARE_SUBSCRIBER(session, lSensorHighLight,
        "autoLights/lightSensor/highLight", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Operational) {
                DSL_IF(mode == BeamsMode::Manual) {
                    DSL_IF(timerState == TimerState::Armed) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                }
                DSL_ELSE_IF(mode == BeamsMode::Automatic) {
                    DSL_IF(beams == HighBeams::On) {
                        DSL_IF(timerState == TimerState::Disarmed) {
                            ZENOH_PUT(tCreate, "5000");
                            DSL_SET(timerState = TimerState::Armed);
                        }
                    }
                    DSL_ELSE_IF(beams == HighBeams::Off) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                }
            }
                
        };
    // frontCamera.carDetected()
    ZENOH_DECLARE_SUBSCRIBER(session, fCamCarDetected,
        "autoLights/frontCamera/carDetected", zenoh::closures::none) 
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Operational) {
                DSL_IF(mode == BeamsMode::Manual) {
                    DSL_IF(timerState == TimerState::Armed) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                }
                DSL_ELSE_IF(mode == BeamsMode::Automatic) {
                    DSL_IF(beams == HighBeams::On) {
                        ZENOH_PUT(hBeamsTurnOff, "turnOff");
                        DSL_SET(beams = HighBeams::Off);
                    }
                }
            }
        };
    // frontCamera.carPassed()
    ZENOH_DECLARE_SUBSCRIBER(session, fCamCarPassed,
        "autoLights/frontCamera/carPassed", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_IF(state == SystemState::Operational) {
                DSL_IF(mode == BeamsMode::Automatic) {
                    DSL_IF(beams == HighBeams::Off) {
                        ZENOH_PUT(hBeamsTurnOn, "turnOn");
                        DSL_SET(beams = HighBeams::On);
                    }
                }
            }
        };
    // lightsTimer.timeout()
    ZENOH_DECLARE_SUBSCRIBER(session, lTimerTimeout,
        "autoLights/lightsTimer/timeout", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_SET(timerState = TimerState::Disarmed);
            DSL_IF(state == SystemState::Operational) {
                DSL_IF(mode == BeamsMode::Manual) {
                    DSL_SET(mode = BeamsMode::Automatic);
                    ZENOH_PUT(hBeamsTurnOn, "turnOn");
                    DSL_SET(beams = HighBeams::On);
                }
                DSL_ELSE_IF(mode == BeamsMode::Automatic) {
                    DSL_IF(lowLight) {
                        // toggleHighBeams();
                        DSL_IF(beams == HighBeams::On) DSL_SET(beams = HighBeams::Off);
                        DSL_ELSE_IF(beams == HighBeams::Off) DSL_SET(beams = HighBeams::On);
                        DSL_SET(lowLight = !lowLight);
                    }
                    DSL_ELSE_IF(!lowLight) {
                        // toggleHighBeams();
                        DSL_IF(beams == HighBeams::On) DSL_SET(beams = HighBeams::Off);
                        DSL_ELSE_IF(beams == HighBeams::Off) DSL_SET(beams = HighBeams::On);
                        DSL_SET(lowLight = !lowLight);
                    }
                }
            }
        };

    // here, we define some convenient pub/subs for getting system input.
    // Normally, these would be defined elsewhere in the system. That's why these
    // are not using the DSL macros.

    auto togglePub = session.declare_publisher("autoLights/lightShifter/toggle");
    auto initPub = session.declare_publisher("autoLights/module/Initialize");
    auto termPub = session.declare_publisher("autoLights/system/Terminate");

    auto ctlPub = session.declare_publisher("autoLights/ctl/AutoLights/reply");

    // define control publishers/subscribers
    auto ctlSub = session.declare_subscriber("autoLights/ctl/AutoLights",
        [&](const zenoh::Sample &sample) {
            std::string command = sample.get_payload().as_string();
            if (command == "initialize") {
                initPub.put("initialize");
            } else if (command == "terminate") {
                termPub.put("terminate");
            } else if (command == "toggleLightShifter") {
                togglePub.put("toggle");
            } 
            else if (command == "status") {
                auto status = std::string("SystemState: ") + (state == SystemState::Uninitialized ? "Uninitialized" : "Operational")
                    + ", BeamsMode: " + (mode == BeamsMode::Manual ? "Manual" : "Automatic")
                    + ", HighBeams: " + (beams == HighBeams::On ? "On" : "Off")
                    + ", TimerState: " + (timerState == TimerState::Armed ? "Armed" : "Disarmed");
                ctlPub.put(status);

            }else {
                std::cout << "Unknown command: " << command << std::endl;
            }
        }, zenoh::closures::none);
    
    while(true){
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}