======================================================================
The Internet Communications Engine
======================================================================

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


======================================================================
About this distribution
======================================================================

This binary distribution provides all Ice run time services and
development tools to build Ice applications:

 - in C++, using Visual Studio 2008 SP1, Visual C++ 2008 Express
   Edition SP1, or C++Builder 2010
 - in .NET, using Visual Studio 2008 SP1
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


======================================================================
Supported Windows versions
======================================================================

This distribution is supported on the following Windows versions:

 - Windows XP with Service Pack 3 (x86)
 - Windows Server 2003 Standard with Service Pack 2 (x86 & x64)
 - Windows Server 2008 Standard with Service Pack 2 (x86 & x64)
 - Windows Server 2008 Standard R2 (x64)
 - Windows Vista with Service Pack 2 (x86 & x64)
 - Windows 7 (x86 & x64)


======================================================================
Third Party Packages
======================================================================

This distribution contains binaries for various third party packages
that are required to use Ice.

 - Berkeley DB 4.8.24 (Visual C++ 2008, C++Builder 2010)
 - OpenSSL 0.9.8k (Visual C++ 2008, Visual C++ 6.0)
 - Expat 2.0.1 (Visual C++ 2008)
 - Bzip2 1.0.5 (Visual C++ 2008, Visual C++ 6.0)
 - Qt 4.5.3 (Visual C++ 2008)
 - STLport 4.6.2 (Visual C++ 6.0)

The licenses for these packages are provided in the file
THIRD_PARTY_LICENSE.txt.

Qt SQL Driver
-------------

The QtSql DLL included in this distribution has the SQLite driver
built-in.


======================================================================
Ice Visual Studio 2008 Extension
======================================================================

The Ice Visual Studio Extension integrates Ice projects into the
Visual Studio 2008 IDE. The extension supports C++, .NET, and
Silverlight projects.

The extension is only installed if Visual Studio 2008 is present on
the target machine. If you install Visual Studio 2008 after installing
Ice, you will have to rerun the Ice installer and choose "Repair" to
install the extension.

Note that the extension is not supported for Visual Studio 2008
Express as Microsoft does not permit extensions to be written for
Express editions of Visual Studio.


Activating the plug-in for a project
------------------------------------

After creating or loading a project, right-click on the project in
Solution Explorer and choose "Ice Configuration..." or go to "Ice
Configuration..." in the "Tools" menu. This opens a dialog where you
can configure Ice build properties.

Note that after adding new configurations or platforms to your
project, it may be necessary to disable and then re-enable the plug-in
in order for the new configuration/platform to have the correct Ice
settings.


Project properties
------------------

  * Ice Home

    Set the directory where Ice is installed.

  * Slice Compiler Options

    Tick the corresponding check boxes to pass options such as --ice,
    --stream, --checksum, or --tie (.NET only) to the Slice compiler.

    Tick "Console Output" if you want compiler output to appear in the
    Output window.

  * Extra Compiler Options

    Add extra Slice compiler options that are not explicitly supported
    above.

    These options must be entered the same as they would be on the
    command line to the Slice compiler. For example, preprocessor
    macros can be defined by entering the following:

    -DFOO -DBAR

  * Slice Include Path

    Set the list of directories to search for included Slice files
    (-I option).

    The checkbox for each path specifies whether the path should be
    relative to the project directory or an absolute path. Note that
    paths that use environment variables are not changed by this
    feature.

  * DLL Export Symbol (C++ only)

    Set the symbol to use for DLL exports (--dll-export option).

  * Ice Components

    Set the list of Ice libraries to link with.


Environment Variables
---------------------

The "Ice Home", "Extra Compiler Options", and "Slice Include Path"
settings support the use of environment variables. Use the $(VAR)
syntax to refer to an environment variable named VAR. For example,
if you have defined the ICE_HOME environment variable, you could
use $(ICE_HOME) in the "Ice Home" field.


Adding Slice files to a project
-------------------------------

Use "Add -> New Item..." to create a Slice file and add it to a
project. Use "Slice File (.ice)" as the file type. To add an existing
Slice file, use "Add -> Existing Item...".


Generating code
---------------

The extension compiles a Slice file whenever you save the file. The
extension also tracks dependencies among Slice files in the project
and recompiles only those files that require it after a change.

Generated files are automatically added to the project. For example,
for Demo.ice, the extension adds Demo.cpp and Demo.h to a C++
project, whereas the extension adds Demo.cs to a C# project.

