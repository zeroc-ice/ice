The Internet Communications Engine
----------------------------------

Ice, the Internet Communications Engine, is middleware for the practical
programmer. A high-performance Internet communications platform, Ice 
includes a wealth of layered services and plug-ins. Ice means 
simplicity, speed, and power.

Ice is available under the terms of the GNU General Public License (GPL)
(see LICENSE file). Commercial licenses are available for customers who 
wish to use Ice with proprietary products. 

Please contact sales@zeroc.com for more information.

Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, you just need to open demo\demo.sln in Visual
Studio .NET, right click on it in the Solution Explorer window and
select "Build". 

To run these demos, you need to add the Ice bin directory to your PATH,
for example:

set PATH=C:\Ice-1.1.0\bin;%PATH%

Then open a Command Prompt, cd to the desired demo subdirectory, and
type 'server' to start the server. In a separate Command Prompt window,
type 'client' to start the client.

Building and running the Java demos
-----------------------------------

The Java demos are in the demoj directory.

To build the Java demos, you need the Java SDK 1.4.1 and Ant 1.5. They 
can be downloaded from:

http://java.sun.com/j2se/1.4.1/download.html
http://ant.apache.org/bindownload.cgi

You also need to set the following environment variables:

set JAVA_HOME=<Java SDK installation root directory>
set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%
set CLASSPATH=%ICE_HOME%\lib\Ice.jar;%ICE_HOME%\lib\db.jar;%CLASSPATH%

Then add the ant bin directory to your PATH, open a Command Prompt and 
type 'ant' in any demo subdirectory to build the corresponding Java 
demo.

To run a demo, open a Command Prompt, cd to the desired demo directory,
and enter the following command to run the server:

java -cp "classes;%CLASSPATH%" Server

In a separate Command Prompt window, enter the following command to
run the client:

java -cp "classes;%CLASSPATH%" Client
