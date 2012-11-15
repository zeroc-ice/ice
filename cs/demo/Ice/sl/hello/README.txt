This demo illustrates how to invoke ordinary (twoway) operations, 
as well as how to make oneway, and batched invocations.

1) You can use a server from any Ice language mapping, before starting
   the server, you must edit the server endpoints to use a port in the
   range allowed by Silverlight.

   Edit the config.server file from the server hello demo directory
   and update the property Hello.Endpoints as follow:

      Hello.Endpoints=tcp -p 4502

2) start the hello server following the instructions in the demo
   README file.

3) In a command window, start the policy server.

   > cd <Ice installation directory>\bin
   > policyserver 127.0.0.1 ..\config\PolicyResponse.xml

4) From Visual Studio open the `hello.Web' project and start the hello
   Silverlight application using the "Debug > Start new instance"
   command.
