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

 - in C++, using Visual Studio 6.0
 - in Ruby, using Ruby 1.8.6
 - in PHP, using PHP 5.2.10

If you want to develop Ice applications in another programming 
language, or with another C++ compiler, please download the 
appropriate Ice binary distribution from the ZeroC web site at

  http://www.zeroc.com/download.html

You only need the development environment for your target programming
language to use this distribution. For example, if you want to build
Ice applications in Ruby, you need to install Ruby, but do not need 
to install Visual Studio or PHP.

This file describes how to setup Visual Studio for Ice (when building
C++ applications), and provides instructions for building and running
the sample programs.


Supported Windows versions
--------------------------

This distribution is supported on Windows XP with Service Pack 2.


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
the Java GUI is not included in the Visual C++ 6.0 binary distribution.
If you want to use the GUI you can obtain it from one of the other
Windows installers (VS2008) or download the Ice source distribution
and build it yourself.


Setting up Visual Studio 6.0 to build Ice applications in C++
-------------------------------------------------------------

This release requires Visual Studio 6.0 Service Pack 5 or later. We
recommend using the most recent update, Service Pack 6.

Before you can use Ice in your C++ applications, you first need to
configure Visual Studio with the locations of the Ice header files,
libraries, and executables.

- In the IDE, choose Tools->Options->Directories

- Select "Include files"

- Add <Ice installation root directory>\include and <Ice installation
  root directory>\include\stlport

  Move the stlport include directory to the top of the list, or at
  least before the Visual C++ include directories. This is necessary
  so that the compiler uses the STL headers from the STLport library
  instead of the Visual C++ STL library.

- Select "Library files"

- Add <Ice installation root directory>\lib

- Select "Executable files"

- Add <Ice installation root directory>\bin


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build the C++ demos, start Visual Studio and open the workspace
demo\demo.dsw. Set your active project to "all" using Project->Set
Active Project, then start the compilation using Build->Build.

To build the MFC demos, you must have the Unicode library installed.
If you get a link failure about a missing MFC42UD.LIB, run Visual Studio
setup, and add the Unicode library (under "MFC and Template Libraries",
"MS Foundation Class Libraries", "Shared Libraries for Unicode") to
your Visual Studio installation.

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

set PATH=<Ice installation root directory>\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.

The Ice source code archive and Windows installer for Visual Studio
2008 include examples for integrating Ice with databases other than 
Berkeley DB. These demos are not supported for Visual C++ 6.0 and
are not included in this distribution.


Running the Ruby demos
----------------------

The Ruby demos are in the demorb directory.

You need Ruby 1.8.6 to run the demos. A binary installer for Ruby
can be downloaded from:

  http://rubyforge.org/projects/rubyinstaller/

You also need to add the Ice bin directory to your PATH, for example:

set PATH=<Ice installation root directory>\bin;%PATH%

Finally, set RUBYLIB so that the Ruby interpreter is able to load
the Ice extension:

set RUBYLIB=<Ice installation root directory>\ruby;%RUBYLIB%

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

You will need PHP 5.2.10 to run the demos. It can be downloaded from:

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

   extension_dir = "./"

   If the extension_dir setting contains a relative path such as the
   one shown above, it is resolved relative to the working directory
   of the Apache process. Normally Apache's working directory is its
   installation directory. Therefore, unless you change the value of
   extension_dir, the DLL for the Ice extension must be copied to the
   Apache installation directory:

   > copy C:\Ice\bin\php_ice.dll \
        "C:\Program Files\Apache Software Foundation\Apache2.2"

4) Verify that Apache can load the dependent libraries for the Ice
   extension. Regardless of the location of PHP's extension directory,
   the Ice extension's dependent libraries must be located in Apache's
   executable search path.

   The Ice extension depends on the following libraries:

   bzip2.dll
   ice@libver@.dll
   iceutil@libver@.dll
   msvcp60.dll
   msvcrt.dll
   slice@libver@.dll
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

   If Apache cannot find or access a DLL, the PHP module will usually
   ignore the Ice extension and continue its initialization, therefore
   a successful Apache startup does not necessarily mean that the Ice
   extension has been loaded. Unfortunately, the message reported by
   PHP in Apache's error log is not very helpful; the error implies
   that it cannot find php_ice.dll when in fact it was able to open
   php_ice.dll but a dependent DLL was missing.

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

HKEY_LOCAL_MACHINE\Software\ZeroC\Ice @ver@ for Visual Studio 6.0

The install location is stored as a string value named 'InstallDir'.


Acknowledgments
---------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