Errors that occur during Slice compilation are displayed in the Visual
Studio "Output" and "Error List" panels.


VC++ Pre-compiled headers
-------------------------

For C++ projects, pre-compiled headers are detected automatically.
(The extension automatically passes the required --add-header option
to slice2cpp.)

If you change the pre-compiled header setting of a project, you must
rebuild the project.


======================================================================
Setting up your environment to use Ice
======================================================================

Visual C++
----------

To use Ice for C++ you must add the Ice bin directory(s) to your PATH.

  > set PATH=<Ice installation root directory>\bin;%PATH%

If you built an x64 platform, use this setting instead (the x64
directory must come first in your PATH):

  > set PATH=<Ice installation root directory>\bin;%PATH%
  > set PATH=<Ice installation root directory>\bin\x64;%PATH%

If you do not want to use the Ice Visual Studio Extension described
above then you will have to configure Visual Studio manualy in order
to build Ice applications. This involves adding the locations of the
Ice header files, libraries, and executables to Visual Studio's
configuration.

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


C++Builder
----------

To use Ice for C++ with C++Builder you must add the Ice bin
directories to your PATH.

  > set PATH=<Ice installation root directory>\bin;%PATH%
  > set PATH=<Ice installation root directory>\bin\bcc10;%PATH%

Note that the bin\bcc10 directory must be in the PATH before the bin
directory.


.NET
----

To use Ice for .NET, you can either copy the .NET assemblies to the
directory of your executable, use the DEVPATH environment variable or
add the .NET assemblies to the Global Assembly Cache (GAC):

- Copying the Ice for .NET assemblies to the executable directory is
  the most straightforward. 

  You can setup your Visual Studio projects to copy the assemblies by
  setting the "Copy Local" property to "True". 
  
  To access this property, in the Solution Explorer, open the
  References folder of your project and click on the assembly to
  access its properties in the "Properties" panel.

- The DEVPATH environment variable allows you to use Ice for .NET
  assemblies from your <Ice installation root directory>\bin directory
  directly instead of copying them in the executable directory.

  To enable locating assemblies using DEVPATH you need to use a .NET
  configuration file for your executable. The configuration file must
  contain the following settings:

    <configuration>
      <runtime>
        <developmentMode developerInstallation="true"/>
      </runtime>
    </configuration>

  Then, add Ice bin directory to your DEVPATH:

    > set DEVPATH=<Ice installation root directory>\bin;%DEVPATH%

  For more information on DEVPATH, see:

    http://msdn.microsoft.com/en-us/library/cskzh7h6.aspx  

- Finally, you can also add the Ice for .NET assemblies to your GAC.

  To do this, open Windows Explorer and navigate to the directory
  C:\WINDOWS\assembly. Next, drag and drop (or copy and paste) the
  .NET assemblies from the bin directory into the right-hand pane to
  install them in the cache.

  You can use also gacutil from the command line to achieve the same
  result:

    > gacutil /i <library.dll>

  The gacutil tool is included with your Visual C# installation. For
  example, if you have installed Visual C# 9.0 in C:\Program Files,
  the path to gacutil is

    C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin\gacutil.exe

  Once installed in the cache, the assemblies will always be located
  correctly without having to set environment variables or copy them
  into the same directory as an executable.

  If you want line numbers for stack traces, you must also install the
  PDB (.pdb) files in the GAC. Unfortunately, you cannot do this using
  Explorer, so you have to do it from the command line. Open a command
  shell window and navigate to C:\WINDOWS\assembly\GAC_MSIL\Ice
  (assuming C:\WINDOWS is your system root). Doing a directory listing
  there, you will find a directory named @ver@.0__<UUID>, for example:

    @ver@.0__cdd571ade22f2f16

  Change to that directory (making sure that you use the correct
  version number for this release of Ice). In this directory, you will
  see the Ice.dll you installed into the GAC in the preceding
  step. Now copy the Ice.pdb file into this directory:

    > copy <path_to_ice.pdb> .

The Ice for .NET run time implements protocol compression by
dynamically loading the native library bzip2.dll from a directory in
your PATH. Ice disables the protocol compression feature if it is
unable to load the bzip2 library successfully.

This DLL is included in your Ice distribution. Therefore the Ice bin
directory must be added to your PATH:

  > set PATH=<Ice installation root directory>\bin;%PATH%

