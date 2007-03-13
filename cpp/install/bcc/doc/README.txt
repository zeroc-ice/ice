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
development tools to build Ice applications in C++ using Borland
C++Builder 2006 Update 2.

If you want to develop Ice applications in another programming
language, on with another C++ compiler, please download the 
corresponding Ice binary distribution from 
http://www.zeroc.com/download.html.


Supported Windows versions
--------------------------

This distribution is supported on Windows XP with Service Pack 2.


Running IceGrid and Glacier2 components as services
---------------------------------------------------

The WINDOWS_SERVICES.txt file included in this distribution contains
information on how to install and run the IceGrid registry, IceGrid
node, and Glacier2 router as Windows services.


Demos
-----

Sample programs are provided in the Ice-@ver@-demos.zip package,
which can be downloaded from the ZeroC web site at

http://www.zeroc.com/download.html

Please refer to the README.DEMOS file included in that package for
more information.


Binary compatibility
--------------------

Patch releases of Ice are binary compatible. For example, version <x>.<y>.1
is compatible with <x>.<y>.0, so you can run applications compiled with
<x>.<y>.0 with <x>.<y>.1 (or later) assemblies without having to recompile.

With the binary installers, simply uninstall the previous version of
Ice and install the new one. Already deployed applications that were
compiled against the <x>.<y>.<z> run time will automatically use the
<x>.<y>.<z or z+n> assemblies.


Acknowledgements
----------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
