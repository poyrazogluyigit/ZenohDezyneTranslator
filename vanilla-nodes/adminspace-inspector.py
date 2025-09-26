import zenoh

def main():

    with zenoh.open(zenoh.Config()) as session:

        replies = session.get(
            "@/8af63933bde1e59145683590936eff1/peer/**",
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


        replies = session.get(
            "@/afe73cc2336803f175ac916860a258de/router/**",
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


if __name__ == "__main__":
    main()