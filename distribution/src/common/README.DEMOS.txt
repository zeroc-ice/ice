Introduction
------------

This folder contains sample programs for C++, Java, Android, C#, VB,
Ruby, Python and PHP. To build and run the demos, follow the language-
specific instructions below. These build instructions assume that you
have configured your system according to the instructions in the
release notes.


Table of Contents
-----------------

  1.  Building and running the C++ demos (Visual Studio 2010SP1 or 2012)
  2.  Building and running the C++ demos (Visual C++ 2010SP1 or 2012 Express)
  3.  Building and running the C++ WinRT demos (Visual Studio 2012 & Windows 8)
  4.  Building and running the .NET demos
      - Building the C# demos
      - Building the Visual Basic demos
      - Running .NET demos
      - SSL Notes for the .NET demos
  5.  Building and running the .NET Compact Framework demo
  6.  Building and running the Silverlight demos
  7.  Building and running the Java demos
  8.  Building and running the Android demos
  9.  Running the Python demos
  10. Running the Ruby demos
  11. Building and running the PHP demos


======================================================================
1. Building and running the C++ demos (Visual Studio 2010SP1 or 2012)
======================================================================

The C++ demos are in the demo directory.

Note that the Visual Studio project files require the Ice Visual
Studio Add-In, installed as part of the installation of the Ice binary
distribution.

To build the C++ demos, start Visual Studio and open the solution
demo\demo.sln, if you are using Visual Studio 2012 the demo projects
will be converted first to Visual Studio 2012.

Select your target configuration: Debug or Release, Win32 or x64 (on
supported x64 platforms). Right click on the desired demo in the
Solution Explorer window and select "Build".

To run a demo, open a command window and change to the desired demo
subdirectory. Review the README file if one is present. Type 'server'
to start the server. In another command window, type 'client' to start
the client.

This distribution also includes a few demos for integrating Ice with
Oracle. Open the solution demo\demo-oracle.sln to build the corresponding 
projects (TODO what VS version).


======================================================================
2. Building and running the C++ demos (Visual C++ 20010SP1 or 2012
   Express)
======================================================================

The C++ demos are in the demo directory.

Visual C++ Express does not support the Ice Visual Studio Add-in,
therefore you must use NMAKE to build the C++ demos.

To build the C++ demos, open a "Visual Studio Command Prompt" window.
If you installed Ice in a non-default location, set ICE_HOME as shown
below:

  > set ICE_HOME=<Ice installation root directory>

Change to the demo directory and run the following commands to build
the demos:

  > set CPP_COMPILER=VC100_EXPRESS   # (For Visual C++ 2010SP1)
  > set CPP_COMPILER=VC110_EXPRESS  # (For Visual C++ 2012)
  > nmake /f Makefile.mak

Note that the Ice demos that require MFC (demo\Ice\MFC and
demo\IcePatch2\MFC) will not compile because the Express edition does
not include MFC support.

To run a demo, change to the desired demo subdirectory and review the
README file if one is present. Type 'server' to start the server. In
another command window, type 'client' to start the client.

======================================================================
3. Building and running the C++ WinRT demos (Visual Studio 2012)
======================================================================

The C++ demos are in the demo directory.

Note that the Visual Studio project files require the Ice Visual
Studio Add-In, installed as part of the installation of the Ice binary
distribution.

WinRT requires Windows 8 with Visual Studio 2012.

To build the C++ WinRT demos, start Visual Studio 2012 and open the 
solution demo\demo-winrt.sln.

Select your target configuration: Debug or Release, Win32 or x64 (on
supported x64 platforms). Right click on the desired demo in the
Solution Explorer window and select "Build".

To run a demo review the README.txt file in the demo project.


======================================================================
4. Building and running the .NET demos
======================================================================

Note that the Visual Studio project files require the Ice Visual
Studio Add-In, installed as part of the installation of the Ice binary
distribution.


Building the C# demos
---------------------

The C# demos are in the democs directory.

To build a C# demo, start Visual Studio and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".


Building the Visual Basic demos
-------------------------------

The Visual Basic demos are in the demovb directory.

To build a Visual Basic demo, start Visual Studio and open the
solution demovb\demo.sln. Right click on the desired demo in the
Solution Explorer window and select "Build".


Running .NET demos
------------------

To run a demo, open a command window, change to the desired demo
subdirectory and review the README file if one is present. Type
'server.exe' to start the server. In another command window, type
'client.exe' to start the client.


SSL Notes for the .NET demos
----------------------------

In order to use SSL with the sample programs, an SSL certificate must
be installed on your system. The configuration files handle this for
you, but you will be presented with a confirmation dialog the first
time you run a sample program that uses SSL.

Once you are finished with the sample programs, follow these steps to
remove the certificate:

1) Start Internet Explorer.

2) Select Internet Options from the Tools menu.

3) Select the Content tab and click the "Certificates" button.

4) Select the Trusted Root Certification Authorities tab.

5) Select the entry for "ZeroC Test CA", click the Remove button, and
   confirm that you want to remove this certificate.


======================================================================
5. Building and running the .NET Compact Framework demo (Visual Studio 2008)
======================================================================

