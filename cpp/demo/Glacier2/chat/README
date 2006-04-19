This example demonstrates the use of a Glacier2 session to create a
very simple chat server.

To run the demo, first start the chat server:

$ server

In a separate window, start the Glacier2 router:

$ glacier2router --Ice.Config=config.glacier2

In a separate window, start the client:

$ client

If you plan to run this demo using clients running on different
hosts than the glacier2router, it is necessary to first modify the
configuration. You need to change the Glacier2.Client.Endpoints
property in config.glacier2 and the Ice.Default.Router and
Chat.Client.Router properties in config. In all cases you must
replace the "-h 127.0.0.1" parameter with the actual external address
of the machine on which glacier2router is running.
