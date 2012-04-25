This demo shows how to use bidirectional connections for callbacks.
This is typically used if the server cannot open a connection to the
client to send callbacks, for example, because firewalls block
incoming connections to the client.

1) You can use a server from any Ice language mapping, befofe starting
   the server, you must edit the server endpoints to use a port in the
   range allowed by Silverlight.

   Edit the config.server file in the server bidir demo directory and
   update the property Callback.Server.Endpoints as follow:

       Callback.Server.Endpoints=tcp -p 4502
   
2) Start the bidir server following the instructions in the demo
   README file.

3) In a command window, start the policy server.

   > cd <Ice installation directory>\bin
   > policyserver 127.0.0.1 ..\config\PolicyResponse.xml

4) From Visual Studio open the `bidir.Web' and start the bidir
   Silverlight application using the "Debug > Start new instance"
   command.

==========================================================================
Running the demo with .NET Compact Framework bidir server
==========================================================================

1) Start the bidir server following the instructions in the
   demo\Ice\compact\bidir\README.txt file, before clicking the `Start
   Server' button update the port field to 4502

2) In the bidir demo "Properties > Debug" select Out-of-browser
   application, and in the combo box select bidir.

3) Start the bidir client using the "Debug > Start new instance"
   command, set the host field to the ip address used by the emulator,
   and click Run.
