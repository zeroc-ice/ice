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

  http://www.zeroc.com/download_beta.html

If you prefer to compile the third-party packages from source code, we
recommend that you use the same Visual C++ version to build all of the
packages.

You can use an archive utility such as 7-Zip to extract the source
code packages in this distribution. 7-Zip is available for free at the
link below:

  http://www.7-zip.org

For more information about the third-party dependencies, please refer
to the links below:

Berkeley DB    http://www.oracle.com/us/products/database/berkeley-db/overview/index.htm
expat          http://expat.sourceforge.net
OpenSSL        http://www.openssl.org
bzip2          http://www.bzip.org
mcpp           http://mcpp.sourceforge.net


Table of Contents
-----------------

  1. Patches
     - bzip2
     - mcpp
  2. Packages
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

  2. Update the manifest in patch.exe as described at:
     http://math.nist.gov/oommf/software-patchsets/patch_on_Windows7.html


bzip2
-----

The file bzip2/patch.bzip2-1.0.6 in this archive contains updates to allow
compile bzip2 as a DLL with VC and MINGW compilers

After extracting the bzip2 source distribution, change to the
top-level directory and apply the patch as shown below:

  > patch -p0 < ..\bzip2\patch.bzip2-1.0.6


mcpp
----

The file mcpp/patch.mcpp.2.7.2 in this archive contains several
important fixes required by Ice. We expect that these changes will be
included in a future release of mcpp.

After extracting the mcpp source distribution, change to the top-level
directory and apply the patch as shown below:

  > cd mcpp-2.7.2
  > patch -p0 < ..\mcpp\patch.mcpp.2.7.2


======================================================================
2. Packages
======================================================================


Berkeley DB
-----------

When building the debug version of the Berkeley DB DLL, you should
remove the "DIAGNOSTIC" and "CONFIG_TEST" defines and the
/export:__db_assert linker option. Without these modifications,
database environments created by the debug DLL are not compatible with
environments created by the release DLL.

- Java

Berkeley DB for Java is built using Visual Studio 2010. This is the
same compiler used to build Oracle Java 7 JDK and Oracle Java 8 JDK.

The DB VC100 DLL built by Ice uses a "_vc100" suffix. Open Berkeley_DB_vs2010
and in the db project properties update "Target Name" and add the
_vc100 suffix.

To avoid libdb53_vc100.dll depending on the VC100 C++ run time, remove
all .cpp files from the db project.

Edit db_java.vcxproj and replace "-target 1.5" by "-target 1.7" so
that the resulting db.jar will use the Java 1.7 source format.

Then follow the Windows build instructions included in the source
distribution:

docs/installation/build_win.html


expat
-----

Use the provided expat binary installer to install expat in the
directory of your choice. The installer includes binaries as well as
source code.


OpenSSL
-------

- Microsoft Visual Studio

After extracting the OpenSSL source archive, refer to the file
INSTALL.W32 or INSTALL.W64 for build instructions.


bzip2
-----

If you have not already applied the patch for bzip2, please read the
"Patches" section above before continuing.

- Microsoft Visual Studio

  1) Open a Visual Studio command prompt and set the
     CPP_COMPILER variable to VC100, VC110 or VC120

     > set CPP_COMPILER=VC100

  2) Change to the source directory and build:

     > nmake /f Makefile.mak

- MinGW

  1) Open a Windows command prompt

  2) Add MinGW from the Ruby Development Kit to your PATH:

     > C:\DevKit-mingw64-64-4.7.2\devkitvars.bat

  3) Change to the bzip2 source directory and use the replacement
     makefile included in this archive:

     > cd bzip2-1.0.6
     > make -f Makefile-libbz2_so


mcpp
----

Follow these instructions for building mcpp:

- Microsoft Visual Studio:

  1) Change to the mcpp src directory:

     > cd mcpp-2.7.2\src

  2) Apply the patch for noconfig.H appropriate for your compiler from
     the noconfig directory. For example, for VS2010 or VS2012 you
     would run:

     > patch --binary -p0 < ..\noconfig\vc2010.dif

  3) Build the mcpp release library:

     > nmake MCPP_LIB=1 /f ..\noconfig\visualc.mak mcpplib

  4) To build the debug version of the library:

     > nmake MCPP_LIB=1 DEBUG=1 /f ..\noconfig\visualc.mak mcpplib

- MinGW

  1) Open a Windows command prompt and change to the mcpp src
     directory:

     > cd mcpp-2.7.2\src

  2) Apply the build patch:

     > patch --binary -p0 < ..\noconfig\mingw345.dif

  3) Add MinGW from the Ruby Development Kit to your PATH:

     > C:\DevKit-mingw64-64-4.7.2\devkitvars.bat

  4) Build the mcpp library:

     > MCPP_LIB=1 make -f ../noconfig/mingw.mak mcpplib
