To run the demo, first start the IceGrid service:

$ icegridnode --Ice.Config=config.grid

Note that for debug Windows builds you will need to use icegridnoded
rather than icegridnode as the executable name.

In a separate window:

$ icegridadmin --Ice.Config=config.admin -e \
    "application add 'application.xml'"
$ client

This will deploy the application described in the file
"application.xml" and start the client.

The client makes an invocation on the "pricing" well-known object to
retrieve the preferred currencies of the server. The client sets up a
context to indicate its preferred currency to the replica group
filter. The replica group filter filters out servers that don't match.

You can execute the client multiple times to ensure it doesn't always
talk to the same server. The client should always talk to a server
that supports its preferred currency.

You can edit config.client to change or comment out the client's
preferred currency.