Note that the Visual Studio project file requires the Ice Visual
Studio Add-In, installed as part of the installation of the Ice binary
distribution.

The demo for the .NET Compact Framework is located in the
democs\Ice\compact subdirectory.

To build the demo, start Visual Studio and open the solution
democs\democf.sln. Right click on the demo in the Solution Explorer
window and select "Build".

Review the README.txt file in the democs\Ice\compact subdirectory for
instructions on starting a device emulator and deploying the program.


======================================================================
6. Building and running the Silverlight demos (Visual Studio 2010SP1)
======================================================================

Note that the Visual Studio project file requires the Ice Visual
Studio Add-In, installed as part of the installation of the Ice binary
distribution.

The demos for Silverlight are located in democs\Ice\sl and 
democs\Glacier2\sl subidrectories.

To build the demo, start Visual Studio 2010 and open the solution
democs\demosl.sln. Right click on the demo in the Solution Explorer
window and select "Build".

To run a demo review the README.txt file in the demo project.


======================================================================
7. Building and running the Java demos
======================================================================

The Java demos are in the demoj directory.

To build the Java demos, you need J2SE SDK 1.6.0 or later, and Ant
1.7.0 or later. They can be downloaded from:

  http://www.oracle.com/technetwork/java/javase/overview/index.html
  http://ant.apache.org/bindownload.cgi

The ant bin directory must be added to your PATH, and the following
environment variables need to be defined:

  > set JAVA_HOME=<Java SDK installation root directory>
  > set PATH=<Ice installation root directory>\bin;%PATH%

If you installed Ice in a non-default location, set ICE_HOME as shown
below:

  > set ICE_HOME=<Ice installation root directory>

You can build all of the demos by running 'ant' in the demoj
directory, or you can build individual demos by changing to the
desired subdirectory and then running 'ant'.

Before running the demos you must modify your CLASSPATH as follows:

  > set CLASSPATH=classes;%CLASSPATH%

Your CLASSPATH should also include Ice.jar and Freeze.jar, as per 
the Release Notes instructions. 

To run a demo, open a command window, change to the desired demo
directory, and enter the following command to run the server:

  > java Server

In a separate command window, enter the following command to run the
client:

  > java Client

Some demo directories contain README files if additional requirements
are necessary.


======================================================================
8. Building and running the Android demos
======================================================================

Several sample Android projects are provided in the demoj/android
subdirectory. You must use Eclipse and the Slice2Java plug-in to build
these projects. The ZeroC web site describes how to install the
Eclipse plug-in:

  http://www.zeroc.com/eclipse.html

In Eclipse, you can open a sample project by choosing File->Import...;
in the "General" group, select "Existing Project into Workspace", then
open one of the subdirectories of demoj/android.

The sample projects are configured to locate the Ice run time JAR file
(Ice.jar) via the ICE_HOME classpath variable, as described in the Ice
manual:

  http://doc.zeroc.com/display/Ice/Eclipse+Plug-in

If you installed Ice.jar in a different location, you will need to add
it as an external JAR file in each sample project:

 1. Open the project's properties and select Java Build Path
 2. Click on the Libraries tab
 3. Click Add External JARs... and navigate to Ice.jar
 4. Click OK to save your settings

After successfully building an Android project, deploy it onto a
suitable emulator or device and review the README file in the project
subdirectory for further instructions.


======================================================================
9. Running the Python demos
======================================================================

The Python demos are in the demopy directory.

You need Python 2.6.6 to run the demos. A binary installer for Python
can be downloaded from:

  http://www.python.org/download

To run a demo, open a command window, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate command window, type 'python Client.py' to run the client.


======================================================================
10. Running the Ruby demos
======================================================================

The Ruby demos are in the demorb directory.

You need Ruby 1.9.3 to run the demos. A binary installer for Ruby
can be downloaded from:

  http://rubyinstaller.org/downloads/

Since Ice for Ruby does not support server-side activities, only
clients are provided in the demos. In order to run the demos you must
use the corresponding C++ server.

For example, to run the hello application in demo\Ice\hello, we begin
by starting the C++ server:

  > cd <Ice demo root directory>\demo\Ice\hello
  > server

Then in a separate command window, start the Ruby client:

  > cd <Ice demo root directory>\demorb\Ice\hello
  > ruby Client.rb


======================================================================
11. Building and running the PHP demos
======================================================================

PHP demos are provided in the demophp directory.

You will need PHP 5.3.6 to run the demos. It can be downloaded from:

  http://www.php.net/downloads.php

Building the demos requires Microsoft NMAKE. If you do not already
have NMAKE installed, you can download it as part of the Windows
Platform SDK:

  http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC&displaylang=en

Open a command window. If you installed Ice in a non-default location,
set ICE_HOME as shown below:

  > set ICE_HOME=<Ice installation root directory>

Change to the demophp directory and run the following command to build
the demos:

  > nmake /f Makefile.mak

The examples in demophp\Ice\hello and demophp\Glacier2\hello
demonstrate using the Ice extension for PHP in a dynamic Web page,
whereas the example in demophp\Ice\value requires PHP's command-line
interpreter. A README file is provided in each of the example
directories.
