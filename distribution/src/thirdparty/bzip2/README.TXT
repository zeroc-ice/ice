The bzip2-1.0.3 distribution does not directly support creating DLLs. In
order to re-create the DLL included in the Ice distribution, apply the
patch bzlib.patch to the bzlib.h header and run 'nmake /f Makefile.mak'
using the supplied Makefile.mak file. This will build the release and debug
versions of the bzip2 DLLs.
