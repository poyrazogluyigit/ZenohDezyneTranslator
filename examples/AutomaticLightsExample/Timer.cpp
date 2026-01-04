#include <functional>
#include <chrono>
#include <atomic>
#include <memory>
#include <thread>

#include "zenoh.hxx"    // IWYU pragma: keep
#include "Language.h"

typedef std::chrono::milliseconds Interval;
typedef std::function<void(void)> Timeout;

struct TimerThread {
    std::thread thread;
    std::atomic<bool> running;
    Timeout timeout;

    ~TimerThread() {
        this->running = false;
        if (this->thread.joinable()) this->thread.join();
    }
};

std::shared_ptr<TimerThread> start(const Interval &interval, const Timeout &timeout){
    auto th = std::make_shared<TimerThread>();
    th->running = true;
    th->timeout = timeout;
    th->thread = std::thread([th, interval]() {
        std::this_thread::sleep_for(interval);
        if (th->running) th->timeout();
    });
    th->thread.detach();
    return th;
}

void stop(std::shared_ptr<TimerThread> th){
    if (th) {
        th->running = false;
    }
}

int main() {
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    auto ctlPub = session.declare_publisher("autoLights/ctl/lightsTimer/reply");

    std::shared_ptr<TimerThread> th;
    ZENOH_DECLARE_PUBLISHER(session, pub, "autoLights/lightsTimer/timeout");

    // Subscriber for timer create
    ZENOH_DECLARE_SUBSCRIBER(session, timerCreateSub, 
        "autoLights/lightsTimer/create", zenoh::closures::none)
        [&pub, &th, &ctlPub, &session](const zenoh::Sample &sample) {
            if (th) {
                stop(th); 
            }
            std::string message = sample.get_payload().as_string();
            int time = std::stoi(message);
            th = start(Interval(time), [&]() {
                ZENOH_PUT(pub, "Timeout");
                ctlPub.put("Timeout");
            });
            ctlPub.put("Timer created for " + std::to_string(time) + " ms");
        };

    // Subscriber for timer cancel
    ZENOH_DECLARE_SUBSCRIBER(session, timerCancelSub, "autoLights/lightsTimer/cancel",
        zenoh::closures::none)
        [&ctlPub, &th](const zenoh::Sample &sample) {
            std::string message = sample.get_payload().as_string();
            (void)message; // unused
            stop(th);
            ctlPub.put("Timer cancelled");
        };

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}