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
    
    // lightShifter.toggle()
    ZENOH_DECLARE_SUBSCRIBER(session, lShifterToggle, 
        "autoLights/lightShifter/toggle", zenoh::closures::none)
        [&](const zenoh::Sample &sample) {
            DSL_GUARD(state == SystemState::Uninitialized) {
                ZENOH_PUT(tCancel, "cancel");
                DSL_SET(timerState = TimerState::Disarmed);
                // TODO toggleHighBeams()
            }
            DSL_GUARD(state == SystemState::Operational) {
                DSL_GUARD(mode == BeamsMode::Manual) {
                    ZENOH_PUT(tCancel, "cancel");
                    DSL_SET(timerState = TimerState::Disarmed);
                    // TODO toggleHighBeams();
                }
                DSL_GUARD(mode == BeamsMode::Automatic) {
                    DSL_GUARD(beams == HighBeams::On) {
                        ZENOH_PUT(hBeamsTurnOff, "turnOff");
                        DSL_SET(beams = HighBeams::Off);
                        DSL_SET(mode = BeamsMode::Manual);
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                    DSL_GUARD(beams == HighBeams::Off) {
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
            DSL_GUARD(state == SystemState::Operational) {
                DSL_GUARD(mode == BeamsMode::Manual) {
                    DSL_GUARD(timerState == TimerState::Disarmed) {
                        DSL_GUARD(beams == HighBeams::Off) {
                            ZENOH_PUT(tCreate, "5000");
                            DSL_SET(timerState = TimerState::Armed);
                        }
                    }
                }
                DSL_GUARD(mode == BeamsMode::Automatic) {
                    DSL_GUARD(beams == HighBeams::On) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                    DSL_GUARD(beams == HighBeams::Off) {
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
            DSL_GUARD(state == SystemState::Operational) {
                DSL_GUARD(mode == BeamsMode::Manual) {
                    DSL_GUARD(timerState == TimerState::Armed) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                }
                DSL_GUARD(mode == BeamsMode::Automatic) {
                    DSL_GUARD(beams == HighBeams::On) {
                        DSL_GUARD(timerState == TimerState::Disarmed) {
                            ZENOH_PUT(tCreate, "5000");
                            DSL_SET(timerState = TimerState::Armed);
                        }
                    }
                    DSL_GUARD(beams == HighBeams::Off) {
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
            DSL_GUARD(state == SystemState::Operational) {
                DSL_GUARD(mode == BeamsMode::Manual) {
                    DSL_GUARD(timerState == TimerState::Armed) {
                        ZENOH_PUT(tCancel, "cancel");
                        DSL_SET(timerState = TimerState::Disarmed);
                    }
                }
                DSL_GUARD(mode == BeamsMode::Automatic) {
                    DSL_GUARD(beams == HighBeams::On) {
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
            DSL_GUARD(state == SystemState::Operational) {
                DSL_GUARD(mode == BeamsMode::Automatic) {
                    DSL_GUARD(beams == HighBeams::Off) {
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
            DSL_GUARD(state == SystemState::Operational) {
                DSL_GUARD(mode == BeamsMode::Manual) {
                    DSL_SET(mode = BeamsMode::Automatic);
                    ZENOH_PUT(hBeamsTurnOn, "turnOn");
                    DSL_SET(beams = HighBeams::On);
                }
                DSL_GUARD(mode == BeamsMode::Automatic) {
                    DSL_GUARD(lowLight) {
                        // TODO toggleHighBeams();
                        DSL_SET(lowLight = !lowLight);
                    }
                    DSL_GUARD(!lowLight) {
                        // TODO toggleHighBeams();
                        DSL_SET(lowLight = !lowLight);
                    }
                }
            }
        };
    
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}