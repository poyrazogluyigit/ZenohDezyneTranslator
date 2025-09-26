import time
import zenoh
from utils.wrapper import Wrapper

def main():

    conf = zenoh.Config()
    # conf.insert_json5("connect/endpoints", '["tcp/127.0.0.1:0"]')

    Wrapper.open(conf)

    pub = Wrapper.declare_publisher("demo/wrapper/2")

    sub = Wrapper.declare_subscriber(
        "demo/wrapper/1",
        lambda sample: print(f"Received: {sample.payload.to_string()}"))
    
    print(f"Node 2 running...")

    # getPeerInfo(session)

    try: 
        while True:
            msg = "Sent from 2"
            pub.put(msg)
            time.sleep(2)
    
    except KeyboardInterrupt:
        pass
    finally:
        sub.undeclare()
        pub.undeclare()

if __name__ == "__main__":
    main()