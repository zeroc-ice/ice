The Internet Communications Engine
----------------------------------

Ice, the Internet Communications Engine, is middleware for the
practical programmer. A high-performance Internet communications
platform, Ice includes a wealth of layered services and plug-ins.
Ice means simplicity, speed, and power.

Ice is available under the terms of the GNU General Public License
(GPL) (see LICENSE file). Commercial licenses are available for
customers who wish to use Ice with proprietary products. 

Please contact sales@zeroc.com for more information.

Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, you just need to open demo\demo.sln in Visual
Studio .NET, right click on it in the Solution Explorer window and
select "Build". 

To run these demos, you need to add the Ice bin directory to your
PATH, for example:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Then open a Command Prompt, change to the desired demo subdirectory,
and and type 'server' to start the server. In a separate Command
Prompt window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.

Note that extra steps may be required when running the IceBox demo,
depending on how you compiled the demos. If you used the Release
configuration, then no extra steps are necessary. However, if you
used the Debug configuration, you will need to use the debug
version of icebox.exe located in the bin\debug subdirectory.

Building and running the Java demos
-----------------------------------

The Java demos are in the demoj directory.

To build the Java demos, you need the Java SDK 1.4.2 or later,
and Ant 1.5. They can be downloaded from:

http://java.sun.com/j2se/index.jsp
http://ant.apache.org/bindownload.cgi

You also need to set the following environment variables:

set JAVA_HOME=<Java SDK installation root directory>
set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%
set CLASSPATH=%ICE_HOME%\lib\Ice.jar;%ICE_HOME%\lib\db.jar;classes;%CLASSPATH%

Then add the ant bin directory to your PATH, open a Command Prompt
and type 'ant' in any demo subdirectory to build the corresponding
Java demo.

To run a demo, open a Command Prompt, change to the desired demo
directory, and enter the following command to run the server:

java Server

In a separate Command Prompt window, enter the following command to
run the client:

java Client

Some demo directories contain README files if additional requirements
are necessary.

Building and running the C# demos
-----------------------------------

The C# demos are in the democs directory.

To build the C# demos, you need Visual C# 7.1, build 3088 or later,
and .NET 1.1, build 4322 or later.

To build a C# demo, you need to open democs\demo.sln in Visual
Studio .NET, right click on it in the Solution Explorer window and
select "Build". 

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.

Building and running the Visual Basic demos
-------------------------------------------

The Visual Basic demos are in the demovb directory.

To build the Visual Basic demos, you need Visual Basic .NET 2003
and .NET 1.1, build 4322 or later.

To build a Visual Basic demo, you need to open demovb\demo.sln in
Visual Studio .NET, right click on it in the Solution Explorer window
and select "Build". 

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.

Running the Python demos
------------------------

The Python demos are in the demopy directory.

To run the Python demos, you need Python 2.4 or later.
It can be downloaded from:

http://www.python.org/download

You also need to add the Ice bin directory to your
PATH, for example:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

You should also set PYTHONPATH so that the Python interpreter is able
to load the Ice extension:

set PYTHONPATH=%ICE_HOME%\bin;%ICE_HOME%\python

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate Command Prompt window, type 'python Client.py' to run the
client.

Running the PHP demos
---------------------

Two PHP demos are provided in the demophp directory. 

To run the PHP demos, you need PHP 5.0.3 or later. 
It can be downloaded from:

http://www.php.net/downloads.php

The example in demophp/Ice/hello demonstrates the use of the Ice extension for
PHP in a dynamic Web page. The example in demophp/Ice/value is used
with PHP's command line interpreter. Both examples require that an
Ice server be available; a matching server from any of the other
language mappings can be used. A README file is provided in each of
the example directories.
