This demo demonstrates the use of Glacier2 session helpers and how to
create a graphical client for the simple chat server.

First follow the instructions from the C++ Glacier2 chat demo README
to start the server and the Glacier2 router. Since the WinRT SSL
transport doesn't support client side SSL authentication, you also
need to edit the Glacier2 router config.glacier configuration file to
uncomment the IceSSL.VerifyPeer=0 property.

Build and deploy the demo using "Deploy chat" from the "Build" menu.

Start the "Glacier2 Chat Demo" application from the Windows Start
screen or right click on the project and select Debug -> Start new
instance.
