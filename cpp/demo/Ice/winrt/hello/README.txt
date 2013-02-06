This demo illustrates how to invoke ordinary (twoway) operations,
as well as how to make oneway, datagram, secure, and batched
invocations.

First follow the instructions from the C++ Ice hello demo README to
start the server. Since the WinRT SSL transport doesn't support client
side SSL authentication, you also need to edit the hello server
config.server configuration file to uncomment the IceSSL.VerifyPeer=0
property. If you run the client from a remote device such as the
Surface, you will also need to regenerate the server certificate to
ensure the certificate common name is set to the IP address of the
server. To regenerate the certificate, you can run the
makewinrtcerts.py Python script from the certs directory at the top of
this distribution. For example:

  > cd certs
  > makewinrtcerts.py 192.168.1.53

This will regenerate a server certificate with a common name set to
192.168.1.53. This can either be set to an IP address or DNS name, the
only requirement is that it matches the value that will be used by the
chat client to connect to the Glacier2 router.

Build and deploy the demo using "Deploy hello" from the "Build" menu.

Start the "Ice Hello Demo" application from the Windows Start screen
or right click on the project and select Debug -> Start new instance.

To test timeouts you can use the timeout slider to set a timeout in
the client proxy and the delay slider to set a delayed response in
the server to cause a timeout.

You will notice that two "Hello World!" messages will be printed by
the server in this case. This is because the sayHello method is marked
as idempotent in the Slice, meaning that Ice does not need to follow
the at-most-once retry semantics.

See the manual for more information about retry behavior.
