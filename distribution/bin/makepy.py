#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, glob, fnmatch, string, re
from stat import *

#
# NOTE: See lib/DistUtils.py for default third-party locations and
# languages to be built on each platform.
#

iceVersion = "3.6.0"

distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(distDir, "lib"))
import DistUtils

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] tag"
    print
    print "Options:"
    print "-c    Clean previous build"
    print "-h    Show this message."
    print "-v    Be verbose."

#
# Check arguments
#
verbose = 0
forceclean = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-v":
        verbose = 1
    elif x == "-c":
        forceclean = 1
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        if not x in platform.languages:

            print sys.argv[0] + ": language `" + x + "' not supported on this platform"
            print
            usage()
            sys.exit(1)
        buildLanguages.append(x)

if verbose:
    quiet = "v"
else:
    quiet = ""

#
# Ensure the script is being run from the dist directory.
#
cwd = os.getcwd()
if not os.path.exists(os.path.join(distDir, "src", "windows", "LICENSE.rtf")):
    print sys.argv[0] + ": you must run makebindist.py from the dist-" + iceVersion + " directory created by makedist.py"
    sys.exit(1)

print "Building Ice " + iceVersion

#
# Ensure that the source archive or directory exists and create the build directory.
#
buildRootDir = os.path.join(distDir, "..", "build-py-" + iceVersion)
srcDir = os.path.join(buildRootDir, "Ice-" + iceVersion + "-src")

if forceclean or not os.path.exists(srcDir):
    if os.path.exists(buildRootDir):
        print "Removing previous build from build-py-" + iceVersion + "...",
        sys.stdout.flush()
        shutil.rmtree(buildRootDir)
        print "ok"
    os.mkdir(buildRootDir)

    if not os.path.exists(os.path.join(cwd, "Ice-" + iceVersion + ".tar.gz")):
        print sys.argv[0] + ": cannot find " + os.path.join(cwd, "Ice-" + iceVersion + ".tar.gz")
        sys.exit(1)

    print "Unpacking ./Ice-" + iceVersion + ".tar.gz ...",
    sys.stdout.flush()
    os.chdir(buildRootDir)
    if os.system("gunzip -c " + os.path.join(cwd, "Ice-" + iceVersion + ".tar.gz") + " | tar x" + quiet + "f -"):
        print sys.argv[0] + ": failed to unpack ./Ice-" + iceVersion + ".tar.gz"
        sys.exit(1)
    os.rename("Ice-" + iceVersion, srcDir)

    os.chdir(cwd)
    print "ok"

    buildDirs = [ "cpp/src/IceUtil", "cpp/src/Slice", "cpp/src/slice2py", "cpp/src/slice2cpp", "py/python" ]
    for l in buildDirs:
        print "building sources in " + os.path.join(srcDir, l)
        os.chdir(os.path.join(srcDir, l))
        if os.system("make") != 0:
            print sys.argv[0] + ": `" + l + "' build failed"
            os.chdir(cwd)
            sys.exit(1)

    sliceBuildDirs = [ "cpp/src/Ice", "cpp/src/IceSSL", "cpp/src/IceDiscovery", "cpp/src/IceLocatorDiscovery" ]
    for l in sliceBuildDirs:
        print "building sources in " + os.path.join(srcDir, l)
        os.chdir(os.path.join(srcDir, l))
        if os.system("make compile-slice") != 0:
            print sys.argv[0] + ": `" + l + "' build failed"
            os.chdir(cwd)
            sys.exit(1)

    os.chdir(cwd)
    print
    print "============= Finished building sources ============="
    print

os.chdir(buildRootDir)
#thirdPartyPackage = "ThirdParty-Sources-" + iceVersion
thirdPartyPackage = "ThirdParty-Sources-3.6b"
downloadUrl = "http://www.zeroc.com/download/Ice/3.6/"

if not os.path.isfile(os.path.expanduser("~/Downloads/%s.tar.gz" % thirdPartyPackage)):
    if os.system(os.path.expanduser("cd ~/Downloads && wget http://www.zeroc.com/download/Ice/3.6/%s.tar.gz" % thirdPartyPackage)) != 0:
        print sys.argv[0] + ": downloading the third parties tar.gz failed"
        os.chdir(cwd)
        sys.exit(1)

if not os.path.isfile(os.path.expanduser("~/Downloads/%s.zip" % thirdPartyPackage)):
    if os.system(os.path.expanduser("cd ~/Downloads && wget http://www.zeroc.com/download/Ice/3.6/%s.zip" % thirdPartyPackage)) != 0:
        print sys.argv[0] + ": downloading the third parties zip failed"
        os.chdir(cwd)
        sys.exit(1)

if os.system(os.path.expanduser(
           "rm -rf %(thirdParty)s && tar zxf ~/Downloads/%(thirdParty)s.tar.gz && cd %(thirdParty)s && tar zxf mcpp-2.7.2.tar.gz && "
           "cd mcpp-2.7.2 && patch -p0 < ../mcpp/patch.mcpp.2.7.2" % {"thirdParty": thirdPartyPackage})) != 0:
    print sys.argv[0] + ": extracting mcpp failed"
    os.chdir(cwd)
    sys.exit(1)

