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

 - in C++, using Visual Studio 2008, Visual C++ 2008 Express Edition
   or C++Builder 2010
 - in .NET, using Visual Studio 2008
 - in Java, using Java 5 or Java 6
 - in Python, using Python 2.6.4
 - in Ruby, using Ruby 1.8.6
 - in PHP, using PHP 5.3.1

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
 - Windows Server 2008 Standard R2 (x86 & x64)
 - Windows Vista (x86 & x64)
 - Windows 7 (x86 & x64)


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
<Ice installation root directory>\bin\x64 for a 64-bit system).


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


Building and running the demos
------------------------------

This distribution includes an archive named Ice-@ver@-demos.zip that
contains sample programs for the various supported languages. To build
and run the demos you must first extract this archive in the location
of your choice.


Building and running the C++ demos (Visual C++ 2008)
----------------------------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio 2008 and open the solution
demo\demo.sln. Select your target configuration: Debug or Release,
Win32 or x64 (on supported x64 platforms). Right click on the desired
demo in the Solution Explorer window and select "Build".

Note that if you are using Visual C++ 2008 Express Edition, the Ice
demos that require MFC (demo\Ice\MFC and demo\IcePatch2\MFC) will not
compile since only the Professional edition contains MFC support. You
can either ignore any compile errors these demos generate or just
remove the projects from the solution entirely.

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
demo.sln file mentioned above. If you want to build these demos you
must add the demo project files located in the demo\Database
subdirectories to the demo solution and build from there.

The current examples are for Oracle and require the appropriate
Oracle development environment. Please see the individual demo README
files for more information.


Building and running the C++ demos (C++ Builder 2008)
----------------------------------------------------

The C++ demos are in the demo directory.

To build the C++ demos, open a "RAD Studio Command Prompt" window,
change to the demo directory and run the following command to build
the demos:

  > nmake /f Makefile.mak

Note that this requires Microsoft NMAKE. If you do not already have
NMAKE installed, you can download it as part of the Windows Platform
SDK:

  http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC&displaylang=en

To run these demos, you will typically need at least two Command
Prompt windows. In each command prompt, add the Ice bin directory to
your PATH, as shown below:

set PATH=<Ice installation root directory>\bin;%PATH%
set PATH=<Ice installation root directory>\bin\bcc10;%PATH%

Note that the bin\bcc10 directory must be in the PATH before the bin
directory.

Change to the desired demo subdirectory and review the README file if
one is present. Type 'server' to start the server. In another command
prompt, type 'client' to start the client.


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

  > set JAVA_HOME=<Java SDK installation root directory>
  > set PATH=<Ice installation root directory>\bin;%PATH%
  > set CLASSPATH=<Ice installation root directory>\lib\db.jar;%CLASSPATH%

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


Running the Python demos
------------------------

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


Running the Ruby demos
----------------------

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

  > cd <Ice installation root directory>\demo\Ice\hello
  > server

Then in a separate window, start the Ruby client:

  > cd <Ice installation root directory>\demorb\Ice\hello
  > ruby Client.rb


Ruby/OpenSSL Compatibility Issue
--------------------------------

The Ruby installer includes versions of the OpenSSL DLLs that are not
compatible with the ones supplied with Ice. If you intend to use SSL
in your Ice for Ruby applications, you will need to remove or rename
the following files in the Ruby installation directory:

  libeay32.dll
  ssleay32.dll

If you used the default installation directory, these files are
located in C:\ruby\bin.

Also be aware that the Ruby installer inserts C:\ruby\bin at the
beginning of the system PATH, therefore the DLLs listed above can also
have an adverse impact on other Ice language mappings that use
OpenSSL, such as C++ and Python.


Running the PHP demos
---------------------

PHP demos are provided in the demophp directory.

You will need PHP 5.3.1 to run the demos. It can be downloaded from:

  http://www.php.net/downloads.php

The examples in demophp\Ice\hello and demophp\Glacier2\hello
demonstrate using the Ice extension for PHP in a dynamic Web page,
whereas the example in demophp\Ice\value requires PHP's command-line
interpreter. A README file is provided in each of the example
directories.


Using IcePHP with Apache
------------------------

The binary distribution of PHP for Windows includes loadable modules
for Apache 1, Apache 2.0, and Apache 2.2. The Ice extension for PHP
can be used with all Apache versions.

The PHP documentation describes how to configure the Apache servers
for PHP, and the PHP installer may have already performed the
necessary steps. We provide instructions below for configuring PHP
to use the Ice extension. These instructions make several assumptions:

  * Apache 2.2 is installed and configured to load PHP
  * PHP is installed in C:\Program Files\PHP
  * Ice is installed in C:\Ice

