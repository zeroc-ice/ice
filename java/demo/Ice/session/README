This example demonstrates how to clean up per-client objects through
the use of sessions.

When the client starts, a session object is created through which all
per-client objects are created. The session object's timestamp must be
refreshed by the client on a periodic basis otherwise it will be
destroyed. Once the session is destroyed, whether by the client
destroying the session upon termination or due to a timeout, all
per-client objects are automatically cleaned up by the session object.

To run the demo, first start the server:

$ java -jar build/libs/server.jar

In a separate window, start the client:

$ java -jar build/libs/client.jar
