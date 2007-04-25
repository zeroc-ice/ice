The server-side portion of the MFC hello demo. Note that this demo can
be used with the MFC client or any of the other hello clients (for
example, the one in demo/Ice/hello).

To run the demo, first start the server:

$ server

Then start the client:

$ cd ../client
$ client

To test timeouts, enable the "Timeout" check box to set a timeout on
the client proxy and the "Delay" check box to force the server to
delay its response long enough to trigger the timeout in the client.
You will notice that the server prints two "Hello World!" messages.
These occur because the Slice operation sayHello is marked as
idempotent, which allows the Ice run time to retry the operation in
the case of a timeout failure. See the Ice manual for more information
about retry behavior.
