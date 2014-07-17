This demo shows how to write a simple C# client application using Ice
for .NET Compact Framework (Compact 2013).

This demo requires a hello server. You can use the C# server located
in ..\..\hello, or you can use a server from any other language mapping.

You must use Visual Studio 2013 to build this client. Follow these
instructions to build and run the client:

1) Install Windows Embedded Compact 2013 as described below:

   http://msdn.microsoft.com/en-us/library/jj200354.aspx

2) Setup a Virtual Test Device:

   http://msdn.microsoft.com/en-us/library/jj200348.aspx.

3) Create an OS:

   http://msdn.microsoft.com/en-us/library/jj200351.aspx

   Ensure that you select "Win32 and WinForms UI Device", and ensure
   that "Application Debugging support" and "Remote Tools Support" is
   enabled.

4) Build the OS and ensure that it successfully deploys to your
   virtual device.

5) At this point it may be useful to create a test Subproject to
   ensure you can build and deploy executables:

   http://msdn.microsoft.com/en-us/library/jj200344.aspx

6) Select Subprojects>Add Existing Subproject. Navigate to
   "cs/demo/Ice/compact/hello" and select "hello.pbpxml".

6) Select Build>Build All Subprojects.

7) To run the application, go to Target>Run Programs and select
   hello.exe.

8) In the client's Hostname field, enter the IP address or host name
   of the machine on which the hello server is running. Click the
   Say Hello button to send a request to the server.

   Note that the initial request takes a little longer to complete
   while the connection is being established.
