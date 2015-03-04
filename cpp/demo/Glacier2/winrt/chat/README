This demo demonstrates the use of Glacier2 session helpers and how to
create a graphical client for the simple chat server.

First follow the instructions from the C++ Glacier2 chat demo README
to start the server and the Glacier2 router. Since the WinRT SSL
transport doesn't support client-side SSL authentication, you also
need to edit the Glacier2 router configuration file config.glacier to
uncomment the IceSSL.VerifyPeer=0 property. If you run the client from
a remote device such as the Surface, you will also need to regenerate
the server certificate to ensure the certificate common name is set to
the IP address of the server. To regenerate the certificate, you can
run the makewinrtcerts.py Python script from the certs directory at
the top of this distribution. For example:

  > cd certs
  > makewinrtcerts.py 192.168.1.53

This will regenerate a server certificate with a common name set to
192.168.1.53. This can either be set to an IP address or DNS name, the
only requirement is that it matches the value that will be used by the
chat client to connect to the Glacier2 router.

Build and deploy the demo using "Deploy chat" from the "Build" menu.

Start the "Glacier2 Chat Demo" application from the Windows Start
screen or right click on the project and select Debug -> Start new
instance.
