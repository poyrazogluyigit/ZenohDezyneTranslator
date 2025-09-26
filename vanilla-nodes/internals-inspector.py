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

def main():
        
        subPattern = r".*\/subscriber\/(.*)"
        pubPattern = r".*\/publisher\/(.*)"
        
        conf = zenoh.Config()
        session = zenoh.open(conf)

        topics = dict()

        subscriberReplies = session.get(
            "@/*/router/subscriber/**",
            target=zenoh.QueryTarget.ALL,
            payload=None,
            timeout=10.0
        )
        for reply in subscriberReplies:
            try:
                match = re.findall(subPattern, str(reply.ok.key_expr))
                if match:
                    topic = match[0]
                    if topic not in topics:
                         topics[topic] = {"subscriber" : [], "publisher": []}
                    topics[topic]["subscriber"].append(json.loads(reply.ok.payload.to_string()))
            except:
                print(f">> Received (ERROR: '{reply.err.payload.to_string()}')")

        publisherReplies = session.get(
            "@/*/router/publisher/**",
            target=zenoh.QueryTarget.ALL,
            payload=None,
            timeout=10.0
        )
        for reply in publisherReplies:
            try:
                match = re.findall(pubPattern, str(reply.ok.key_expr))
                if match:
                    topic = match[0]
                    if topic not in topics:
                         topics[topic] = {"subscriber" : [], "publisher": []}
                    topics[topic]["publisher"].append(json.loads(reply.ok.payload.to_string()))
            except:
                print(f">> Received (ERROR: '{reply.err.payload.to_string()}')")

        prettyPrint(topics)

    
if __name__ == "__main__":
    main()