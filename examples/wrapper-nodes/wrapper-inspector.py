import time
import zenoh
import json
import regex as re

def prettyPrint(topics):
    for topic in topics:
        print(topic + ":")
        print("\tSubscribers: ")
        for subList in topics[topic]["subscriber"]:
            for subType in subList:
                if subType == "routers":
                    continue
                print(f"\t\t{subType}:")
                for sub in subList[subType]:
                    print(f"\t\t\t- {sub}")
        print("\tPublishers: ")
        for pubList in topics[topic]["publisher"]:
            for pubType in pubList:
                if pubType == "routers":
                    continue
                print(f"\t\t{pubType}:")
                for pub in pubList[pubType]:
                    print(f"\t\t\t- {pub}")


def callback(sample):
    print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    # match = re.findall(subPattern, str(reply.ok.key_expr))
    # if match:
    #     topic = match[0]
    #     if topic not in topics:
    #          topics[topic] = {"subscriber" : [], "publisher": []}
    #     topics[topic]["subscriber"].append(json.loads(reply.ok.payload.to_string()))



def main():
        try:
            subPattern = r".*\/subscriber\/(.*)"
            pubPattern = r".*\/publisher\/(.*)"

            conf = zenoh.Config()
            session = zenoh.open(conf)

            topics = dict()

            sub1 = session.declare_subscriber("@cmpe491/subscribers/reply", callback)
            sub2 = session.declare_subscriber("@cmpe491/publishers/reply", callback)

            subscriberReplies = session.put(
                "@cmpe491/subscribers",
                " "
            )


            publisherReplies = session.put(
                "@cmpe491/publishers",
                " "
            )

            time.sleep(5)
            # prettyPrint(topics)
        except Exception as e:
            print(e)

    
if __name__ == "__main__":
    main()