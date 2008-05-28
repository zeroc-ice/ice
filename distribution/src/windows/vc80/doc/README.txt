The Internet Communications Engine
----------------------------------

Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+. It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, .NET, Java,
Python, Ruby, and PHP mappings, a highly efficient protocol,
asynchronous method invocation and dispatch, dynamic transport 
plug-ins, TCP/IP and UDP/IP support, SSL-based security, a firewall
solution, and much more.

Ice is available under the terms of the GNU General Public License
(GPL) (see LICENSE file). Commercial licenses are available for
customers who wish to use Ice in proprietary products. Please contact
sales@zeroc.com for more information on licensing Ice.


About this distribution
-----------------------

This binary distribution provides all Ice run time services and 
development tools to build Ice applications:

 - in C++, using Visual Studio 2005 SP1 or Visual C++ 2005 Express
   Edition SP1
 - in .NET, using Visual Studio 2005 SP1
 - in Java, using Java 5 or Java 6
 - in Python, using Python 2.5.2

If you want to develop Ice applications in Ruby or in PHP, or with
another C++ compiler, please download the appropriate Ice binary 
distribution from the ZeroC web site at

  http://www.zeroc.com/download.html

You only need the development environment for your target programming
language to use this distribution. For example if you want to build
Ice applications in Java, you need to install a JDK, but do not need 
to install Visual Studio or Python.

This file describes how to setup Visual Studio for Ice (when building
C++ applications), and provides instructions for building and running 
the sample programs.


Supported Windows versions
--------------------------

This distribution is supported on the following Windows versions:

 - Windows XP with Service Pack 2 (x86)
 - Windows Server 2003 Standard (x86 and x64)
 - Windows Vista (x86 and x64)


Monotonic clock
---------------

Ice uses the QueryPerformanceCounter Windows API function to measure
time with a monotonic clock. If you are experiencing timing or
performance issues, there are two knowledgebase articles that may be
relevant for your system:

  http://support.microsoft.com/?id=896256
  http://support.microsoft.com/?id=895980


Running IceGrid and Glacier2 components as services
---------------------------------------------------

An appendix in the Ice manual provides information on installing and
running the IceGrid registry, IceGrid node, and Glacier2 router as
Windows services.


Using the IceGrid Administrative Console
----------------------------------------

A Java-based graphical tool for administering IceGrid applications
is included in this distribution. The Java archive file is installed
as

<Ice installation root directory>\bin\IceGridGUI.jar

With a suitable Java installation, you can execute the application
by double-clicking on this file.


Python Compatibility
--------------------

The binary distributions of Python 2.5.x for Windows available from
www.python.org are compiled with Visual C++ 7.1, and the Ice extension
for Python included in this installer is compiled with Visual C++ 8.0.
Python's developers do not support extensions built with another
compiler, however we have tested the Ice extension extensively and
have not encountered any compatibility issues.

If you distribute an Ice for Python application, you must include the
DLLs for the extension, the Ice run time, and the Visual C++ 8.0 run
time. An appendix in the Ice manual provides more information on
distributing Ice applications. The link below describes the process
of deploying the Visual C++ 8.0 run time:

  http://msdn2.microsoft.com/en-us/library/ms235291(VS.80).aspx


Setting up Visual Studio 2005 SP1 to build Ice applications in C++
------------------------------------------------------------------

Before you can use Ice in your C++ applications, you first need to
configure Visual Studio with the locations of the Ice header files,
libraries, and executables.

- In the IDE, choose Tools->Options->Projects and Solutions->VC++ Directories

- Select "Include files"

- Add <Ice installation root directory>\include

- Select "Library files"

- For x86 libraries add <Ice installation root directory>\lib

  or

  For x64 libraries add <Ice installation root directory>\lib\x64

- Select "Executable files"

- For x86 binaries add <Ice installation root directory>\bin

  or

  For x64 binaries add <Ice installation root directory>\bin\x64


Setting up Visual C++ 2005 Express Edition SP1
----------------------------------------------

Visual C++ 2005 Express and SP1 is available for download from:

  http://www.microsoft.com/express/2005/

In addition to the steps listed above for setting up Visual Studio
2005, users of Visual C++ 2005 Express Edition must also follow the
instructions at the link below for installing and configuring the
Platform SDK:

  http://msdn2.microsoft.com/en-us/express/aa700755.aspx


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio 2005 and open the solution
demo\demo.sln. Select your target configuration: Debug or Release,
Win32 or x64 (on supported x64 platforms). Right click on the desired
demo in the Solution Explorer window and select "Build".

Note that if you are using Visual C++ 2005 Express Edition the Ice demos
that require MFC (demo\Ice\MFC and demo\IcePatch2\MFC) will not compile
since only the Professional edition contains MFC support. You can
either ignore any compile errors these demos generate or just remove the
projects from the solution entirely.

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your PATH, as shown below:

set PATH=<Ice installation root directory>\bin;%PATH%

If you built an x64 configuration, use this setting instead (the
x64 directory must come first in your PATH):

set PATH=<Ice installation root directory>\bin;%PATH%
set PATH=<Ice installation root directory>\bin\x64;%PATH%

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server' to start the server. In another command
prompt, type 'client' to start the client.

