import zenoh

class Wrapper:

    _subs = []
    _pubs = []
    _transitions = []

    _session = None

    _subQueryEngine = None
    _pubQueryEngine = None

    @staticmethod
    def __sendSubscribers(sample):
        try:
            subs = str(Wrapper._session.info.zid()) + " "
            for sub in Wrapper._subs:
                subs += sub
                subs += "\n"
            Wrapper._session.put(
                key_expr=str(sample.key_expr) + "/reply",
                payload=subs
            )
        except Exception as e:
            print(e)
    

    @staticmethod
    def __sendPublishers(sample):
        try:
            pubs = str(Wrapper._session.info.zid()) + " "
            for pub in Wrapper._pubs:
                pubs += pub
                pubs += "\n"
            Wrapper._session.put(
                key_expr=str(sample.key_expr) + "/reply",
                payload=pubs
            )
        except Exception as e:
            print(e)

    @classmethod
    def open(cls, *args, **kwargs):
        cls._session = zenoh.open(*args, **kwargs)
        cls._subQueryEngine = cls._session.declare_subscriber("@cmpe491/subscribers", handler=cls.__sendSubscribers)
        cls._pubQueryEngine = cls._session.declare_subscriber("@cmpe491/publishers", handler=cls.__sendPublishers)
        print(cls._session.info.zid())


    @classmethod
    def declare_subscriber(cls, *args, **kwargs):
        try:
            key_expr = ""
            if (len(args) > 0):
                cls._subs.append(args[0])
            else:
                cls._subs.append(kwargs["key_expr"])
            currentState = kwargs.pop("currentState", None)
            nextState = kwargs.pop("nextState", None)
            if currentState is not None:
                cls._transitions.append(("sub", cls._subs[-1], currentState, nextState))
            return cls._session.declare_subscriber(*args, **{kw: kwargs[kw] for kw in kwargs if (kw != "nextState")})
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