#include "zenoh.hxx"

class Wrapper {

    std::unique_ptr<zenoh::Session> _session;

    // potential shallow copy issue
    std::vector<std::string> _subscribedTopics;
    std::vector<std::string> _publishedTopics;

    std::unique_ptr<zenoh::Subscriber<void>> _pubQuery;
    std::unique_ptr<zenoh::Subscriber<void>> _subQuery;

    void _sendSubscribers();
    void _sendPublishers();

    public:
        // TODO delete default constructor? idk if it
        // makes sense
        void open(zenoh::Config &config);

        template<class C, class D>
        zenoh::Subscriber<void> declare_subscriber(const zenoh::KeyExpr &key_expr, C &&on_sample, D &&on_drop,
            zenoh::Session::SubscriberOptions &&options=zenoh::Session::SubscriberOptions::create_default(),
            zenoh::ZResult *err=nullptr) {
            this->_subscribedTopics.push_back(std::string(key_expr.as_string_view()));
            return this->_session->declare_subscriber(key_expr, on_sample, on_drop, std::move(options), err);
        }

        void put(const zenoh::KeyExpr &key_expr, zenoh::Bytes &&payload, 
            zenoh::Session::PutOptions &&options=zenoh::Session::PutOptions::create_default(), 
            zenoh::ZResult *err=nullptr);

        zenoh::Publisher declare_publisher(const zenoh::KeyExpr &key_expr, 
            zenoh::Session::PublisherOptions &&options=zenoh::Session::PublisherOptions::create_default(), 
            zenoh::ZResult *err=nullptr);


};
