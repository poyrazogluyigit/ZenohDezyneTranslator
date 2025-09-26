import zenoh

class Wrapper:

    _subs = []
    _pubs = []

    _session = None

    _subQueryEngine = None
    _pubQueryEngine = None

    @staticmethod
    def __sendSubscribers(query):
        try:
            subs = str(Wrapper._session.info.zid()) + " "
            for sub in Wrapper._subs:
                subs += sub
                subs += "\n"
            query.reply(
                key_expr=query.selector.key_expr,
                payload=subs
            )
            query.drop()
        except Exception as e:
            print(e)

    @staticmethod
    def __sendPublishers(query):
        try:
            pubs = str(Wrapper._session.info.zid()) + " "
            for pub in Wrapper._pubs:
                pubs += pub
                pubs += "\n"
            query.reply(
                key_expr=query.selector.key_expr,
                payload=pubs
            )
            query.drop()
        except Exception as e:
            print(e)

    @classmethod
    def open(cls, *args, **kwargs):
        cls._session = zenoh.open(*args, **kwargs)
        cls._subQueryEngine = cls._session.declare_queryable("@cmpe491/subscribers", handler=cls.__sendSubscribers)
        cls._pubQueryEngine = cls._session.declare_queryable("@cmpe491/publishers", handler=cls.__sendPublishers)
        print(cls._session.info.zid())


    @classmethod
    def declare_subscriber(cls, *args, **kwargs):
        try:
            key_expr = ""
            if (len(args) > 0):
                cls._subs.append(args[0])
            else:
                cls._subs.append(kwargs["key_expr"])
            return cls._session.declare_subscriber(*args, **kwargs)
        except KeyError:
            print("Error at declare_subscriber()")
            exit()

    @classmethod
    def declare_publisher(cls, *args, **kwargs):
        try:
            if (len(args) > 0):
                cls._pubs.append(args[0])
            else:
                cls._pubs.append(kwargs["key_expr"])
            return cls._session.declare_publisher(*args, **kwargs)
        except KeyError:
            print("Error at declare_publisher()")