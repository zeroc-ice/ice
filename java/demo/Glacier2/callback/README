This example demonstrates the use of a Glacier2 router with a set of
well known server objects. In this demo no Glacier2 session manager is
used.

To run the demo:

Start the server:

$ java -jar build/libs/server.jar

In a separate window, start the Glacier2 router:

$ glacier2router --Ice.Config=config.glacier2

In a separate window, start the client:

$ java -jar build/libs/client.jar

If you plan to run this demo using clients on different hosts than the
router, you must first modify the configuration. You need to change
the Glacier2.Client.Endpoints property in config.glacier2 and the
Ice.Default.Router properties in config.client. In all cases you must
replace the "-h 127.0.0.1" parameter with the actual external address
of the machine on which glacier2router is running.
