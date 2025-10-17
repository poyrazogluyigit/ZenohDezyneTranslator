#include "zenoh.hxx"

class Wrapper {

    std::unique_ptr<zenoh::Session> _session;

    // potential shallow copy issue
    std::vector<std::string> _subscribedTopics;
    std::vector<std::string> _publishedTopics;

        void open(zenoh::Config &config);

        template<class C, class D>
        void declare_subscriber(zenoh::KeyExpr &key_expr, C &&on_sample, D &&on_drop,
            zenoh::Session::SubscriberOptions &&options=zenoh::Session::SubscriberOptions::create_default(),
            zenoh::ZResult *err=nullptr);

        void put(const zenoh::KeyExpr &key_expr, zenoh::Bytes &&payload, 
            zenoh::Session::PutOptions &&options=zenoh::Session::PutOptions::create_default(), 
            zenoh::ZResult *err=nullptr);

        zenoh::Publisher declare_publisher(const zenoh::KeyExpr &key_expr, 
            zenoh::Session::PublisherOptions &&options=zenoh::Session::PublisherOptions::create_default(), 
            zenoh::ZResult *err=nullptr);


};