if os.system(os.path.expanduser(
           "rm -rf zip && unzip ~/Downloads/%(thirdParty)s.zip -d zip && cd zip/%(thirdParty)s && tar zxf bzip2-1.0.6.tar.gz"
           % {"thirdParty": thirdPartyPackage})) != 0:
    print sys.argv[0] + ": extracting bzip failed"
    os.chdir(cwd)
    sys.exit(1)


os.chdir(cwd)

#
# Extract all the relevant pieces from the source tree into the pyDir.
#
pyDir = os.path.join(buildRootDir, "Ice-" + iceVersion + "-py")
pyDirExt = os.path.join(pyDir, "src")
pyDirLib = os.path.join(pyDir, "lib")
if os.path.exists(pyDir):
    print "Removing previous build from " + pyDir + " ...",
    sys.stdout.flush()
    shutil.rmtree(pyDir)
    print "ok"

os.mkdir(pyDir)
os.mkdir(pyDirExt)
os.mkdir(pyDirLib)
DistUtils.copy(os.path.join(srcDir, "ICE_LICENSE"), pyDir, verbose)
DistUtils.copy(os.path.join(srcDir, "LICENSE"), pyDir, verbose)

DistUtils.copy(os.path.join(distDir, "src/py/MANIFEST.in"), pyDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/py/setup.py"), pyDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/py/setup.cfg"), pyDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/py/README.rst"), pyDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/py/slice2py.py"), pyDirLib, verbose)
DistUtils.copyMatchingFiles(os.path.join(srcDir, "py/modules/IcePy"), pyDirExt, ["*.cpp", "*.h"], False, verbose)
DistUtils.copyMatchingFiles(os.path.join(srcDir, "py/python"), pyDirLib, ["*.py"], False, verbose)
for d in os.listdir(os.path.join(srcDir, "py/python")):
    DistUtils.copyMatchingFiles(os.path.join(srcDir, "py/python", d), os.path.join(pyDirLib, d), ["*.py"], False, verbose)

pyDirIce = os.path.join(pyDir, "src", "ice")
os.mkdir(pyDirIce)

#
# Extract the relevant portions of the Ice C++ source tree into the package.
#
copyDirs = [ "cpp/src/IceUtil", "cpp/src/Slice", "cpp/src/Ice", "cpp/include/IceUtil",
	     "cpp/include/Slice", "cpp/include/Ice", "cpp/src/IceSSL", "cpp/include/IceSSL",
	     "cpp/include/IceDiscovery", "cpp/src/IceDiscovery", "cpp/include/IceLocatorDiscovery", "cpp/src/IceLocatorDiscovery" ]
for d in copyDirs:
    DistUtils.copyMatchingFiles(os.path.join(srcDir, d), os.path.join(pyDirIce, d), ["*.cpp", "*.h"], False, verbose)

for d in os.listdir(os.path.join(srcDir, "slice")):
    DistUtils.copyMatchingFiles(os.path.join(srcDir, "slice", d), os.path.join(pyDirLib, "slice", d), ["*"], False, verbose)

os.unlink(os.path.join(pyDirExt, "ice/cpp/src/Ice/DLLMain.cpp"))
os.unlink(os.path.join(pyDirExt, "ice/cpp/src/Ice/Service.cpp"))
os.unlink(os.path.join(pyDirExt, "ice/cpp/src/Ice/Application.cpp"))

for d in ["main.c", "directive.c", "eval.c", "expand.c", "mbchar.c", "support.c", "system.c", "internal.H", "mcpp_out.h", "mcpp_lib.h", "system.H", "configed.H"]:
    DistUtils.copy("%s/%s/mcpp-2.7.2/src/%s" % (buildRootDir, thirdPartyPackage, d), os.path.join(pyDirIce, "mcpp", d))
DistUtils.copyMatchingFiles(os.path.join(distDir, "src/py/src/mcpp", ), os.path.join(pyDirIce, "mcpp"), ["*"], False, verbose)

DistUtils.copy(os.path.join(distDir, "src/unix/MCPP_LICENSE"), pyDirIce)

for d in ['blocksort.c', 'bzlib.c', 'compress.c','crctable.c','decompress.c','huffman.c','randtable.c','bzlib.h','bzlib_private.h']:
    DistUtils.copy("%s/zip/%s/bzip2-1.0.6/%s" % (buildRootDir, thirdPartyPackage, d), os.path.join(pyDirIce, "bzip2", d))
DistUtils.copy("%s/zip/%s/bzip2-1.0.6/LICENSE" % (buildRootDir, thirdPartyPackage), os.path.join(pyDirIce , "BZIP_LICENSE"))

os.chdir(pyDir)
pyFile = ""
if os.system("python setup.py sdist") != 0:
    print sys.argv[0] + ": building the source dist"
    os.chdir(cwd)
    sys.exit(1)
os.chdir(cwd)
pyFile = os.path.join("dist", "zeroc-ice-" + iceVersion + ".tar.gz")
DistUtils.copy(os.path.join(pyDir, pyFile), ".", verbose)

print
print "============= Finished building " + pyFile + " ============="
print
