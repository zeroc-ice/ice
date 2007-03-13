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
development tools to build Ice applications:

 - in C++, using Visual Studio 6.0
 - in Ruby, using Ruby 1.8.5
 - in PHP, using PHP 5.2.1

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


Running IceGrid and Glacier2 components as services
---------------------------------------------------

The WINDOWS_SERVICES.txt file included in this distribution contains
information on how to install and run the IceGrid registry, IceGrid
node, and Glacier2 router as Windows services.


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build the C++ demos, start Visual Studio and open the workspace
demo\demo.dsw. Set your active project to "all" using Project->Set
Active Project, then start the compilation using Build->Build.

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.


Running the Ruby demos
----------------------

The Ruby demos are in the demorb directory.

You need Ruby 1.8.5 to run the demos. A binary installer for Ruby
can be downloaded from:

  http://rubyforge.org/projects/rubyinstaller/

You also need to add the Ice bin directory to your PATH, for example:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Finally, set RUBYLIB so that the Ruby interpreter is able to load
the Ice extension:

set RUBYLIB=%ICE_HOME%\ruby;%RUBYLIB%

Since Ice for Ruby does not support server-side activities, only 
clients are provided in the demos. In order to run the demos you must
use the corresponding C++ server.

For example, to run the hello application in demo\Ice\hello, we begin
by starting the C++ server:

> cd %ICE_HOME%\demo\Ice\hello
> server

Then in a separate window, start the Ruby client:

> cd %ICE_HOME%\demorb\Ice\hello
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

Two PHP demos are provided in the demophp directory.

To run the PHP demos, you need PHP 5.2.1. It can be downloaded from:

  http://www.php.net/downloads.php

The Ice extension for PHP is provided as bin\php_ice.dll. In order to
use the extension, you must first configure PHP to load it. PHP looks
for extensions in the directory C:\php5 by default, so you can copy
php_ice.dll to this directory and then modify PHP's configuration
file (php.ini) to load the extension using the following directive:

extension = php_ice.dll

If you want to use a different extension directory, you must add
another directive to php.ini as shown below:

extension_dir = C:\MyApp\PHPExtensions
extension = php_ice.dll

Note that the extension_dir directive can be specified multiple times,
but subsequent definitions override previous ones and only the last
definition is actually used to load extensions. Also be aware that the
php.ini-recommended and php.ini-dist files contain a definition for
extension_dir, so if you used one of those files as a starting point
for your php.ini file, you most likely need to disable or remove the
existing definition of extension_dir.

The example in demophp/Ice/hello demonstrates the use of the Ice
extension for PHP in a dynamic Web page, whereas the example in
demophp/Ice/value requires PHP's command line interpreter. Both
examples require that an Ice server be available; a matching server
from any of the other language mappings can be used. A README file is
provided in each of the example directories.

Note that you must modify the php.ini files in each demo directory to
match your PHP installation and ensure that the Ice extension is
loaded properly.


Using IcePHP with Apache
------------------------

The binary distribution of PHP5 for Windows includes loadable modules
for Apache1 and Apache2. The Ice extension can be used with both
Apache versions.

The PHP documentation describes how to configure the Apache servers
for PHP5, but a summary of the steps is provided below. These
instructions assume that you have extracted the PHP5 binary
distribution into the directory C:\PHP5 and that Ice is installed in
C:\Ice. If you have chosen different directories, you will need to
make the appropriate changes as you follow the instructions.

Before proceeding, verify that the Apache server is not currently
running. If you installed the server as a Windows service, you can
use the Services control panel to stop the server.

Apache1 only:

1) Open the Apache configuration file. In the default installation,
   the configuration file can be found here:

   C:\Program Files\Apache Group\Apache\conf\httpd.conf

 a) Add the following line at the end of the LoadModule section:

    LoadModule php5_module "C:/PHP5/php5apache.dll"

 b) Add the following line at the end of the AddModule section:

    AddModule mod_php5.c

 c) Add this line inside the <IfModule mod_mime.c> conditional brace:

    AddType application/x-httpd-php .php

