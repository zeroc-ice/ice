This demo demonstrates the use of the session activation mode where
the server is activated on demand once it is allocated by the client
and deactivated when the client releases the server.

To run the demo, first start the IceGrid service:

$ icegridnode --Ice.Config=config.grid

In a separate window:

$ icegridadmin --Ice.Config=config.grid -e \
    "application add 'application.xml'"
$ client

This will deploy the application described in the file
"application.xml" and start the client.

Messages will be displayed in the IceGrid service window.


Vista Note
----------

On Vista-derived operating systems, the IceGrid node may emit the
following warning:

  warning: Unable to lookup the performance counter name:
  Unable to connect to the specified computer or the computer is
  offline. This usually occurs when you do not have sufficient
  privileges.

This issue can be resolved by granting appropriate permissions to the
node's user account. Please refer to the Troubleshooting section of
Appendix H in the Ice manual for more information.
