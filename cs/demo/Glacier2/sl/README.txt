This demo demonstrates the use of a Glacier2 session helpers and
Silverlight to create a graphical client for the simple chat server.

First follow the instructions in the C++ Glacier2 chat demo README
to start the server and the Glacier2 router.

1) You must use the server from the C++ demo directory, before
   starting the Glacier2 router, you must edit the client endpoints to
   use a port in the range allowed by Silverlight.

   Edit the config.glacier2 file from the C++ chat demo directory and
   update the property Glacier2.Client.Endpoints as follow:

      Glacier2.Client.Endpoints=tcp -p 4502 -h 127.0.0.1
   
2) Follow the instructions in the C++ demo README to start the server
   and the Glacier2 router.

3) In a command window, start the policy server.

   > cd <Ice installation directory>\bin
   > policyserver 127.0.0.1 ..\config\PolicyResponse.xml

4) From Visual Studio open the `chat.Web' project and start the chat
   Silverlight application using the "Debug > Start new instance"
   command.
