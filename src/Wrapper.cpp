#include "Wrapper.h"
#include <memory>

// TODO these two can be written as a single function
void Wrapper::_sendSubscribers(){
    std::string payload = this->_session->get_zid().to_string() + " ";
    for (auto &str: this->_subscribedTopics){
        payload += str;
    }
    this->_session->put("@cmpe491/subscribers/reply", payload);
}

void Wrapper::_sendPublishers(){
    std::string payload = this->_session->get_zid().to_string() + " ";
    for (auto &str: this->_publishedTopics){
        payload += str;
    }
    this->_session->put("@cmpe491/publishers/reply", payload);
}

void Wrapper::add_transition(std::string expr, std::string_view currentState, std::string_view nextState) {
    this->_transitions.push_back(Wrapper::Transition(expr, std::string(currentState), std::string(nextState)));
}

void Wrapper::open(zenoh::Config &config) {
    this->_session = std::make_unique<zenoh::Session>(zenoh::Session::open(std::move(config)));
    // declare publishers/subscribers for controller querying
    this->_subQuery = std::make_unique<zenoh::Subscriber<void>>(this->_session->declare_subscriber("@cmpe491/subscribers", 
        [this](const zenoh::Sample& sample){this->_sendSubscribers();}, zenoh::closures::none)); 
    this->_pubQuery = std::make_unique<zenoh::Subscriber<void>>(this->_session->declare_subscriber("@cmpe491/publishers", 
        [this](const zenoh::Sample& sample){this->_sendPublishers();}, zenoh::closures::none));
}

void Wrapper::put(const zenoh::KeyExpr &key_expr, zenoh::Bytes &&payload, 
    zenoh::Session::PutOptions &&options, zenoh::ZResult *err) {
     
    }

// TODO research error
zenoh::Publisher Wrapper::declare_publisher(const zenoh::KeyExpr &key_expr, 
    zenoh::Session::PublisherOptions &&options, zenoh::ZResult *err) {
        this->_publishedTopics.push_back(std::string(key_expr.as_string_view()));
        return this->_session->declare_publisher(key_expr, std::move(options), err);
    }