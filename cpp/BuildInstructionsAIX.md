# Building Ice for C++ on AIX

This document provides step-by-step instructions for building Ice for C++
from sources on AIX.

## Unsupported Platform

AIX is currently an unsupported platform. Only minimal testing was performed
on AIX, and there is no guarantee future releases of Ice will build or run
on this platform.

## Recommended Operating System and C++ Compiler

 - AIX 6.1, AIX 7.1
 - IBM XL C/C++ 12.1 with latest Fix Pack
 - 32 bit or 64 bit

## Build Pre-Requisites

Before building Ice, you need to install or build a number of third-party
packages that Ice depends on, and install some build tools.

### Third-Party Packages

Ice depends on several open-source packages: bzip2 1.0, expat 2.x, LMDB 0.9.x, 
mcpp 2.7.2 (+patches) and OpenSSL.

OpenSSL is a system package on AIX. RPM packages for bzip2 and expat are
available in the [AIX Toolbox for Linux Applications][1]. You can install
them as follows:

```
# As root
rpm -i ftp://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/bzip2/bzip2-1.0.5-3.aix5.3.ppc.rpm
rpm -i ftp://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/expat/expat-2.0.1-2.aix5.3.ppc.rpm
rpm -i ftp://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/expat/expat-devel-2.0.1-2.aix5.3.ppc.rpm

```

LMDB and mcpp must be built from sources, as described below. 

### Build Tools

You need GNU make (gmake) to build Ice:
```
# As root
rpm -i ftp://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/make/make-4.1-2.aix6.1.ppc.rpm
```

The preferred way to retrieve the Ice and mcpp source distributions is with
git. A RPM for git is available from [bullfreeware.com][2].

### Building mcpp from Sources

If building 64-bit binaries, set `OBJECT_MODE` to 64:
```
export OBJECT_MODE=64
```
Otherwise, leave `OBJECT_MODE` unset.

Then clone the zeroc-ice/mcpp repository and build with gmake:
```
git clone https://github.com/zeroc-ice/mcpp.git
cd mcpp
gmake
```

This build creates a static library, `lib/libmcpp.a`. `libmcpp.a` is used
only when building Ice from sources, and does not need to be installed.

### Building LMDB from Sources

TBD

## Building Ice for C++

### Clone zeroc-ice/ice

```
git clone https://github.com/zeroc-ice/ice.git
cd ice
```

### Configure

Edit `config/Make.rules` to establish your build configuration. The comments
in the file provide more information.

Make sure to set `LMDB_HOME` to your LMDB installation directory and 
`MCPP_HOME` to your mcpp build directory (for example `$(HOME)/builds/mcpp`).

### Build

Use the gmake `-j` option to speed up the build:

```
cd cpp
gmake -j8
```

This builds the Ice core libraries, services, and tests.

### Install

```
gmake install
```

This installs Ice for C++ in the directory specified by `prefix` in
`config/Make.rules`. By default, all Ice binaries embed the library path
`prefix/lib:/usr/lib`.

After installation, make sure that the `prefix/bin` directory is in your `PATH`.

### Run the Test Suite (Optional)

Python is required to run the test suite. You can install Python as follows:
```
# As root
rpm -i ftp://ftp.software.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/python/python-2.7.10-1.aix6.1.ppc.rpm
rpm -i ftp://ftp.software.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/python/python-tools-2.7.10-1.aix6.1.ppc.rpm
rpm -i ftp://ftp.software.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/python/python-devel-2.7.10-1.aix6.1.ppc.rpm
```

Additionally, the Glacier2 tests require the Python module `passlib`, available
from the Python Package Index:
```
# As root
wget https://bootstrap.pypa.io/get-pip.py
python get-pip.py
export PATH=/opt/freeware/bin:$PATH
pip install passlib
```

Then run the test suite with:
```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: http://www-03.ibm.com/systems/power/software/aix/linux
[2]: http://www.bullfreeware.com
