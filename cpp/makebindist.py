#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, string

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print
    print "If no tag is specified, HEAD is used."

def getIceVersion(file):
    config = open(file, "r")
    return re.search("ICE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

def getIceSoVersion(file):
    config = open(file, "r")
    intVersion = int(re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1))
    majorVersion = intVersion / 10000
    minorVersion = intVersion / 100 - 100 * majorVersion
    return '%d' % (majorVersion * 10 + minorVersion)

#
# Check arguments
#
tag = "-rHEAD"
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        tag = "-r" + x

if not os.environ.has_key("ICE_HOME"):
    print "The ICE_HOME environment variable is not set."
    sys.exit(1)

#
# Get platform.
#
platform = ""
if sys.platform.startswith("win") or sys.platform.startswith("cygwin"):
    platform = "win32"
elif sys.platform.startswith("linux"):
    platform = "linux"
elif sys.platform.startswith("sunos"):
    platform = "solaris"
elif sys.platform.startswith("hp"):
    platform = "hpux"
elif sys.platform.startswith("darwin"):
    platform = "macosx"
elif sys.platform.startswith("aix"):
    platform = "aix"
else:
    print "unknown platform (" + sys.platform + ")!"
    sys.exit(1)

#
# Remove any existing distribution directory and create a new one.
#
distdir = "bindist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)
cwd = os.getcwd()

#
# Export Config.h from CVS.
#
if platform == "aix":
    os.environ["LIBPATH"] = ""
    os.system("cvs -d cvsint.mutablerealms.com:/home/cvsroot export " + tag + " ice/include/IceUtil/Config.h")
else:
    os.system("cvs -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " ice/include/IceUtil/Config.h")

#
# Get Ice version.
#
version = getIceVersion("ice/include/IceUtil/Config.h")
intVer = getIceSoVersion("ice/include/IceUtil/Config.h")

#
# Verify Ice version in CVS export matches the one in ICE_HOME.
#
version2 = getIceVersion(os.environ["ICE_HOME"] + "/include/IceUtil/Config.h")

shutil.rmtree("ice")

if version != version2:
    print sys.argv[0] + ": the CVS version (" + version + ") does not match ICE_HOME (" + version2 + ")"
    sys.exit(1)

icever = "Ice-" + version
os.mkdir(icever)

#
# Copy executables and libraries.
#
icehome = os.environ["ICE_HOME"]
executables = [ ]
libraries = [ ]
symlinks = 0
debug = ""
strip = 0
if platform == "win32":
    winver = version.replace(".", "")
    if not os.path.exists(icehome + "/bin/iceutil" + winver + ".dll"):
        debug = "d"
    executables = [ \
        "icecpp.exe",\
        "slice2cs.exe",\
        "slice2docbook.exe",\
        "slice2freezej.exe",\
        "slice2java.exe",\
        "slice2py.exe",\
        "iceutil" + winver + debug + ".dll",\
        "slice" + winver + debug + ".dll",\
    ]
    libraries = [ \
    ]
else:
    executables = [ \
        "icecpp",\
        "slice2cs",\
        "slice2docbook",\
        "slice2freezej",\
        "slice2java",\
        "slice2py",\
    ]
    libraries = [ \
        "libIceUtil",\
        "libSlice",\
    ]
    if platform != "aix":
	symlinks = 1
    strip = 1

bindir = icever + "/bin"
libdir = icever + "/lib"
os.mkdir(bindir)
os.mkdir(libdir)

for x in executables:
    shutil.copyfile(icehome + "/bin/" + x, bindir + "/" + x)

if symlinks:
    for so in libraries:
        if platform == "hpux":
            soBase = so
            soLib = so + ".sl"
        else:
            soBase = so + ".so"
            soLib = soBase

        if platform == "macosx":
	    soVer = so + '.' + version + ".dylib"
            soInt = so + '.' + intVer + ".dylib"
            soLib = so + ".dylib"
        else:	
            soVer = soBase + '.' + version
            soInt = soBase + '.' + intVer

        shutil.copyfile(icehome + "/lib/" + soVer, libdir + "/" + soVer)
        os.chdir(libdir)
        os.symlink(soVer, soInt)
        os.symlink(soInt, soLib)
        os.chdir(cwd)
else:
    for lib in libraries:
	if platform == "aix":
	    shutil.copyfile(icehome + "/lib/" + lib + ".a", libdir + "/" + lib + ".a")
	else:
	    shutil.copyfile(icehome + "/lib/" + lib, libdir + "/" + lib)

if platform == "win32":
    if not os.environ["WINDIR"]:
        print "WINDIR environment variable not set"
        sys.exit(1)

    dlls = [ \
        "msvcp70" + debug + ".dll",\
        "msvcr70" + debug + ".dll",\
    ]
    for dll in dlls:
        dllpath = os.environ["WINDIR"] + "/system32/" + dll
        if not os.path.exists(dllpath):
            print "VC++.NET runtime DLL " + dllpath + " not found"
            sys.exit(1)

        shutil.copyfile(dllpath, bindir + "/" + dll)

if strip:

    stripOpts=""
    if platform == "macosx":
	stripOpts="-x"

    for x in executables:
        os.system("strip " + stripOpts + " " + bindir + "/" + x)
        os.chmod(bindir + "/" + x, 0755)
    for x in libraries:
        if platform == "hpux":
            soLib = x + ".sl"
	elif platform == "macosx":
            soLib = x + ".dylib"
	elif platform == "aix":
            soLib = x + ".a"
        else:
            soLib = x + ".so"
        os.system("strip " + stripOpts + " " + libdir + "/" + soLib)

#
# Create binary archives.
#
os.system("tar cvf " + icever + "-bin-" + platform + ".tar " + icever)
os.system("gzip -9 " + icever + "-bin-" + platform + ".tar")
os.system("zip -9ry " + icever + "-bin-" + platform + ".zip " + icever)

#
# Copy files (README, etc.).
#

#
# Done.
#
shutil.rmtree(icever)
