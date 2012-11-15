This demo illustrates how to invoke ordinary (twoway) operations,
as well as how to make oneway, datagram, secure, and batched 
invocations.

First follow the instructions in the C++ Ice hello demo README 
to start the server.

To start the client first build the demo using "Build hello" command
in "Build" menu.

Deploy the demo using "Deploy hello" command in "Build" menu.

Then start the "Ice Hello Demo" application from Windows Start menu.

To test timeouts you can use the timeout slider to set a timeout in the
client proxy. And the delay slider to set a delayed response in the server
to cause a timeout.

You will notice that two "Hello World!" messages will be printed by
the server in this case. This is because the sayHello method is marked
as idempotent in the slice, meaning that Ice does not need to follow
the at-most-once retry semantics. 

See the manual for more information about retry behavior.