This distribution includes examples for integrating Ice with databases
other than Berkeley DB. These examples are not included in the
demo.sln file mentioned above. If you want to build these demos you
must add the demo project files located in the demo\Database
subdirectories to the demo solution and build from there.

The current examples are for Oracle, and require the appropriate
Oracle development environment. Please see the individual demo README
files for more information.


Building and running the C# demos
---------------------------------

The C# demos are in the democs directory.

To build a C# demo, start Visual Studio 2005 and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".

Note that Visual Studio may present you with a "Security Warning"
dialog when you open the solution. This harmless warning is caused
by the technique that the projects use to compile Slice files. You
can safely allow the projects to load normally.

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


Building and running the Visual Basic demos
-------------------------------------------

The Visual Basic demos are in the demovb directory.

To build a Visual Basic demo, start Visual Studio 2005 and open the
solution demovb\demo.sln. Right click on the desired demo in the
Solution Explorer window and select "Build".

Note that Visual Studio may present you with a "Security Warning"
dialog when you open the solution. This harmless warning is caused
by the technique that the projects use to compile Slice files. You
can safely allow the projects to load normally.

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


SSL Notes for the .NET demos
----------------------------

In order to use SSL with the sample programs, an SSL certificate must
be installed on your system. The configuration files handle this for
you, but you will be presented with a confirmation dialog the first
time you run a sample program.

Once you are finished with the sample programs, follow these steps to
remove the certificate:

1) Start Internet Explorer.

2) Select Internet Options from the Tools menu.

3) Select the Content tab and click the "Certificates" button.

4) Select the Trusted Root Certification Authorities tab.

5) Select the entry for "ZeroC Test CA", click the Remove button, and
   confirm that you want to remove this certificate.


Building and running the Java demos
-----------------------------------

The Java demos are in the demoj directory.

To build the Java demos, you need J2SE SDK 1.5.0 or later, and Ant
1.7.0 or later. They can be downloaded from:

  http://java.sun.com/j2se/index.jsp
  http://ant.apache.org/bindownload.cgi

The ant bin directory must be added to your PATH, and the following
environment variables need to be defined:

set JAVA_HOME=<Java SDK installation root directory>
set PATH=<Ice installation root directory>\bin;%PATH%
set CLASSPATH=<Ice installation root directory>\lib\db.jar;%CLASSPATH%

You can build all of the demos by running 'ant' in the demoj
directory, or you can build individual demos by changing to the
desired subdirectory and then running 'ant'.

Before running the demos you must modify your CLASSPATH as follows:

set CLASSPATH=<Ice installation root directory>\lib\Ice.jar;classes;%CLASSPATH%

If you prefer to use Ice for Java with the Java2 mapping, modify your
CLASSPATH as shown below:

set CLASSPATH=<Ice installation root directory>\lib\java2\Ice.jar;classes;%CLASSPATH%

In addition, the JVM requires the directory containing the Berkeley DB
libraries to be listed in java.library.path, therefore the Ice bin
directory must be in your PATH in order to use the Java demos that
depend on the Freeze component of Ice.

To run a demo, open a Command Prompt, change to the desired demo
directory, and enter the following command to run the server:

java Server

In a separate Command Prompt window, enter the following command to
run the client:

java Client

Some demo directories contain README files if additional requirements
are necessary.


Running the Python demos
------------------------

The Python demos are in the demopy directory.

You need Python 2.5.2 to run the demos. A binary installer for Python
can be downloaded from:

  http://www.python.org/download

You also need to add the Ice bin directory to your PATH, for example:

set PATH=<Ice installation root directory>\bin;%PATH%

Finally, set PYTHONPATH so that the Python interpreter is able to load
the Ice extension. For a 32-bit Python installation, use this setting:

set PYTHONPATH=<Ice installation root directory>\python

For a 64-bit Python installation, use this setting instead:

set PYTHONPATH=<Ice installation root directory>\python\x64

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate Command Prompt window, type 'python Client.py' to run the
client.


Protocol compression with 64-bit Windows
----------------------------------------

Ice for .NET attempts to dynamically load bzip2.dll to support
protocol compression. On 64-bit Windows, you have to make sure that
the 64-bit version of bzip2.dll is found instead of the 32-bit
version. The bzip2 libraries are installed in <prefix>\bin\x64 and
<prefix>\bin, respectively. For 64-bit Windows, you need to make sure
that <prefix>\bin\x64 appears in the application's PATH instead of
<prefix>\bin. (The Ice run time prints a warning to the console if it
detects a bzip2.dll format mismatch during start-up.)


Binary compatibility
--------------------

Ice patch releases are binary compatible. For example, Ice version 
<x>.<y>.1 is compatible with <x>.<y>.0: you can run an application 
built against Ice <x>.<y>.0 with Ice <x>.<y>.1 (or later) without 
having to recompile or relink this application.

Please refer to the RELEASE_NOTES.txt file included in this
distribution for detailed upgrade instructions.


Ice and the Windows Registry
----------------------------

The Ice installer adds information to the Windows registry to indicate
where it was installed. Developers can use this information to locate
the Ice files in their applications.

The registration key used by this installer is:

HKEY_LOCAL_MACHINE\Software\ZeroC\Ice @ver@ for Visual Studio 2005

The install location is stored as a string value named 'InstallDir'.


Acknowledgments
---------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
