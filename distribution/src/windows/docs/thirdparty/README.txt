Ice @ver@ Third Party Packages
-----------------------------

This distribution contains the development kit (including binaries)
for the open-source packages used by Ice @ver@ on Windows. Its primary
purpose is to simplify building Ice @ver@ from sources.

The following versions of the third party software were used to
create this package:

 - Berkeley DB 4.8.24
 - OpenSSL 0.9.8l
 - Expat 2.0.1
 - Bzip2 1.0.5
 - MCPP 2.7.2
 - Qt 4.5.3
 - STLport 4.6.2
 - JGoodies Forms 1.2.1
 - JGoodies Looks 2.3.0

This distribution provides:

 - Header files for Berkeley DB, OpenSSL, Expat, Bzip2, Qt and STLport
   in the include directory.

 - Import libraries for Berkeley DB, OpenSSL, Expat, Bzip2, MCPP and
   Qt built using Visual Studio 2008 SP1, in the lib and lib\x64
   directories.

 - Import libraries for Berkeley DB, OpenSSL, Expat, Bzip2 and MCPP
   built using CodeGear C++Builder 2010, in the lib\bcc10 directory. 

 - Import libraries for OpenSSL, Bzip2 and STLport built using Visual
   C++ 6.0 in the lib\vc6 directory. 

 - DLLs and exes for Berkeley DB, OpenSSL, Expat, Bzip2 and Qt built
   using Visual Studio 2008 SP1 in the bin and bin\x64 directories.

 - DLL and exes for Berkeley DB built using CodeGear C++Builder 2010
   in the bin/bcc10 directory. For other third-party DLLs, the Visual
   Studio 2008 DLLs are used, with just the import libraries generated
   for C++Builder 2010.

 - DLLs for OpenSSL, Bzip2 and STLport built using Visual C++ 6.0 in 
   the bin directory.

 - JAR files for JGoodies Looks and JGoodies Forms in lib directory.
