This archive contains sample programs for C++, Java, C#, VB, Ruby,
Python and PHP. To build and run the demos, follow the language-
specific instructions below.


======================================================================
Building and running the C++ demos (Visual C++ 2008)
======================================================================

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio 2008 and open the solution
demo\demo.sln. Select your target configuration: Debug or Release,
Win32 or x64 (on supported x64 platforms). Right click on the desired
demo in the Solution Explorer window and select "Build".

Note that the Visual Studio project files require the Ice Visual
Studio Extension.

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your PATH, as shown below:

  > set PATH=<Ice installation root directory>\bin;%PATH%

If you built an x64 configuration, use this setting instead (the
x64 directory must come first in your PATH):

  > set PATH=<Ice installation root directory>\bin;%PATH%
  > set PATH=<Ice installation root directory>\bin\x64;%PATH%

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server' to start the server. In another command
prompt, type 'client' to start the client.

This distribution includes examples for integrating Ice with databases
other than Berkeley DB. These examples are not included in the
demo.sln file mentioned above. If you want to build these demos, you
must add the demo project files located in the demo\Database
subdirectories to the demo solution and build from there.

The current examples are for Oracle and require the appropriate
Oracle development environment. Please see the individual demo README
files for more information.


======================================================================
Building and running the C++ demos (Visual Studio 2008 Express)
======================================================================

The C++ demos are in the demo directory.

NMAKE is used to build the demos with Visual Studio Express since it
doesn't support the Ice Visual Studio extension.

To build the C++ demos, open a "Visual Studio 2008 Command Prompt"
window, change to the demo directory and run the following commands to
build the demos:

  > set CPP_COMPILER=VC90_EXPRESS
  > nmake /f Makefile.mak

Note that the Ice demos that require MFC (demo\Ice\MFC and
demo\IcePatch2\MFC) will not compile since the Express edition does
not include MFC support.

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your PATH, as shown below:

  > set PATH=<Ice installation root directory>\bin;%PATH%

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server' to start the server. In another command
prompt, type 'client' to start the client.


======================================================================
Building and running the C++ demos (C++ Builder 2010)
======================================================================

The C++ demos are in the demo directory.

To build the C++ demos, open a "RAD Studio Command Prompt" window,
change to the demo directory and run the following command to build
the demos:

  > set CPP_COMPILER=BCC2010
  > nmake /f Makefile.mak

Note that this requires Microsoft NMAKE. If you do not already have
NMAKE installed, you can download it as part of the Windows Platform
SDK:

  http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC&displaylang=en

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your PATH, as shown below:

  > set PATH=<Ice installation root directory>\bin;%PATH%
  > set PATH=<Ice installation root directory>\bin\bcc10;%PATH%

Note that the bin\bcc10 directory must be in the PATH before the bin
directory.

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server' to start the server. In another command
prompt, type 'client' to start the client.


======================================================================
Building and running the .NET demos
======================================================================

Note that the Visual Studio project files require the Ice Visual
Studio Extension.


Building the C# demos
---------------------

The C# demos are in the democs directory.

To build a C# demo, start Visual Studio 2008 and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".


Building the Visual Basic demos
-------------------------------

The Visual Basic demos are in the demovb directory.

To build a Visual Basic demo, start Visual Studio 2008 and open the
solution demovb\demo.sln. Right click on the desired demo in the
Solution Explorer window and select "Build".


Running .NET demos
------------------

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your DEVPATH, as shown below:

set DEVPATH=<Ice installation root directory>\bin;%DEVPATH%

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server.exe' to start the server. In another
command prompt, type 'client.exe' to start the client.


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


======================================================================
Building and running the Java demos
======================================================================

The Java demos are in the demoj directory.

To build the Java demos, you need J2SE SDK 1.5.0 or later, and Ant
1.7.0 or later. They can be downloaded from:

  http://java.sun.com/j2se/index.jsp
  http://ant.apache.org/bindownload.cgi

The ant bin directory must be added to your PATH, and the following
environment variables need to be defined:

  > set JAVA_HOME=<Java SDK installation root directory>
  > set PATH=<Ice installation root directory>\bin;%PATH%

You can build all of the demos by running 'ant' in the demoj
directory, or you can build individual demos by changing to the
desired subdirectory and then running 'ant'.

Before running the demos you must modify your CLASSPATH as follows:

  > set CLASSPATH=<Ice installation root directory>\lib\Ice.jar;classes;%CLASSPATH%

Some of the demos use Freeze for Java. To run these demos you must
also include Freeze.jar in your CLASSPATH:

  > set CLASSPATH=<Ice installation root directory>\lib\Freeze.jar;%CLASSPATH%

Furthermore, to use a Freeze demo the JVM requires that the directory
containing Berkeley DB's native libraries be included in
java.library.path. Setting PATH as shown above satisfies this
requirement.

To run a demo, open a Command Prompt, change to the desired demo
directory, and enter the following command to run the server:

  > java Server

In a separate Command Prompt window, enter the following command to
run the client:

  > java Client

Some demo directories contain README files if additional requirements
are necessary.


======================================================================
Running the Python demos
======================================================================

The Python demos are in the demopy directory.

You need Python 2.6.4 to run the demos. A binary installer for Python
can be downloaded from:

  http://www.python.org/download

You also need to add the Ice bin directory to your PATH, for example:

  > set PATH=<Ice installation root directory>\bin;%PATH%

Finally, set PYTHONPATH so that the Python interpreter is able to load
the Ice extension. For a 32-bit Python installation, use this setting:

  > set PYTHONPATH=<Ice installation root directory>\python

For a 64-bit Python installation, use this setting instead:

  > set PYTHONPATH=<Ice installation root directory>\python\x64

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate Command Prompt window, type 'python Client.py' to run the
client.


======================================================================
Running the Ruby demos
======================================================================

The Ruby demos are in the demorb directory.

You need Ruby 1.8.6 to run the demos. A binary installer for Ruby
can be downloaded from:

  http://rubyforge.org/projects/rubyinstaller/

You also need to add the Ice bin directory to your PATH, for example:

  > set PATH=<Ice installation root directory>\bin;%PATH%

Finally, set RUBYLIB so that the Ruby interpreter is able to load
the Ice extension:

  > set RUBYLIB=<Ice installation root directory>\ruby;%RUBYLIB%

Since Ice for Ruby does not support server-side activities, only
clients are provided in the demos. In order to run the demos you must
use the corresponding C++ server.

For example, to run the hello application in demo\Ice\hello, we begin
by starting the C++ server:

  > cd <Ice demo root directory>\demo\Ice\hello
  > server

Then in a separate window, start the Ruby client:

  > cd <Ice demo root directory>\demorb\Ice\hello
  > ruby Client.rb


======================================================================
Running the PHP demos
======================================================================

PHP demos are provided in the demophp directory.

You will need PHP 5.3.1 to run the demos. It can be downloaded from:

  http://www.php.net/downloads.php

The examples in demophp\Ice\hello and demophp\Glacier2\hello
demonstrate using the Ice extension for PHP in a dynamic Web page,
whereas the example in demophp\Ice\value requires PHP's command-line
interpreter. A README file is provided in each of the example
directories.
