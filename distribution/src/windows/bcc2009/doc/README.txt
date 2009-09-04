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
development tools to build Ice applications in C++ using C++Builder
2009.

If you want to develop Ice applications in another programming
language, or with another C++ compiler, please download the 
corresponding Ice binary distribution from 

  http://www.zeroc.com/download.html


Supported Windows versions
--------------------------

This distribution is supported on

  - Windows XP with Service Pack 2
  - Windows Vista


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


IceGrid GUI
-----------

Some of the IceGrid demos recommend using the IceGrid GUI. However
the Java GUI is not included in C++Builder binary distribution. If you
want to use the GUI you can obtain it from one of the other Windows
installers (VS2008) or download the Ice source distribution and build
it yourself.


NMAKE and Message Compiler (MC.EXE)
-----------------------------------

In order to build Ice demos, you will need the Microsoft NMAKE and
Message Compiler utilities. These are included in all full Microsoft
C++ compiler distributions. 

You can also download them as part of the Windows Platform SDK:

  http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC&displaylang=en


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build the C++ demos, open a "RAD Studio Command Prompt" and
change to the demo directory and run the following command to build
the demos

  > nmake /f Makefile.mak

Note that this requires Microsoft NMAKE. If you do not already have
NMAKE installed, you can download it as part of the Windows Platform
SDK:

  http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC&displaylang=en

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

  > set PATH=<Ice installation root directory>\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.


Binary compatibility
--------------------

Ice patch releases are binary compatible. For example, Ice version
@mmver@.1 is compatible with @mmver@.0: you can run an application built
against Ice @mmver@.0 with Ice @mmver@.1 (or later) without having to
recompile or relink this application.

Please refer to the RELEASE_NOTES.txt file included in this
distribution for detailed upgrade instructions.


Acknowledgments
---------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