If you have a different installation, you will need to make the
appropriate changes as you follow the instructions.

1) With Apache running, verify that PHP has been loaded successfully
   by creating a file in Apache's document directory (htdocs) called
   phpinfo.php that contains the following line:

   <?php phpInfo();?>

   Open the file in your browser using a URL such as

   http://127.0.0.1/phpinfo.php

   If you have configured PHP correctly, you should see a long page of
   PHP configuration information. If you do not see this page, or an
   error occurs, check Apache's error log as well as the Windows event
   log for more information. Also note that you may need to restart
   Apache if it was running at the time you installed PHP.

2) Review the settings on the browser page for an entry titled
   "Loaded Configuration File". It will have a value such as

   C:\Program Files\PHP\php.ini

   Open this file in a text editor and append the following line:

   extension = php_ice.dll

   The file php_ice.dll contains the Ice extension for PHP.

3) Look for the "extension_dir" setting in the browser page or in
   PHP's configuration file. It typically has the following value by
   default:

   extension_dir = "C:\Program Files\PHP\ext"

   If instead the extension_dir setting contains a relative path, it
   it is resolved relative to the working directory of the Apache
   process (Apache's working directory is usually its installation
   directory).

   Copy the DLL for the Ice extension to PHP's extension directory:

   > copy C:\Ice\bin\php_ice.dll "C:\Program Files\PHP\ext"

4) Verify that Apache can load the dependent libraries for the Ice
   extension. Regardless of the location of PHP's extension directory,
   the Ice extension's dependent libraries must be located in Apache's
   executable search path.

   The Ice extension depends on the following libraries:

   bzip2_vc6.dll
   icevc60_@libver@.dll
   iceutilvc60_@libver@.dll
   msvcp60.dll
   msvcrt.dll
   slicevc60_@libver@.dll
   stlport_vc646.dll

   All of these files can be found in the bin subdirectory of your Ice
   installation (e.g., C:\Ice\bin). Apache must be able to locate
   these DLLs during startup, and several alternatives are available:

   * Add the Ice installation directory to the System PATH. Using the
     System control panel, change the System PATH to include
     C:\Ice\bin. Note that Windows must be restarted for this change
     to take effect.

   * Copy the dependent libraries to Apache's installation directory.

   * Copy the dependent libraries to the Windows system directory
     (\WINDOWS\system32). We do not recommend this option.

   If Apache cannot find or access a DLL, Apache startup may fail with
   an access violation, or the PHP module may ignore the Ice extension
   and continue its initialization. Consequently, a successful Apache
   startup does not necessarily mean that the Ice extension has been
   loaded. Unfortunately, the message reported by PHP in Apache's
   error log is not very helpful; the error might imply that it cannot
   find php_ice.dll when in fact it was able to open php_ice.dll but a
   dependent DLL was missing or inaccessible.

5) Review the access rights on PHP's extension directory, the Ice
   extension DLL, and its dependent libraries. When running as a
   Windows service, Apache runs in the "Local System" account (also
   known as "NT Authority\SYSTEM"). You can use the "cacls" utility in
   a command window to view and modify access rights. For example, run
   the following commands to review the current access rights of the
   Ice extension:

   > cd \Program Files\Apache Software Foundation\Apache2.2
   > cacls php_ice.dll

6) Restart Apache and verify that the PHP module and the Ice extension
   have been loaded successfully. After reloading the phpinfo.php page
   in your browser, scan the entries for a section titled "ice". The
   presence of this section indicates that the extension was loaded.

   If Apache does not start, check the Windows Event Viewer as well as
   Apache's log files for more information. The most likely reasons
   for Apache to fail at startup are missing DLLs (see step 4) or
   insufficient access rights (see step 5).

7) Your application will also need to include at least some of the Ice
   for PHP run-time source files (installed in C:\Ice\php). To make
   these files available to your application, you can either modify
   PHP's include path or copy the necessary files to a directory that
   is already in the interpreter's include path. You can determine the
   current include path by loading the phpinfo.php page in your
   browser and searching for an entry named "include_path".

   If you want to make the Ice run-time files available to all PHP
   applications on the host, you can modify the include_path setting
   in php.ini to add the installation directory:

   include_path = C:\Ice\php;...

   Another option is to modify the include path from within your
   script prior to including any Ice run-time file:

   // PHP
   ini_set('include_path',
     ini_get('include_path') . PATH_SEPARATOR . 'C:/Ice/php')
   require 'Ice.php'; // Load the core Ice run time definitions.


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
