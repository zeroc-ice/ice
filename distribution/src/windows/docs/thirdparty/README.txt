Ice 3.6.0 Third Party Packages
-----------------------------

This distribution contains the development kit (including binaries)
for the open-source packages used by Ice 3.6.0 on Windows. Its primary
purpose is to simplify building Ice 3.6.0 from sources.

The following versions of the third party software were used to
create this package:

 - Berkeley 5.3.28
 - OpenSSL 1.0.2
 - Expat 2.1.0
 - Bzip2 1.0.6
 - MCPP 2.7.2

This distribution provides:

 - Header files for Berkeley DB, OpenSSL, Expat and Bzip2 in the 
   include directory.

 - Import libraries for Berkeley DB, OpenSSL, Expat, Bzip2 and 
   MCPP built using Visual Studio 2013, in the lib and lib\x64 
   directories.

 - Import libraries for Berkeley DB, OpenSSL, Expat, Bzip2 and 
   MCPP built using Visual Studio 2012, in the lib\vc110 and 
   lib\vc110\x64 directories.

 - Import libraries for Bzip2 and MCPP built using Visual Studio
   2010, in the lib\vc100 and lib\vc100\x64 directories.

 - Static library for MCPP built using MinGW 4.7.3, in the 
   lib\mingw directory.

 - Static library for MCPP built using MinGW 4.7.2, in the 
   lib\mingw\x64 directory.

 - DLLs and exes for Berkeley DB, OpenSSL, Expat and Bzip2 built
   using Visual Studio 2013 in the bin and bin\x64 directories.

 - DLLs and exes for Berkeley DB, OpenSSL, Expat and Bzip2 built
   using Visual Studio 2012 in the bin\vc110 and bin\vc110\x64 
   directories.

 - DLLs for Bzip2 built using Visual Studio 2010 in the bin and
   bin\x64 directories. These files have a "_vc100" suffix.

 - DLL for Bzip2 built using MinGW 4.7.3 in the bin directory.
   This file has a "_mingw" suffix.

 - DLL for Bzip2 built using MinGW 4.7.2 in the bin\x64 directory.
   This file has a "_mingw" suffix.

 - JAR file for Berkeley DB in the lib directory.


Source Code
-----------

The source distributions of Berkeley DB, bzip2/libbzip2, OpenSSL,
Expat and MCPP used to build this distribution can be downloaded
at no cost from

  https://www.zeroc.com/download.html
