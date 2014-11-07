To run the demo, start the 3 servers:

$ server.exe --Ice.Config=config.server1
$ server.exe --Ice.Config=config.server2
$ server.exe --Ice.Config=config.server3

In a separate window:

$ java -jar build/libs/client.jar

The client invokes the number of specified iterations with a given
delay on a well-known proxy configured to use per-request load
balancing. Each invocation on the proxy queries the Ice locator
implemented by the IceDiscovery plugin.

While the client is running and invoking on the server, you can try to
stop some of the servers. As long as one server is still running, the
client will continue to work.
