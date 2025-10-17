import time
import zenoh

def getPeerInfo(session):
        replies = session.get(
            "@/**",
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

    # pub = session.declare_publisher(f"demo/vanilla/2")

    sub = session.declare_subscriber(
        "demo/vanilla/ses2/1",
        lambda sample: print(f"Received: {sample.payload.to_string()}"))
    
    print(f"Node 3 for 2nd session running...")

    # getPeerInfo(session)

    try: 
        while True:
            msg = "Sent from 2"
            # pub.put(msg)
            time.sleep(2)
    
    except KeyboardInterrupt:
        pass
    finally:
        sub.undeclare()
        # pub.undeclare()
        session.close()

if __name__ == "__main__":
    main()