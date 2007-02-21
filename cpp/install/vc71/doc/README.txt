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
development tools to build Ice applications in C++ using Visual
Studio .NET 2003.

If you want to develop Ice applications in another programming
language, on with another C++ compiler, please download the 
corresponding Ice binary distribution from 
http://www.zeroc.com/download.html.

This file describes how to setup Visual Studio for Ice and provides 
instructions for building and running the sample programs.

See doc/README.html for information on the documentation included with
this distribution.


Setting up Visual Studio .NET to build Ice applications in C++
--------------------------------------------------------------

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


Binary compatibility
--------------------

Patch releases of Ice are binary compatible. For example, version
<x>.<y>.1 is compatible with <x>.<y>.0, so you can run applications
compiled with <x>.<y>.0 with <x>.<y>.1 (or later) assemblies without
having to recompile.

With the binary installers, simply uninstall the previous version of
Ice and install the new one. Already deployed applications that were
compiled against the <x>.<y>.<z> run time will automatically use the
<x>.<y>.<z or z+n> assemblies.

Note: Under Mono, binary compatibility currently does not work due to
      issues with Mono. Until this problem in Mono is fixed, you cannot
      run applications compiled with previous minor versions of Ice
      against a newer version of the Ice assemblies. For example, an
      application compiled with version <x>.<y>.0 of Ice cannot run with
      the <x>.<y>.1 Ice assemblies.


Acknowledgements
----------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
