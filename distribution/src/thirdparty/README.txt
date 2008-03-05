======================================================================
Introduction
======================================================================

This archive contains the source code distributions, including any
source patches, for the third-party packages required to build Ice on
Windows.

This document provides instructions for applying patches and important
information about building the third-party packages. Note that you do
not need to build these packages yourself, as ZeroC supplies a Windows
installer for each supported compiler that contains release and debug
libraries for all of the third-party dependencies. The installer is
available at

  http://www.zeroc.com/download.html

If you prefer to compile the third-party packages from source code, we
recommend that you use the same Visual C++ version to build all of the
packages. You will need a utility such as WinZip to extract the
source code distributions.

For more information about the third-party dependencies, please refer
to the links below:

STLport        http://www.stlport.org
Berkeley DB    http://www.oracle.com/database/berkeley-db/index.html
expat          http://expat.sourceforge.net
OpenSSL        http://www.openssl.org
bzip2          http://sources.redhat.com/bzip2
mcpp           http://mcpp.sourceforge.net


======================================================================
Patches
======================================================================

Applying patches requires the "patch" utility. You can download a
Windows executable from the following location:

  http://gnuwin32.sourceforge.net/packages/patch.htm


bzip2
-----

The bzip2-1.0.4 distribution does not directly support creating DLLs.
The file bzlib.patch in this archive contains a patch for bzlib.h that
allows bzip2 to be compiled into a DLL.

After extracting the bzip2 source distribution, change to the
top-level directory and apply the patch as shown below:

  > patch -p0 bzlib.h < ..\bzlib.patch


mcpp
----

The file mcpp-2.4.6.patch in this archive contains several important
fixes required by Ice. We expect that these changes will be included
in a future release of mcpp.

After extracting the mcpp source distribution, change to the top-level
directory and apply the patch as shown below:

  > patch -p0 < mcpp-2.6.4.patch


======================================================================
Packages
======================================================================


STLport
-------

STLport is only required when using Visual C++ 6.0. For installation
instructions, please refer to

  http://www.stlport.org/doc/install.html


Berkeley DB
-----------

Users of Visual C++ 6.0 must configure Visual Studio to use STLport
before building Berkeley DB:

- In the Visual C++ 6.0 IDE, choose Tools->Options->Directories

- Select "Include files"

- Add the include directory for STLport first in the list. (Note that
  you must add the "include\stlport" directory, not just "include".)

- Select "Library files"

- Add the lib directory for STLport.

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
INSTALL.W32 for build instructions.


bzip2
-----

If you have not already applied the patch for bzip2, please read the
"Patches" section above before continuing.

To build bzip2, change to the source directory and use the replacement
makefile included in this archive:

  > nmake /f ..\Makefile.bzip2

This will build the release and debug versions of the bzip2 DLLs.


mcpp
----

If you have not already applied the patch for mcpp, please read the
"Patches" section above before continuing.

Follow these instructions for building mcpp:

- Change to the mcpp src directory:

  > cd mcpp-2.6.4\src

- Apply the patch for noconfig.H appropriate for your compiler from
  the noconfig directory. For example, for VS2005 you would run:

  > patch -p0 < ..\noconfig\vc2005.def

- Build the mcpp release library:

  > nmake MCPP_LIB=1 /f ..\nonconfig\visualc.mak mcpplib

  To build the debug version of the library:

  > nmake MCPP_LIB=1 DEBUG=1 /f ..\nonconfig\visualc.mak mcpplib