2) Create a php.ini file. You can create an empty one, or copy a
   sample file that is included in the PHP5 distribution. In the
   default configuration, the php.ini file is expected to reside in
   the Windows system directory (e.g., C:\WINDOWS or C:\WINNT).
   For example:

   > cd \WINDOWS
   > copy C:\PHP5\php.ini-recommended php.ini

Apache2 only:

1) Open the Apache configuration file. In the default installation,
   the configuration file can be found here:

   C:\Program Files\Apache Group\Apache2\conf\httpd.conf

   Add the following lines:

   LoadModule php5_module "C:/PHP/php5apache2.dll"
   AddType application/x-httpd-php .php

2) Create a php.ini file. You can create an empty one, or copy a
   sample file that is provided in the PHP5 distribution. In the
   default configuration, the php.ini file is expected to reside in
   the Windows system directory (e.g., C:\WINDOWS or C:\WINNT).
   For example:

   > cd \WINDOWS
   > copy C:\PHP5\php.ini-recommended php.ini

   To place the file in a different directory, add the PHPIniDir
   directive to httpd.conf. For example, the directive below
   indicates that php.ini is located in C:\PHP5:

   PHPIniDir "C:/PHP5"

   NOTE: It is important to use forward slashes in the PHPIniDir
   directive.

Apache1 and Apache2:

3) To ensure that you are using the correct php.ini file, create a
   file in Apache's document directory (htdocs) called phpinfo.php
   that contains the following line:

   <?phpInfo();?>
 
   Temporarily start the Apache server, then open the file in your
   browser using a URL such as

   http://127.0.0.1/phpinfo.php

   If you have configured PHP correctly, you should see a very long
   page of PHP configuration information. Review the entry for
   "Configuration File (php.ini) Path" and verify that its value is
   correct.

   Don't forget to stop the Apache server.

4) Place the Ice extension and its dependencies in a directory that
   is in your System PATH. For example, you could copy these files to
   the Apache directory, or you could add the C:\Ice\bin directory to
   your System PATH using the Environment Variables dialog in the
   System control panel.

   The required files from C:\Ice\bin are listed below:

   bzip2.dll
   icecpp.exe
   ice@libver@.dll
   iceutil@libver@.dll
   msvcp60.dll
   msvcrt.dll
   php_ice.dll
   slice@libver@.dll
   stlport_vc646.dll

   NOTE: If you modify the System PATH, you will need to restart your
   computer for the changes to take effect.

5) Open php.ini and add the following lines to the end of the file:

   extension = php_ice.dll

   If the extension file php_ice.dll is not in C:\PHP5 you will need
   to specify the location using the extension_dir directive. For
   example, if you want to leave the extension in C:\Ice\bin then add
   the following line to php.ini:

   extension_dir = C:\Ice\bin

   Note that the extension_dir directive affects the loading of all
   PHP extensions.

6) Start Apache and verify that the PHP module has been loaded
   successfully. If Apache does not start, check the Windows Event
   Viewer as well as Apache's log files for more information. The most
   likely reasons for Apache to fail at startup are missing DLLs (see
   step 4) or insufficient privilege settings.

7) In order to load Slice definitions for a PHP script, you must
   modify php.ini and then restart Apache. For example, the "hello"
   demo in C:\Ice\demophp\Ice\hello requires the following addition to
   php.ini:

   ice.slice = C:\Ice\demophp\Ice\hello\Hello.ice

   Be aware that specifying a relative path for a Slice file means
   the path is relative to the Apache directory.


Binary compatibility
--------------------

Patch releases of Ice are binary compatible. For example, version <x>.<y>.1
is compatible with <x>.<y>.0, so you can run applications compiled with
<x>.<y>.0 with <x>.<y>.1 (or later) assemblies without having to recompile.

With the binary installers, simply uninstall the previous version of
Ice and install the new one. Already deployed applications that were
compiled against the <x>.<y>.<z> run time will automatically use the
<x>.<y>.<z or z+n> assemblies.


Acknowledgments
---------------

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young 
(eay@cryptsoft.com).
