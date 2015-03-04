To run the demo, first start the IceGrid service:

$ icegridnode --Ice.Config=config.grid

This demo contains two application descriptor files. The first
descriptor, application-single.xml, contains a single server and
object. This object is allocated by the client using the
allocateObjectById operation. Only one client can access this object
at a time. All other clients will hang until the object is released.
Use the following command to deploy this descriptor:

$ icegridadmin --Ice.Config=config.grid -e \
    "application add 'application-single.xml'"

The second descriptor, application-multiple.xml, contains two servers,
each with an object. The clients retrieve these objects using
allocateObjectByType. Since there are two objects available, two
clients can get access simultaneously. Additional clients will hang
until one of the clients with an allocated object releases it. Use the
following command to deploy this descriptor:

$ icegridadmin --Ice.Config=config.grid -e \
    "application add 'application-multiple.xml'"

To run the client type:

$ client

If you have already deployed the application, you can update it to try
a different set of descriptors, for example:

$ icegridadmin --Ice.Config=config.grid -e \
    "application update 'application-multiple.xml'"

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
