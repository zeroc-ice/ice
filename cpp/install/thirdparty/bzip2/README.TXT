The bzip2-1.0.2 distribution does not directly support creating DLLs. Apply the
bzlib.patch to the bzlib.h header file and use the Makefile.mak file in this
directory to build the debug and release DLLs. The makefile creates both
versions of the libraries in one run.
