The Internet Communications Engine
----------------------------------

Ice is a modern alternative to object middleware such as CORBA or
COM/DCOM/COM+. It is easy to learn, yet provides a powerful network
infrastructure for demanding technical applications. It features an
object-oriented specification language, easy to use C++, Java, Python,
PHP, C#, and Visual Basic mappings, a highly efficient protocol,
asynchronous method invocation and dispatch, dynamic transport
plug-ins, TCP/IP and UDP/IP support, SSL-based security, a firewall
solution, and much more.

Ice is available under the terms of the GNU General Public License
(GPL) (see LICENSE file). Commercial licenses are available for
customers who wish to use Ice in proprietary products. Please contact
sales@zeroc.com for more information on licensing Ice.

This file describes how to setup Visual Studio for Ice, and provides
instructions for building and running the sample programs.

See doc/README.html for information on the documentation included with
this distribution.


Setting up Visual Studio .NET
-----------------------------

Before you can use Ice in your C++ applications, you first need to
configure Visual Studio with the locations of the Ice header files,
libraries, and executables.

- In the IDE, choose Tools->Options->Projects->VC++ Directories

- Select "Include files"

- Add <Ice installation root directory>\include

- Select "Library files"

- Add <Ice installation root directory>\lib

- Select "Executable files"

- Add <Ice installation root directory>\bin


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio .NET and open the solution
demo\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.

Note that extra steps may be required when running the IceBox demo,
depending on how you compiled the demos. If you used the Release
configuration, then no extra steps are necessary. However, if you used
the Debug configuration, you will need to use the debug version of
icebox.exe located in the bin\debug subdirectory.


Building and running the Java demos
-----------------------------------

The Java demos are in the demoj directory.

To build the Java demos, you need the Java SDK 1.4.2 or later, and Ant
1.6.2. They can be downloaded from:

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


Building and running the C# demos
---------------------------------

The C# demos are in the democs directory.

To build the C# demos, you need Visual C# 7.1, build 3088 or later,
and .NET 1.1, build 4322 or later.

To build a C# demo, start Visual Studio .NET and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


Building and running the Visual Basic demos
-------------------------------------------

The Visual Basic demos are in the demovb directory.

To build the Visual Basic demos, you need Visual Basic .NET 2003 and
.NET 1.1, build 4322 or later.

To build a Visual Basic demo, start Visual Studio .NET and open the
solution demovb\demo.sln. Right click on the desired demo in the
Solution Explorer window and select "Build".

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


Running the Python demos
------------------------

The Python demos are in the demopy directory.

You need Python 2.4 or later to run the demos. A binary installer for
Python 2.4 can be downloaded from:

http://www.python.org/download

You also need to add the Ice bin directory to your PATH, for example:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Finally, set PYTHONPATH so that the Python interpreter is able to load
the Ice extension:

set PYTHONPATH=%ICE_HOME%\bin;%ICE_HOME%\python

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate Command Prompt window, type 'python Client.py' to run the
client.


Running the PHP demos
---------------------

Two PHP demos are provided in the demophp directory.

To run the PHP demos, you need PHP 5.0.3. It can be downloaded from:

http://www.php.net/downloads.php

The example in demophp/Ice/hello demonstrates the use of the Ice
extension for PHP in a dynamic Web page. The example in
demophp/Ice/value is used with PHP's command line interpreter. Both
examples require that an Ice server be available; a matching server
from any of the other language mappings can be used. A README file is
provided in each of the example directories.