On 64-bit Windows use the following setting instead:

  > set PATH=<Ice installation root directory>\bin\x64;%PATH%

If the wrong PATH is set, the Ice run time prints a warning to the
console when it detects a bzip2.dll format mismatch during start-up


Java
----

To use Ice for Java, you must add Ice.jar to your CLASSPATH, as shown
below:

  > set CLASSPATH=<Ice installation root directory>\lib\Ice.jar;%CLASSPATH%

If you intend to use Freeze for Java, you must include Freeze.jar in
your CLASSPATH along with Ice.jar:

  > set CLASSPATH=<Ice installation root directory>\lib\Freeze.jar;%CLASSPATH%

Furthermore, to use a Freeze demo the JVM requires that the directory
containing Berkeley DB's native libraries be included in
java.library.path, therefore you must add this directory to your PATH.

  > set PATH=<Ice installation root directory>\bin;%PATH%

Ice for Java supports protocol compression using the bzip2 classes
included with ant. Compression is automatically enabled if these
classes are present in your CLASSPATH. You can either add ant.jar to
your CLASSPATH, or download only the bzip2 classes from

  http://www.kohsuke.org/bzip2/

Note that these classes are a pure Java implementation of the bzip2
algorithm and therefore add significant latency to Ice requests.


Python
------

To use Ice for Python, you must add Ice bin directory to your PATH and
set PYTHONPATH so that the Python interpreter is able to load the Ice
extension. For a 32-bit Python installation, use these settings:


  > set PATH=<Ice installation root directory>\bin;%PATH%
  > set PYTHONPATH=<Ice installation root directory>\python

For a 64-bit Python installation, use these settings instead:

  > set PATH=<Ice installation root directory>\bin\x64;%PATH%
  > set PYTHONPATH=<Ice installation root directory>\python\x64


Ruby
----

To use Ice for Ruby, you must add Ice bin directory to your PATH.

  > set PATH=<Ice installation root directory>\bin;%PATH%

You must also set RUBYLIB so that the Ruby interpreter is able to 
load the Ice extension.

 > set RUBYLIB=<Ice installation root directory>\ruby;%RUBYLIB%

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


PHP
---

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


======================================================================
Running IceGrid and Glacier2 components as services
======================================================================

An appendix in the Ice manual provides information on installing and
running the IceGrid registry, IceGrid node, and Glacier2 router as
Windows services.


======================================================================
Using the IceGrid Administrative Console
======================================================================

A Java-based graphical tool for administering IceGrid applications
is included in this distribution. The Java archive file is installed
as

<Ice installation root directory>\bin\IceGridGUI.jar

With a suitable Java installation, you can execute the application
by double-clicking on this file.


======================================================================
Demos
======================================================================

This distribution includes an archive named demos.zip that contains
sample programs for the supported languages. To build and run the
demos, you must extract this archive in the location of your choice
and follow the instructions in the README.txt file located in the
archive.


======================================================================
Binary compatibility
======================================================================

Please refer to the RELEASE_NOTES.txt file included in this
distribution for information on binary compatibility and detailed
upgrade instructions.


======================================================================
Monotonic clock
======================================================================

Ice uses the QueryPerformanceCounter Windows API function to measure
time with a monotonic clock. If you are experiencing timing or
performance issues, there are two knowledgebase articles that may be
relevant for your system:

  http://support.microsoft.com/?id=896256
  http://support.microsoft.com/?id=895980


======================================================================
Managed code in Ice for .NET
======================================================================

The main Ice for .NET assembly (Ice.dll) included in this distribution
uses unmanaged code. If you require only managed code then you can
download the Ice source distribution and build Ice for .NET in a
purely managed version. Note that the managed version of Ice for .NET
omits support for protocol compression and for signal handling in the
Ice.Application class.

You can download the source distribution at the link below:

    http://www.zeroc.com/download.html


======================================================================
Ice and the Windows Registry
======================================================================

The Ice installer adds information to the Windows registry to indicate
where it was installed. Developers can use this information to locate
the Ice files in their applications.

The registration key used by this installer is:

HKEY_LOCAL_MACHINE\Software\ZeroC\Ice @ver@

The install location is stored as a string value named 'InstallDir'.


======================================================================
Acknowledgments
======================================================================

This product includes software developed by the OpenSSL Project for
use in the OpenSSL Toolkit (http://www.openssl.org/).

This product includes cryptographic software written by Eric Young
(eay@cryptsoft.com).
