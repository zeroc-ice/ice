To run the demo, first start the IceGrid service:

$ icegridnode --Ice.Config=config.grid

Note that for debug Windows builds you will need to use icegridnoded
rather than icegridnode as the executable name.

In a separate window:

$ icegridadmin --Ice.Config=config.grid -e \
    "application add 'application.xml'"
$ java -jar build/libs/client.jar

This will deploy the application described in the file
"application.xml" and start the client.

Messages will be displayed in the IceGrid service window.

You can also use the descriptors in the following files to deploy the
application:

- application_with_template.xml: These descriptors demonstrate the use
  of templates for the server definition. Templates make it easy to
  deploy multiple instances of the same server.

- application_with_replication.xml: These descriptors demonstrate the
  use of replication to balance the load of the application over
  several servers.

If you have already deployed the application, you can update it to try
a new set of descriptors, for example:

$ icegridadmin --Ice.Config=config.grid -e \
    "application update 'application_with_template.xml'"

When using the descriptors from 'application_with_template.xml' or
'application_with_replication.xml', you can easily deploy more servers
based on the `SimpleServer' template. For example, you can use the
following command to deploy a new server:

$ icegridadmin --Ice.Config=config.grid -e \
    "server template instantiate Simple node1 SimpleServer index=4"
