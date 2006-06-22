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


Visual Studio Requirements
--------------------------

This release requires Visual Studio 2005 version 8.0.50727.42 or
later. For C# or Visual Basic, this release also requires the .NET
Framework Version 2.0.50727 or later.


Setting up Visual Studio 2005
-----------------------------

Before you can use Ice in your C++ applications, you first need to
configure Visual Studio with the locations of the Ice header files,
libraries, and executables.

- In the IDE, choose Tools->Options->Projects and Solutions->VC++ Directories

- Select "Include files"

- Add <Ice installation root directory>\include

- Select "Library files"

- Add <Ice installation root directory>\lib

- Select "Executable files"

- Add <Ice installation root directory>\bin


Visual C++ 2005 Express Notes
-----------------------------

In addition to the steps listed above for setting up Visual Studio
2005, users of Visual C++ 2005 Express must also follow the
instructions at the link below for installing and configuring the
Platform SDK:

http://msdn.microsoft.com/vstudio/express/visualc/usingpsdk/


Using the IceGrid Administrative Console
----------------------------------------

A Java-based graphical tool for administering IceGrid applications
is included in this distribution. The Java archive (JAR) file is
installed as

<Ice installation root directory>\bin\IceGridGUI.jar

With a suitable Java installation, you can execute the application
directly by double-clicking on its icon, or you can start it from
a command prompt:

> java -jar IceGridGUI.jar


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build a C++ demo, start Visual Studio 2005 and open the solution
demo\demo.sln. Right click on the desired demo in the Solution Explorer
window and select "Build".

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.


Building and running the C# demos
---------------------------------

The C# demos are in the democs directory.

To build a C# demo, start Visual Studio 2005 and open the solution
democs\demo.sln. Right click on the desired demo in the Solution
Explorer window and select "Build".

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.

For backward compatibility with .NET 1.1 users, support for SSL is
disabled by default in the demos. To enable SSL, edit the config.*
files in each demo directory and follow the instructions in the
comments.


Building and running the Visual Basic demos
-------------------------------------------

The Visual Basic demos are in the demovb directory.

To build a Visual Basic demo, start Visual Studio 2005 and open the
solution demovb\demo.sln. Right click on the desired demo in the
Solution Explorer window and select "Build".

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'server.exe' to start the server. In a separate
Command Prompt window, type 'client.exe' to run the client.

Some demo directories contain README files if additional requirements
are necessary.


Building and running the Java demos
-----------------------------------

The Java demos are in the demoj directory.

To build the Java demos, you need the Java SDK 1.4.2 or later, and Ant
1.6.3 or later. They can be downloaded from:

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

If you prefer to use Ice for Java5 instead, modify your CLASSPATH
as shown below:

set CLASSPATH=%ICE_HOME%\lib\java5\Ice.jar;%ICE_HOME%\lib\db.jar;classes;%CLASSPATH%

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


Running the Python demos
------------------------

The Python demos are in the demopy directory.

You need Python 2.4.3 to run the demos. A binary installer for Python
can be downloaded from:

http://www.python.org/download

You also need to add the Ice bin directory to your PATH, for example:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Finally, set PYTHONPATH so that the Python interpreter is able to load
the Ice extension:

set PYTHONPATH=%ICE_HOME%\python

To run a demo, open a Command Prompt, change to the desired demo
directory, and type 'python Server.py' to start the server. In a
separate Command Prompt window, type 'python Client.py' to run the
client.


Running the PHP demos
---------------------

Two PHP demos are provided in the demophp directory.

To run the PHP demos, you need PHP 5.1.4. It can be downloaded from:

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
php.ini-recommended file contains a definition for extension_dir, so
if you used that file as a starting point for your php.ini file, you
most likely need to disable or remove the existing definition of
extension_dir.

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

   php_ice.dll
   ice31.dll
   iceutil31.dll
   slice31.dll
   icecpp.exe
   bzip2.dll
   msvcp80.dll
   msvcr80.dll

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
   successfully. If Apache does not start, check the Windows Event Viewer
   as well as Apache's log files for more information. The most likely
   reason for Apache to fail at startup is missing DLLs (see step 4).

7) In order to load Slice definitions for a PHP script, you must
   modify php.ini and then restart Apache. For example, the "hello" demo
   in C:\Ice\demophp\Ice\hello requires the following addition to
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

Note: Under Mono, binary compatibility currently does not work due to
      issues with Mono. Until this problem in Mono is fixed, you cannot
      run applications compiled with previous minor versions of Ice
      against a newer version of the Ice assemblies. For example, an
      application compiled with version <x>.<y>.0 of Ice cannot run with
      the <x>.<y>.1 Ice assemblies.

