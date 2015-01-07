The demo shows how to make:

  - Twoway invocations. This is the default mode for any remote
    invocation. A twoway invocation is not complete until a reply has
    been received from the server.

  - Oneway invocations. A oneway invocation is complete as soon as the
    client has successfully sent the invocation. Oneway invocations
    may not return values or throw user exceptions.

  - Oneway batched invocations. See below for details on batching.

  - Datagram invocations. Datagram invocations use the udp transport,
    and as such have very minimal quality of service guarantees.
    Datagram invocations are always oneway.

  - Datagram batched invocations. See below for details on batching.

Batching, which is only possible when using oneway or datagram
invocations, combines several Ice messages into a single physical
protocol message. Batching can result in higher throughput and smaller
physical messages due to less protocol overhead. Ice does not actually
send a batched message until it is flushed.

If you enable secure invocations, oneway and twoway operations are
sent using the SSL transport. It is not possible to secure datagram
invocations.

To run the demo, first start the server:

$ server

In a separate window, start the client:

$ client

To test timeouts you can use 'T' to set a timeout on the client proxy
and 'P' to set a delayed response in the server to cause a timeout.
You will notice that two "Hello World!" messages will be printed by
the server in this case. This happens because the Slice operation
sayHello is marked as idempotent, meaning that Ice does not need to
preserve the at-most-once retry semantics. See the manual for
more information about retry behavior.
