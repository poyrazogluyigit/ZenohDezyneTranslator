#include "Wrapper.h"
#include <memory>

void Wrapper::_sendSubscribers(){
    std::string payload = this->_session->get_zid().to_string() + "{";
    for (auto &str: this->_subscribedTopics){
        payload += str + ",";
    }
    payload += "}";
    this->_session->put("@cmpe491/subscribers/reply", payload);
}

void Wrapper::_sendPublishers(){
    std::string payload = this->_session->get_zid().to_string() + "{";
    for (auto &str: this->_publishedTopics){
        payload += str;
    }
    payload += "}";
    this->_session->put("@cmpe491/publishers/reply", payload);
}

void Wrapper::_sendTransitions(){
    std::string payload = this->_session->get_zid().to_string() + "{";
    for (auto &transition : this->_transitions) {
        payload += "-" + std::get<0>(transition) + "," + std::get<1>(transition) + "," + std::get<2>(transition);
    }
    payload += "}";
    this->_session->put("@cmpe491/transitions/reply", payload);
}

void Wrapper::_sendEnums(){
    std::string payload = this->_session->get_zid().to_string() + "{";
    for (auto &enumStr : this->_enums){
        payload += enumStr;
    }
    payload += "}";
    this->_session->put("@cmpe491/enums/reply", payload);
}

void Wrapper::_sendSubints(){
    std::string payload = this->_session->get_zid().to_string() + "{";
    for (auto &subintStr : this->_subints){
        payload += subintStr;
    }
    payload += "}";
    this->_session->put("@cmpe491/subints/reply", payload);
}

void Wrapper::_sendBools(){
    std::string payload = this->_session->get_zid().to_string() + "{";
    for (auto &boolStr : this->_bools){
        payload += boolStr;
    }
    payload += "}";
    this->_session->put("@cmpe491/bools/reply", payload);
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
    this->_transitionQuery = std::make_unique<zenoh::Subscriber<void>>(this->_session->declare_subscriber("@cmpe491/transitions", 
        [this](const zenoh::Sample &sample){this->_sendTransitions();}, zenoh::closures::none));
    this->_enumQuery = std::make_unique<zenoh::Subscriber<void>>(this->_session->declare_subscriber("@cmpe491/enums", 
        [this](const zenoh::Sample& sample){this->_sendEnums();}, zenoh::closures::none)); 
    this->_boolQuery = std::make_unique<zenoh::Subscriber<void>>(this->_session->declare_subscriber("@cmpe491/bools", 
        [this](const zenoh::Sample& sample){this->_sendBools();}, zenoh::closures::none));
    this->_subintQuery = std::make_unique<zenoh::Subscriber<void>>(this->_session->declare_subscriber("@cmpe491/subints", 
        [this](const zenoh::Sample &sample){this->_sendSubints();}, zenoh::closures::none));
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

void Wrapper::register_enum(const std::string_view *enumData, std::size_t enumSize){
    std::string enumStr;
    for (int i = 1; i < enumSize; i++){
        enumStr += std::string(enumData[0]) + "." + std::string(enumData[i]) + ";";
    }
    this->_enums.push_back(enumStr);
}

void Wrapper::register_bool(std::string_view &variable_name){
    this->_bools.push_back(std::string(variable_name) + ";");
}

void Wrapper::register_subint(const std::string &variable_name, int lower_bound, int upper_bound){
    std::string subint;
    subint += std::string(variable_name) + " {" + std::to_string(lower_bound) + ".." + std::to_string(upper_bound) + "};";
    this->_subints.push_back(subint);
}