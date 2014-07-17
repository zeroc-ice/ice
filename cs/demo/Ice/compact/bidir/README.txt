This demo shows how to write a simple C# server application using Ice
for .NET Compact Framework (Compact 2013).

This demo requires a bidir client. You can use the C# client located
in ..\..\bidir, or you can use a client from any other language
mapping.

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
   "cs/demo/Ice/compact/bidir" and select "bidir.pbpxml".

   You will need to build Callback.ice with slice2cs. To do this
   select Subprojects>bidir>Open Build Window

   At the prompt enter slice2cs Callback.ice. This produces
   Callback.cs which is built by the project.

7) Select Build>Build All Subprojects.

8) To run the application, go to Target>Run Programs and select
   bidir.exe.

9) Before you can run the bidir client, you need to know the IP
   address being used by the Virtual Device. When the virtual device
   is opened it will report the IP address

   Got Response from DHCP server x.x.x.x with address y.y.y.y

   The device's IP address is the y.y.y.y address.

   Edit the config.client file for your bidir client and change the
   following line:

   CallbackSender.Proxy=sender:tcp -h localhost -p 10000

   Replace "localhost" with the IP address of the Virtual Device.

10) To run the application, go to Target>Run Programs and select
    bidir.exe.
