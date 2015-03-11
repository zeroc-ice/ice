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

print "Building Ice " + iceVersion + " gem"

#
# Ensure that the source archive or directory exists and create the build directory.
#
buildRootDir = os.path.join(distDir, "..", os.path.join("build-gem-" + iceVersion))
srcDir = os.path.join(buildRootDir, "Ice-" + iceVersion + "-src")

if forceclean or not os.path.exists(srcDir):
    if os.path.exists(buildRootDir):
        print "Removing previous build from " + os.path.join("build-gem-" + iceVersion) + "...",
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

    buildDirs = [ "cpp/src/IceUtil", "cpp/src/Slice", "cpp/src/slice2rb", "cpp/src/slice2cpp", "rb/ruby" ]
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

def getGemVersion(file):
    for line in open('ice.gemspec'):
        if line.find('s.version') != -1:
            i = line.find("'")
            line = line[i+1:]
            end = line.find("'")
            return line[:end]
    return None

copyFiles = []

#
# Extract all the relevant pieces from the source tree into the gemDir.
#
dirName = "Ice-" + iceVersion + "-gem"
gemDir = os.path.join(buildRootDir, dirName)
gemDirLib = os.path.join(gemDir, "lib")
gemDirBin = os.path.join(gemDir, "bin")
if os.path.exists(gemDir):
    print "Removing previous build from " + gemDir + " ...",
    sys.stdout.flush()
    shutil.rmtree(gemDir)
    print "ok"

os.mkdir(gemDir)
os.mkdir(gemDirLib)
os.mkdir(gemDirBin)
DistUtils.copy(os.path.join(srcDir, "ICE_LICENSE"), gemDir, verbose)
DistUtils.copy(os.path.join(srcDir, "LICENSE"), gemDir, verbose)

DistUtils.copy(os.path.join(distDir, "src/rb/ice.gemspec"), gemDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/rb/ice-x64-mingw.gemspec"), gemDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/rb/ice-x86-mingw.gemspec"), gemDir, verbose)
DistUtils.copy(os.path.join(distDir, "src/rb/slice2rb"), gemDirBin, verbose)
DistUtils.copyMatchingFiles(os.path.join(srcDir, "rb/ruby"), gemDirLib, ["*.rb"], False, verbose)
for d in os.listdir(os.path.join(srcDir, "rb/ruby")):
    DistUtils.copyMatchingFiles(os.path.join(srcDir, "rb/ruby", d), os.path.join(gemDirLib, d), ["*.rb"], False, verbose)

for d in os.listdir(os.path.join(srcDir, "slice")):
    DistUtils.copyMatchingFiles(os.path.join(srcDir, "slice", d), os.path.join(gemDir, "slice", d), ["*"], False, verbose)

gemDirExt = os.path.join(gemDir, "ext")
gemDirIce = os.path.join(gemDir, "ext", "ice")
os.mkdir(gemDirExt)
os.mkdir(gemDirIce)

DistUtils.copy(os.path.join(distDir, "src/rb/extconf.rb"), gemDirExt, verbose)
DistUtils.copyMatchingFiles(os.path.join(srcDir, "rb/src/IceRuby"), gemDirExt, ["*.cpp", "*.h"], False, verbose)

#
# Extract the relevant portions of the Ice C++ source tree into the gem.
#
copyDirs = [ "cpp/src/IceUtil", "cpp/src/Slice", "cpp/src/Ice", "cpp/include/IceUtil",
             "cpp/include/Slice", "cpp/include/Ice", "cpp/src/IceSSL", "cpp/include/IceSSL",
             "cpp/include/IceDiscovery", "cpp/src/IceDiscovery", "cpp/include/IceLocatorDiscovery", "cpp/src/IceLocatorDiscovery" ]
for d in copyDirs:
    DistUtils.copyMatchingFiles(os.path.join(srcDir, d), os.path.join(gemDirIce, d), ["*.cpp", "*.h", "Makefile"], False, verbose)

os.unlink(os.path.join(gemDirExt, "ice/cpp/src/Ice/DLLMain.cpp"))

for d in ["main.c", "directive.c", "eval.c", "expand.c", "mbchar.c", "support.c", "system.c", "internal.H", "mcpp_out.h", "mcpp_lib.h", "system.H", "configed.H"]:
    DistUtils.copy("%s/%s/mcpp-2.7.2/src/%s" % (buildRootDir, thirdPartyPackage, d), os.path.join(gemDirIce, "mcpp", d))
DistUtils.copyMatchingFiles(os.path.join(distDir, "src/rb/src/mcpp", ), os.path.join(gemDirIce, "mcpp"), ["*"], False, verbose)

DistUtils.copy(os.path.join(distDir, "src/unix/MCPP_LICENSE"), gemDirIce)

for d in ['blocksort.c', 'bzlib.c', 'compress.c','crctable.c','decompress.c','huffman.c','randtable.c','bzlib.h','bzlib_private.h']:
    DistUtils.copy("%s/zip/%s/bzip2-1.0.6/%s" % (buildRootDir, thirdPartyPackage, d), os.path.join(gemDirIce, "bzip2", d))
DistUtils.copy("%s/zip/%s/bzip2-1.0.6/LICENSE" % (buildRootDir, thirdPartyPackage), os.path.join(gemDirIce , "BZIP_LICENSE"))

os.chdir(gemDir)
gemFile = "zeroc-ice-" + getGemVersion('ice.gemspec') + ".gem"
if os.system("gem build ice.gemspec") != 0:
    print sys.argv[0] + ": building the gemspec failed"
    os.chdir(cwd)
    sys.exit(1)
copyFiles.append(os.path.join(gemDir, gemFile))

os.chdir(buildRootDir)

zipFile = "zeroc-ice-%s-gem.zip" % (iceVersion)
tarFile = "zeroc-ice-%s-gem.tar" % (iceVersion)
os.system("tar cf %s %s" % (tarFile, dirName))
if os.path.exists("%s.gz" % (tarFile)):
    os.unlink("%s.gz" % (tarFile))
os.system("gzip -9 %s" % (tarFile))
tarFile = tarFile + ".gz"

os.system("zip -9rq %s %s" % (zipFile, dirName))

copyFiles.append(os.path.join(buildRootDir, tarFile))
copyFiles.append(os.path.join(buildRootDir, zipFile))

os.chdir(cwd)
for f in copyFiles:
    print "Copying %s" % (f)
    DistUtils.copy(f, ".", verbose)
    os.unlink(f)

