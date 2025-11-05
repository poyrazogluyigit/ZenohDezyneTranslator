#include "zenoh.hxx"
#include <zenoh/api/subscriber.hxx>

/*
    Transition format:
        add_transition(std::function, firstState, nextState)
        ex. 
            add_transition([](){x > 3}, state.A, state.B)
        notes:
            lambda ya da std::function'daki expression'i okumak gerekecek. 
            direkt bir yol galiba yok (cunku compilation) ama gpt diyor ki:
                1 - expression templates
                2 - macro captures
            araciligiyla compile time'da/run time'da ast veya string insa edebilirsin
            aslinda bu ikisinden birini kullanmak feci mantikli ama o sonraki is olsun, simdilik string devam edelim.
        add_transition(std::string, firstState, nextState)
        ex.
            add_transition("x > 3", state.A, state.B)

    Enum gommece:
    reflective enums diye geciyormus
*/

// FIXME this code smells bad af

class Wrapper {

    using Transition = std::tuple<std::string, std::string, std::string>;

    std::unique_ptr<zenoh::Session> _session;

    // potential shallow copy issue
    std::vector<std::string> _subscribedTopics;
    std::vector<std::string> _publishedTopics;
    std::vector<std::string> _enums;
    std::vector<std::string> _bools;
    std::vector<std::string> _subints;
    std::vector<Transition> _transitions;

    std::unique_ptr<zenoh::Subscriber<void>> _pubQuery;
    std::unique_ptr<zenoh::Subscriber<void>> _subQuery;
    std::unique_ptr<zenoh::Subscriber<void>> _transitionQuery;
    std::unique_ptr<zenoh::Subscriber<void>> _enumQuery;
    std::unique_ptr<zenoh::Subscriber<void>> _boolQuery;
    std::unique_ptr<zenoh::Subscriber<void>> _subintQuery;

    void _sendSubscribers();
    void _sendPublishers();
    void _sendTransitions();
    void _sendEnums();
    void _sendBools();
    void _sendSubints();

    public:
        void open(zenoh::Config &config);

        void add_transition(std::string expr, std::string_view currentState, std::string_view nextState);

        template<class C, class D>
        zenoh::Subscriber<void> declare_subscriber(const zenoh::KeyExpr &key_expr, C &&on_sample, D &&on_drop,
            zenoh::Session::SubscriberOptions &&options=zenoh::Session::SubscriberOptions::create_default(),
            zenoh::ZResult *err=nullptr) {
            this->_subscribedTopics.push_back(std::string(key_expr.as_string_view()));
            return this->_session->declare_subscriber(key_expr, on_sample, on_drop, std::move(options), err);
        }

        void register_enum(const std::string_view * enumData, std::size_t enumSize);
        void register_bool(std::string_view &variable_name);
        void register_subint(const std::string& variable_name, int lower_bound, int upper_bound);

        void put(const zenoh::KeyExpr &key_expr, zenoh::Bytes &&payload, 
            zenoh::Session::PutOptions &&options=zenoh::Session::PutOptions::create_default(), 
            zenoh::ZResult *err=nullptr);

        zenoh::Publisher declare_publisher(const zenoh::KeyExpr &key_expr, 
            zenoh::Session::PublisherOptions &&options=zenoh::Session::PublisherOptions::create_default(), 
            zenoh::ZResult *err=nullptr);


};
