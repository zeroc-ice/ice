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

 - in C++, using Visual Studio 2008 or Visual C++ 2008 Express Edition
 - in .NET, using Visual Studio 2008
 - in Java, using Java 5 or Java 6
 - in Python, using Python 2.6.1

If you want to develop Ice applications in Ruby or PHP, or with another
C++ compiler, please download the appropriate Ice binary distribution
from the ZeroC web site at

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
 - Windows Server 2003 Standard with Service Pack 2 (x86 & x64)
 - Windows Server 2008 Standard (x86 & x64)
 - Windows Vista (x86 & x64)


Monotonic clock
---------------

Ice uses the QueryPerformanceCounter Windows API function to measure
time with a monotonic clock. If you are experiencing timing or
performance issues, there are two knowledgebase articles that may be
relevant for your system:

  http://support.microsoft.com/?id=896256
  http://support.microsoft.com/?id=895980


Protocol compression
--------------------

The Ice for Java and Ice for .NET run times implement protocol
compression by dynamically loading third-party bzip2 libraries. Ice
disables the protocol compression feature if it is unable to load the
bzip2 library successfully.

In the case of Java, the bzip2 classes are included in the Apache
Ant JAR file (ant.jar). To use protocol compression, you must either
include ant.jar in your CLASSPATH, or you can download just the
bzip2-related classes at the link below:

  http://www.kohsuke.org/bzip2/

Note that these classes are a pure Java implementation of the bzip2
algorithm and therefore add significant latency to Ice requests.

For .NET, Ice attempts to load the native library bzip2.dll from a
directory in your PATH. This DLL is included in your Ice distribution
and can be found in <Ice installation root directory>\bin (or in
<Ice installation root directory>\bin\bin\x64 for a 64-bit system).


Managed code in Ice for .NET
----------------------------

The main Ice for .NET assembly (Ice.dll) included in this distribution
uses unmanaged code. If you require only managed code then you can
download the Ice source distribution and build Ice for .NET in a
purely managed version. Note that the managed version of Ice for .NET
omits support for protocol compression and for signal handling in the
Ice.Application class.

You can download the source distribution at the link below:

    http://www.zeroc.com/download.html


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


Setting up Visual Studio 2008 to build Ice applications in C++
--------------------------------------------------------------

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


Setting up Visual C++ 2008 Express Edition
------------------------------------------

Visual C++ 2008 Express is available for download from:

  http://www.microsoft.com/express/vc

In addition to the steps listed above for setting up Visual Studio
2008, users of Visual C++ 2008 Express Edition must also install
the Platform SDK in order to obtain the Microsoft Message Compiler
(MC.EXE). The Platform SDK is available at the following link:

  http://www.microsoft.com/downloads/details.aspx?FamilyId=F26B1AA4-741A-433A-9BE5-FA919850BDBF&displaylang=en


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio 2008 and open the solution
demo\demo.sln. Select your target configuration: Debug or Release,
Win32 or x64 (on supported x64 platforms). Right click on the desired
demo in the Solution Explorer window and select "Build".

Note that if you are using Visual C++ 2008 Express Edition the Ice
demos that require MFC (demo\Ice\MFC and demo\IcePatch2\MFC) will not
compile since only the Professional edition contains MFC support. You
can either ignore any compile errors these demos generate or just
remove the projects from the solution entirely.

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

To build a C# demo, start Visual Studio 2008 and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".

Note that Visual Studio may present you with a "Security Warning"
dialog when you open the solution. This harmless warning is caused
by the technique that the projects use to compile Slice files. You
can safely allow the projects to load normally.

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your DEVPATH, as shown below:

set DEVPATH=<Ice installation root directory>\bin;%DEVPATH%

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server.exe' to start the server. In another 
command prompt, type 'client.exe' to start the client.


Building and running the Visual Basic demos
-------------------------------------------

The Visual Basic demos are in the demovb directory.

To build a Visual Basic demo, start Visual Studio 2008 and open the
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


Installing the .NET assemblies in the GAC
-----------------------------------------

You can add the .NET assemblies to the Global Assembly Cache (GAC). To
do this, open Windows Explorer and navigate to the directory
C:\WINDOWS\assembly. Next, drag and drop (or copy and paste) the
.NET assemblies from bin directory into the right-hand pane to install
them in the cache.

Or you can use gacutil from the command line to achieve the same
result:

  > gacutil /i <library.dll>

You can find gacutil.exe in the SDK\Tools\Bin folder of your Visual C#
installation. For example, if you have installed Visual C# 8.0 in
C:\Program Files, the path to gacutil is

  C:\Program Files\Microsoft Visual Studio 8\SDK\v2.0\Bin\gacutil.exe

Once installed in the cache, the assemblies will always be located
correctly without having to set environment variables or copy them
into the same directory as an executable.

If you want line numbers for stack traces, you must also install the
PDB (.pdb) files in the GAC. Unfortunately, you cannot do this using
Explorer, so you have to do it from the command line. Open a command
shell window and navigate to C:\WINDOWS\assembly\GAC_MSIL\Ice.
(Assuming C:\WINDOWS is your system root.) Doing a directory listing
there, you will find a directory named @ver@.0__<UUID>, for
example:

  @ver@.0__cdd571ade22f2f16

Change to that directory (making sure that you use the correct version
number for this release of Ice). In this directory, you will see the
Ice.dll you installed into the GAC in the preceding step. Now copy the
Ice.pdb file into this directory:

  > copy <path_to_ice.pdb> .


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

Some of the demos use Freeze for Java. To run these demos you must
also include Freeze.jar in your CLASSPATH:

set CLASSPATH=<Ice installation root directory>\lib\Freeze.jar;%CLASSPATH%

Furthermore, to use a Freeze demo the JVM requires that the directory
containing Berkeley DB's native libraries be included in
java.library.path. Setting PATH as shown above satisfies this
requirement.

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

You need Python 2.6.1 to run the demos. A binary installer for Python
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
@mmver@.1 is compatible with @mmver@.0: you can run an application built
against Ice @mmver@.0 with Ice @mmver@.1 (or later) without having to
recompile or relink this application.

Please refer to the RELEASE_NOTES.txt file included in this
distribution for detailed upgrade instructions.


Ice and the Windows Registry
----------------------------

The Ice installer adds information to the Windows registry to indicate
where it was installed. Developers can use this information to locate
the Ice files in their applications.

The registration key used by this installer is:

HKEY_LOCAL_MACHINE\Software\ZeroC\Ice @ver@ for Visual Studio 2008

The install location is stored as a string value named 'InstallDir'.


Acknowledgments
---------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young
(eay@cryptsoft.com).
