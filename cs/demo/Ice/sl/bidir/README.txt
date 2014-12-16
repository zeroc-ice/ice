This demo shows how to use bidirectional connections for callbacks.

This feature is typically used if the server cannot open a connection
to the client to send callbacks, for example, because firewalls block
incoming connections to the client. In the case of Silverlight, the
Ice run time does not support server-side functionality, but you can
use a bidirectional connection to receive callbacks.

1) You can use a bidir server from any Ice language mapping. Before
   starting the server, you must edit the server's endpoints to use a
   port in the range allowed by Silverlight.

   Edit the config.server file in the server's demo directory and
   update the property Callback.Server.Endpoints as follows:

     Callback.Server.Endpoints=tcp -p 4502

2) Start the bidir server according to the instructions in the demo
   README file.

3) In a command window, change to this demo directory and start the
   policy server:

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

3) In MainPage.xaml.cs, uncomment the line that sets the policy
   protocol:

   //initData.properties.setProperty("Ice.ClientAccessPolicyProtocol", "Http");

4) Rebuild the demo. You must rebuild both bidir and bidir.Web
   projects.

5) Copy required files. In a command window, execute the following
   commands:

   > mkdir C:\inetpub\wwwroot\bidir
   > xcopy bidir.Web\index.html C:\inetpub\wwwroot\bidir
   > xcopy bidir.Web\Silverlight.js C:\inetpub\wwwroot\bidir
   > xcopy bidir.Web\ClientBin C:\inetpub\wwwroot\bidir\ClientBin /s /i

6) The bidir client should now be accessible at the following URL:

   http://localhost/bidir/index.html


Notes:

* The web server should serve the policy file on standard port 80.

* If you do not have IIS installed on Windows 7, follow the
  instructions on this page:

  http://technet.microsoft.com/en-us/library/cc731911.aspx
