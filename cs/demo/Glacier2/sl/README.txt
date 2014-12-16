This example demonstrates the use of the Glacier2 session helper
classes in a Silverlight client for the simple chat server.

1) You must use the server from the C++ demo directory. Before
   starting the Glacier2 router, you must modify the router's
   client endpoints to use a port in the range allowed by Silverlight.

   Edit the config.glacier2 file in the C++ chat demo directory and
   update the property Glacier2.Client.Endpoints as follows:

     Glacier2.Client.Endpoints=tcp -p 4502 -h 127.0.0.1

2) Follow the instructions in the C++ demo README to start the chat
   server and the Glacier2 router.

3) In a command window, change to this demo directory and start the
   policy server.

   > <Ice installation directory>\bin\policyserver 127.0.0.1 clientaccesspolicy.xml

4) Start the Silverlight client using the "Debug > Start new instance"
   command.


==========================================================================
Using a web server as a policy server
==========================================================================

If you do not want to deploy a policy server, you can use a web server
instead.

IIS instructions:

1) Copy clientaccesspolicy.xml to your web server document root
   directory:

   > xcopy clientaccesspolicy.xml C:\inetpub\wwwroot\

2) Verify that the policy file is accessible at the following URL:

   http://localhost/clientaccesspolicy.xml

3) In Coordinator.cs, uncomment the line that sets the policy
   protocol:

   //initData.properties.setProperty("Ice.ClientAccessPolicyProtocol", "Http");

4) Rebuild the demo. You must rebuild both chat and chat.Web projects.

5) Copy required files. In a command window, execute the following
   commands:

   > mkdir C:\inetpub\wwwroot\chat
   > xcopy chat.Web\index.html C:\inetpub\wwwroot\chat
   > xcopy chat.Web\Silverlight.js C:\inetpub\wwwroot\chat
   > xcopy chat.Web\ClientBin C:\inetpub\wwwroot\chat\ClientBin /s /i

6) The chat client should now be accessible at the following URL:

   http://localhost/chat/index.html


Notes:

* The web server should serve the policy file on standard port 80.

* If you do not have IIS installed on Windows 7, follow the instructions
  on this page:

  http://technet.microsoft.com/en-us/library/cc731911.aspx
