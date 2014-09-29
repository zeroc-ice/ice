This demo illustrates how to invoke ordinary (twoway) operations, as
well as how to make oneway and batched invocations.

1) You can use a server from any Ice language mapping. Before starting
   the server, you must edit the server's endpoints to use a port in
   the range allowed by Silverlight.

   Edit the config.server file in the server's demo directory and
   update the property Hello.Endpoints as follows:

     Hello.Endpoints=tcp -p 4502

2) Start the hello server according to the instructions in the demo
   README file.

3) In a command window, change to this demo directory and start the
   policy server:

   > <Ice installation directory>\bin\policyserver 127.0.0.1 clientaccesspolicy.xml

4) In Visual Studio, open the `hello.Web' project. Open the properties
   using "Project -> hello.Web Properties". Under the Web tab choose
   "Specific Page" as the "Start Action" and helloTestPage.html as the
   page.

5) Start the Silverlight client using the "Debug > Start new instance"
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

4) Rebuild the demo. You must rebuild both hello and hello.web projects.

5) Copy required files. In a command window, execute the following
   commands:

   > mkdir C:\inetpub\wwwroot\hello
   > xcopy hello.web\helloTestPage.html C:\inetpub\wwwroot\hello
   > xcopy hello.web\Silverlight.js C:\inetpub\wwwroot\hello
   > xcopy hello.Web\ClientBin C:\inetpub\wwwroot\hello\ClientBin /s /i

6) The hello client should now be accessible at the following URL:

   http://localhost/hello/helloTestPage.html


Notes:

* The web server should serve the policy file on standard port 80.

* If you do not have IIS installed on Windows 7, follow the instructions
  on this page:

  http://technet.microsoft.com/en-us/library/cc731911.aspx
