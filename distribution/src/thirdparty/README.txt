======================================================================
Third Party Packages
======================================================================

Introduction
------------

This archive contains the source code distributions, including any
source patches, for the third-party packages required to build Ice on
Windows.

This document provides instructions for applying patches and important
information about building the third-party packages. Note that you do
not need to build these packages yourself, as ZeroC supplies a Windows
installer that contains release and debug libraries for all of the
third-party dependencies. The installer is available at

  http://www.zeroc.com/download.html

If you prefer to compile the third-party packages from source code, we
recommend that you use the same Visual C++ version to build all of the
packages.

You can use an archive utility such as 7-Zip to extract the source
code packages in this distribution. 7-Zip is available for free at the
link below:

  http://www.7-zip.org

For more information about the third-party dependencies, please refer
to the links below:

STLport        http://www.stlport.org
Berkeley DB    http://www.oracle.com/database/berkeley-db/index.html
expat          http://expat.sourceforge.net
OpenSSL        http://www.openssl.org
bzip2          http://sources.redhat.com/bzip2
mcpp           http://mcpp.sourceforge.net


Table of Contents
-----------------

  1. Patches
     - bzip2
     - Berkeley DB
     - mcpp
  2. Packages
     - STLport
     - Berkeley DB
     - expat
     - OpenSSL
     - bzip2
     - mcpp


======================================================================
1. Patches
======================================================================

Applying patches requires the "patch" utility. You can download a
Windows executable from the following location:

  http://gnuwin32.sourceforge.net/packages/patch.htm

On Windows Vista or later, UAC can make it difficult to use the patch
utility unless you take extra steps. One solution is to run patch.exe
in a command window that you started with Administrator privileges
(right-click on Command Prompt in the Start menu and choose "Run as
administrator"). If running as administrator is not an option, follow
these recommendations:

  1. Do not install patch.exe in a system-protected directory such as
     C:\Program Files.

  2. Create a manifest file named patch.exe.manifest as explained at
     the link below:

     http://drupal.org/node/99903

     Place the manifest file in the same directory as patch.exe.


bzip2
-----

The bzip2-1.0.5 distribution does not directly support creating DLLs.
The file bzlib.patch in this archive contains a patch for bzlib.h that
allows bzip2 to be compiled into a DLL.

After extracting the bzip2 source distribution, change to the
top-level directory and apply the patch as shown below:

  > patch --binary -p0 bzlib.h < ..\bzip2\bzlib.patch


Berkeley DB
-----------

The file db/patch.4.8.30.17646 in this archive contains an important
fix for Berkeley DB required by Ice. 

After extracting the Berkeley DB 4.8.30 source distribution, change 
to the top-level directory and apply the patches as shown below:

 > cd db-4.8.30
 > patch --binary -p0 < ..\db\patch.db-4.8.30.17646


mcpp
----

The file mcpp/patch.mcpp.2.7.2 in this archive contains several
important fixes required by Ice. We expect that these changes will be
included in a future release of mcpp.

After extracting the mcpp source distribution, change to the top-level
directory and apply the patch as shown below:

  > cd mcpp-2.7.2
  > patch --binary -p0 < ..\mcpp\patch.mcpp.2.7.2


======================================================================
2. Packages
======================================================================


STLport
-------

STLport is only required when using Visual C++ 6.0. For installation
instructions, please refer to

  http://www.stlport.org/doc/install.html


Berkeley DB
-----------

When building the debug version of the Berkeley DB DLL (db_dll
project), you should remove the "DIAGNOSTIC" and "CONFIG_TEST" defines
and the /export:__db_assert linker option. Without these modifications,
database environments created by the debug DLL are not compatible with
environments created by the release DLL.

For installation instructions, please refer to

  http://www.oracle.com/technology/documentation/berkeley-db/db/ref/build_win/intro.html


expat
-----

Use the provided expat binary installer to install expat in the
directory of your choice. The installer includes binaries as well as
source code.


OpenSSL
-------

After extracting the OpenSSL source archive, refer to the file
INSTALL.W32 or INSTALL.W64 for build instructions.

For Visual C++ 6.0, you should use the replacement makefile included
in this archive:

  > nmake /f ..\openssl\ntdll.mak

For 64-bit builds it is also necessary to remove references to
libbufferoverflowu.lib from ms\ntdll.mak before running nmake.


bzip2
-----

If you have not already applied the patch for bzip2, please read the
"Patches" section above before continuing.

To build bzip2, change to the source directory and use the replacement
makefile included in this archive:

  > nmake /f ..\bzip2\Makefile.mak

This will build the release and debug versions of the bzip2 DLLs. If
you are using Visual C++ 6.0, first set the CPP_COMPILER environment
variable as shown below:

  > set CPP_COMPILER=VC60


mcpp
----

Follow these instructions for building mcpp:

- Change to the mcpp src directory:

  > cd mcpp-2.7.2\src

- Apply the patch for noconfig.H appropriate for your compiler from
  the noconfig directory. For example, for VS2008 you would run:

  > patch --binary -p0 < ..\noconfig\vc2008.dif

  and for C++Builder 2010 you would run:

  > patch --binary -p0 < ..\noconfig\bc59.dif

- Microsoft Visual C++: 
 
  Build the mcpp release library:

  > nmake MCPP_LIB=1 /f ..\noconfig\visualc.mak mcpplib

  To build the debug version of the library:

  > nmake MCPP_LIB=1 DEBUG=1 /f ..\noconfig\visualc.mak mcpplib

- CodeGear C++Builder: 

  Build the mcpp library:

  > make -DMCPP_LIB -f..\noconfig\borlandc.mak mcpplib
