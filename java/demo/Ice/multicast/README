This demo illustrates how to use UDP multicast to implement a
discovery service.

To run the demo, first start multiple instances of the server:

$ java -jar build/libs/server.jar

In a separate window, start a client:

$ java -jar build/libs/client.jar

On startup the client will send out a broadcast message containing a
callback proxy and waits for a response. Any servers that are up and 
listening on the broadcast port will then contact the client using the
callback saying they are available. The client selects a server and
proceeds.

By default this demo uses IPv4. If you want to use IPv6 UDP multicast
instead, uncomment the alternative IPv6 configuration in config.client
and config.server.
