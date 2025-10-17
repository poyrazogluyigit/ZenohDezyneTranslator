#include "Wrapper.h"
#include <memory>

void Wrapper::open(zenoh::Config &config) {
    this->_session = std::make_unique<zenoh::Session>(zenoh::Session::open(std::move(config)));
}

template<class C, class D>
void Wrapper::declare_subscriber(zenoh::KeyExpr &key_expr, C &&on_sample, D &&on_drop,
    zenoh::Session::SubscriberOptions &&options, zenoh::ZResult *err) {

    }

void Wrapper::put(const zenoh::KeyExpr &key_expr, zenoh::Bytes &&payload, 
    zenoh::Session::PutOptions &&options, zenoh::ZResult *err) {

    }
// TODO research error
zenoh::Publisher Wrapper::declare_publisher(const zenoh::KeyExpr &key_expr, 
    zenoh::Session::PublisherOptions &&options, zenoh::ZResult *err) {
        return this->_session->declare_publisher(key_expr, options, err);
    }