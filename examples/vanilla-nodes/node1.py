import time
import zenoh

def getPeerInfo(session):
        replies = session.get(
            "@/*/peer/**",
            target=zenoh.QueryTarget.ALL,
            payload=None,
            timeout=10.0
        )
        for reply in replies:
            try:
                print(
                    f">> Received ('{reply.ok.key_expr}': '{reply.ok.payload.to_string()}')"
                )
            except:
                print(f">> Received (ERROR: '{reply.err.payload.to_string()}')")

def main():

    conf = zenoh.Config()
    # conf.insert_json5("connect/endpoints", '["tcp/127.0.0.1:0"]')

    session = zenoh.open(conf)
    session2 = zenoh.open(conf)

    pub = session.declare_publisher(f"demo/vanilla/1")
    pub2 = session.declare_publisher(f"demo/vanilla/ses2/1")

    sub = session.declare_subscriber(
        "demo/vanilla/2",
        lambda sample: print(f"Received: {sample.payload.to_string()}"))
    
    print(f"Node 1 running...")

    getPeerInfo(session)


    try: 
        while True:
            msg = "Sent from 1"
            pub.put(msg)
            pub2.put(msg)
            time.sleep(2)
    
    except KeyboardInterrupt:
        pass
    finally:
        sub.undeclare()
        pub.undeclare()
        session.close()

if __name__ == "__main__":
    main()