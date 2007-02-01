#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, string, glob

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-v    Be verbose."
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

def copyLibrary(src, dst, name, python):
    global platform, version, intVer

    if platform == "hpux":
        soBase = name
        soLib = name + ".sl"
    else:
        soBase = name + ".so"
        soLib = soBase

    if not python and platform == "macosx":
        soVer = name + '.' + version + ".dylib"
        soInt = name + '.' + intVer + ".dylib"
        soLib = name + ".dylib"
    else:       
        soVer = soBase + '.' + version
        soInt = soBase + '.' + intVer

    cwd = os.getcwd()
    shutil.copyfile(src + "/" + soVer, dst + "/" + soVer)
    os.chdir(dst)
    os.symlink(soVer, soInt)
    os.symlink(soInt, soLib)
    os.chdir(cwd)

#
# Check arguments
#
tag = "-rHEAD"
verbose = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-v":
        verbose = 1
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

if not os.path.exists(os.path.join("python", "Ice_BuiltinSequences_ice.py")):
    print "makebindist.py must be run in a compiled IcePy tree"
    sys.exit(1)

#
# Save the current directory.
#
topdir = os.getcwd()

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
# Export sources from CVS.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
if platform == "aix":
    os.environ["LIBPATH"] = ""
    os.system("cvs " + quiet + " -d cvsint.zeroc.com:/home/cvsroot export " + tag +
              " ice/slice ice/include/IceUtil/Config.h icepy")
else:
    os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
              " ice/slice ice/include/IceUtil/Config.h icepy")

#
# Get Ice version.
#
version = getIceVersion("ice/include/IceUtil/Config.h")
intVer = getIceSoVersion("ice/include/IceUtil/Config.h")

#
# Verify Ice version in CVS export matches the one in ICE_HOME.
#
version2 = getIceVersion(os.environ["ICE_HOME"] + "/include/IceUtil/Config.h")
if version != version2:
    print sys.argv[0] + ": the CVS version (" + version + ") does not match ICE_HOME (" + version2 + ")"
    sys.exit(1)

#
# Copy Slice directories.
#
print "Copying Slice directories..."
slicedirs = [\
    "Glacier2",\
    "Ice",\
    "IceBox",\
    "IceGrid",\
    "IcePatch2",\
    "IceStorm",\
]
os.mkdir(os.path.join("icepy", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icepy", "slice", x), 1)
for x in glob.glob(os.path.join("ice", "config", "Make.rules.*")):
    shutil.copyfile(x, os.path.join("icepy", "config", os.path.basename(x)))

#
# Translate Slice files.
#
print "Translating..."
cwd = os.getcwd()
os.chdir(os.path.join("icepy", "python"))
if verbose:
    quiet = ""
else:
    quiet = " -s"
os.system("gmake" + quiet)
os.chdir(cwd)

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icepy", "makedist.py"), \
    os.path.join("icepy", "makebindist.py"), \
    os.path.join("icepy", "all.dsw"), \
    os.path.join("icepy", "Makefile"), \
    os.path.join("icepy", "config", "Make.rules"), \
    os.path.join("icepy", "python", "Makefile"), \
    os.path.join("icepy", "python", "generate.mak"), \
    os.path.join("icepy", "python", "Python.dsp"), \
    os.path.join("icepy", "bin", ".dummy"), \
    ]
for x in filesToRemove:
    os.remove(x)
shutil.rmtree(os.path.join("icepy", "modules"))

#
# Copy executables and libraries.
#
print "Copying executables and libraries..."
icehome = os.environ["ICE_HOME"]
executables = [ ]
libraries = [ ]
symlinks = 0
debug = ""
strip = 1
iceExecutables = [ \
    "icecpp",\
    "slice2py",\
]
iceLibraries = [ \
    "libIceUtil",\
    "libSlice",\
    "libIce",\
    "libIceSSL",\
    "libIceXML",\
]
pyLibraries = [ \
    "IcePy",\
]
if platform != "aix":
    symlinks = 1

bindir = os.path.join("icepy", "bin")
libdir = os.path.join("icepy", "lib")

for x in iceExecutables:
    shutil.copyfile(icehome + "/bin/" + x, bindir + "/" + x)

if symlinks:
    for so in iceLibraries:
        copyLibrary(icehome + "/lib", libdir, so, 0)

    for so in pyLibraries:
        copyLibrary(topdir + "/lib", libdir, so, 1)
else:
    for lib in iceLibraries:
        if platform == "aix":
            shutil.copyfile(icehome + "/lib/" + lib + ".a", libdir + "/" + lib + ".a")
        else:
            shutil.copyfile(icehome + "/lib/" + lib, libdir + "/" + lib)
    for lib in pyLibraries:
        if platform == "aix":
            shutil.copyfile("lib/" + lib + ".a", libdir + "/" + lib + ".a")
        else:
            shutil.copyfile(topdir + "/lib", libdir + "/" + lib)

if strip:

    stripOpts=""
    if platform == "macosx":
        stripOpts="-x"

    for x in iceExecutables:
        os.system("strip " + stripOpts + " " + bindir + "/" + x)
        os.chmod(bindir + "/" + x, 0755)
    for x in iceLibraries:
        if platform == "hpux":
            soLib = x + ".sl"
        elif platform == "macosx":
            soLib = x + ".dylib"
        elif platform == "aix":
            soLib = x + ".a"
        else:
            soLib = x + ".so"
        os.system("strip " + stripOpts + " " + libdir + "/" + soLib)
    for x in pyLibraries:
        if platform == "hpux":
            soLib = x + ".sl"
        elif platform == "aix":
            soLib = x + ".a"
        else:
            soLib = x + ".so"
        os.system("strip " + stripOpts + " " + libdir + "/" + soLib)


#
# Create binary archives.
#
print "Creating distribution..."
icever = "IcePy-" + version
os.rename("icepy", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("tar c" + quiet + "f " + icever + "-bin-" + platform + ".tar " + icever)
os.system("gzip -9 " + icever + "-bin-" + platform + ".tar")
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9 -r -y " + quiet + " " + icever + "-bin-" + platform + ".zip " + icever)

#
# Copy files (README, etc.).
#

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("ice")
print "Done."
