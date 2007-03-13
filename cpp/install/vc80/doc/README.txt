The Internet Communications Engine
----------------------------------

Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+. It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, C#, Java,
Python, Ruby, PHP, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic
transport plug-ins, TCP/IP and UDP/IP support, SSL-based security, a
firewall solution, and much more.

Ice is available under the terms of the GNU General Public License
(GPL) (see LICENSE file). Commercial licenses are available for
customers who wish to use Ice in proprietary products. Please contact
sales@zeroc.com for more information on licensing Ice.


About this distribution
-----------------------

This binary distribution provides all Ice run time services and 
development tools to build Ice applications:

 - in C++, using Visual Studio 2005 or Visual C++ 2005 Express Edition
 - in C#, using Visual Studio 2005 or Visual C# 2005 Express Edition
 - in Visual Basic, using Visual Studio 2005 or Visual Basic 2005 
   Express Edition
 - in Java, using Java 2, Java 5 or Java 6
 - in Python, using Python 2.5

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
 - Windows Server 2003 Standard (x86)
 - Windows Vista (x86)

Setting up Visual Studio 2005 to build Ice applications in C++
--------------------------------------------------------------

Before you can use Ice in your C++ applications, you first need to
configure Visual Studio with the locations of the Ice header files,
libraries, and executables.

- In the IDE, choose Tools->Options->Projects and Solutions->VC++ Directories

- Select "Include files"

- Add <Ice installation root directory>\include

- Select "Library files"

- Add <Ice installation root directory>\lib

- Select "Executable files"

- Add <Ice installation root directory>\bin


Running IceGrid and Glacier2 components as services
---------------------------------------------------

The WINDOWS_SERVICES.txt file included in this distribution contains
information on how to install and run the IceGrid registry, IceGrid
node, and Glacier2 router as Windows services.


Setting up Visual C++ 2005 Express Edition
------------------------------------------

Visual C++ 2005 Express is available for download from:

  http://msdn.microsoft.com/vstudio/express/visualc/

In addition to the steps listed above for setting up Visual Studio
2005, users of Visual C++ 2005 Express Edition must also follow the
instructions at the link below for installing and configuring the
Platform SDK:

  http://msdn.microsoft.com/vstudio/express/visualc/usingpsdk/


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio 2005 and open the solution
demo\demo.sln. Right click on the desired demo in the Solution Explorer
window and select "Build".

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.


Building and running the C# demos
---------------------------------

The C# demos are in the democs directory.

To build a C# demo, start Visual Studio 2005 and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


SSL Notes for the C# demos
--------------------------

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


Building and running the Visual Basic demos
-------------------------------------------

The Visual Basic demos are in the demovb directory.

If you are using Visual Basic 2005 Express, you must execute a
command-line tool before building any of the demos. Change to the
demovb directory and run this command:

..\bin\generatevb . build

To build a Visual Basic demo, start Visual Studio 2005 and open the
solution demovb\demo.sln. Right click on the desired demo in the
Solution Explorer window and select "Build".

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


Building and running the Java demos
-----------------------------------

The Java demos are in the demoj directory.

To build the Java demos, you need the Java SDK 1.4.2 or later, and Ant
1.6.3 or later. They can be downloaded from:

  http://java.sun.com/j2se/index.jsp
  http://ant.apache.org/bindownload.cgi

The ant bin directory must be added to your PATH, and the following
environment variables need to be defined:

set JAVA_HOME=<Java SDK installation root directory>
set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

You can build all of the demos by running 'ant' in the demoj
directory, or you can build individual demos by changing to the
desired subdirectory and then running 'ant'.

Before running the demos you must modify your CLASSPATH as follows:

set CLASSPATH=%ICE_HOME%\lib\Ice.jar;%ICE_HOME%\lib\db.jar;classes;%CLASSPATH%

If you prefer to use Ice for Java2 rather than Java5, modify your CLASSPATH
as shown below:

set CLASSPATH=%ICE_HOME%\lib\java2\Ice.jar;%ICE_HOME%\lib\db.jar;classes;%CLASSPATH%

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

You need Python 2.5 to run the demos. A binary installer for Python
can be downloaded from:

  http://www.python.org/download

You also need to add the Ice bin directory to your PATH, for example:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Finally, set PYTHONPATH so that the Python interpreter is able to load
the Ice extension:

set PYTHONPATH=%ICE_HOME%\python

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate Command Prompt window, type 'python Client.py' to run the
client.


Binary compatibility
--------------------

Patch releases of Ice are binary compatible. For example, version <x>.<y>.1
is compatible with <x>.<y>.0, so you can run applications compiled with
<x>.<y>.0 with <x>.<y>.1 (or later) assemblies without having to recompile.

With the binary installers, simply uninstall the previous version of
Ice and install the new one. Already deployed applications that were
compiled against the <x>.<y>.<z> run time will automatically use the
<x>.<y>.<z or z+n> assemblies.


Acknowledgments
---------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
