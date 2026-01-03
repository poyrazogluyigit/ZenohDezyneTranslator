import time
import zenoh
import json
import regex as re
from pathlib import Path
from collections import defaultdict


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

subscriberInfo = dict()
publisherInfo = dict()
transitionInfo = dict()
enumInfo = dict()
boolInfo = dict()
subintInfo = dict()

def generate_code():
    target_dir = Path.cwd() / "generate"
    target_dir.mkdir(exist_ok=True)
    for key in subscriberInfo.keys():
        with open(target_dir / (key + ".dzn"), "w") as target_file:
            target_file.write(f"interface I{key}\n")
            target_file.write("{\n")
            for topic in subscriberInfo[key].split(","):
                if topic != "":
                    target_file.write(f"in void {topic.replace("/", "_")}();\n")
            for topic in publisherInfo[key].split(","):
                if topic != "":
                    target_file.write(f"out void {topic.replace("/", "_")}();\n")
            enums = defaultdict(list)
            for enumName in enumInfo[key]:
                enumPrefix, enumSuffix = enumName.split(".")
                enums[enumPrefix].append(enumSuffix)
            for enum in enums:
                target_file.write(f"enum {enum} {{")
                target_file.write(", ".join(enums[enum]))
                target_file.write(f"}};\n")
            # for boolVar in boolInfo:
            #     # TODO does it need initial value?
            #     target_file.write(f"bool {boolVar};\n")
            for subint in subintInfo[key].split(";"):
                if subint != "":
                    target_file.write(f"subint {subint};\n")
            if transitionInfo[key] != "":
                target_file.write("behavior\n")
                target_file.write("{\n")
                for transition in [t.strip("\n") for t in transitionInfo[key].split("-") if t]:
                    # FIXME tam olarak 3 e acilmazsa patlar bu
                    condition, currentState, nextState = transition.split(",")
                    # parse condition
                    conditionList = condition.split(":")
                    target_file.write(f"on {conditionList[0].replace("/", "_")}: ")
                    target_file.write("{\n")
                    # target_file.write(f"<pseudo>{currentState}->{nextState}\n")
                    target_file.write("<pseudo>\n")
                    target_file.write(f"[{currentState}")
                    if len(conditionList) > 1:
                        target_file.write(f" && {conditionList[1]}")
                    target_file.write(f"] {nextState}\n")
                    target_file.write("}\n")
                target_file.write("}\n")
            target_file.write("}")

def sub_callback(sample):
    # print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    payload = sample.payload.to_string()
    zid, restWithPts = payload.split("{", 1)
    rest = restWithPts.rsplit("}", 1)[0]
    subscriberInfo[zid] = rest

def pub_callback(sample):
    # print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    payload = sample.payload.to_string()
    zid, restWithPts = payload.split("{", 1)
    rest = restWithPts.rsplit("}", 1)[0]
    publisherInfo[zid] = rest

def trs_callback(sample):
    # print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    payload = sample.payload.to_string()
    zid, restWithPts = payload.split("{", 1)
    rest = restWithPts.rsplit("}", 1)[0]
    transitionInfo[zid] = rest

def enum_callback(sample):
    print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    payload = sample.payload.to_string()
    zid, enumStrWithPts = payload.split("{")
    enumStr, _ = enumStrWithPts.split("}")
    enumInfo[zid] = [i for i in enumStr.split(";") if i]

def bool_callback(sample):
    print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    # payload = sample.payload.to_string()
    # zid, restWithPts = payload.split("{", 1)
    # rest = restWithPts.rsplit("}", 1)[0]
    # transitionInfo[zid] = rest

def subint_callback(sample):
    print(f">> Received ('{sample.key_expr}': '{sample.payload.to_string()}')")
    payload = sample.payload.to_string()
    zid, subintsWithPts = payload.split("{", 1)
    subints, _ = subintsWithPts.rsplit("}", 1)
    subintInfo[zid] = subints


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

            sub1 = session.declare_subscriber("@cmpe491/subscribers/reply", sub_callback)
            sub2 = session.declare_subscriber("@cmpe491/publishers/reply", pub_callback)
            sub3 = session.declare_subscriber("@cmpe491/transitions/reply", trs_callback)
            sub4 = session.declare_subscriber("@cmpe491/enums/reply", enum_callback)
            sub5 = session.declare_subscriber("@cmpe491/bools/reply", bool_callback)
            sub6 = session.declare_subscriber("@cmpe491/subints/reply", subint_callback)

            session.put(
                "@cmpe491/subscribers",
                " "
            )


            session.put(
                "@cmpe491/publishers",
                " "
            )

            session.put(
                "@cmpe491/transitions",
                " "
            )

            session.put(
                "@cmpe491/enums",
                " "
            )


            session.put(
                "@cmpe491/bools",
                " "
            )

            session.put(
                "@cmpe491/subints",
                " "
            )



            time.sleep(2)
            print(subscriberInfo)
            print(publisherInfo)
            print(transitionInfo)
            generate_code()
            # prettyPrint(topics)
        except Exception as e:
            print(e)

    
if __name__ == "__main__":
    main()