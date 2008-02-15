#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

version = "@ver@"
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(distDir, "lib"))
import DistUtils
from DistUtils import copy

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] tag"
    print
    print "Options:"
    print "-c    Clean previous build"
    print "-n    Don't build any language mapping"
    print "-h    Show this message."
    print "-v    Be verbose."

#
# Instantiate the gobal platform object
#
platform = DistUtils.getPlatform()

#
# Check arguments
#
verbose = 0
forceclean = 0
nobuild = 0
buildLanguages = [ ]
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-v":
        verbose = 1
    elif x == "-c":
        forceclean = 1
    elif x == "-n":
        nobuild = 1
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

if len(buildLanguages) == 0:
    buildLanguages = platform.languages

if nobuild:
    buildLanguages = []

if verbose:
    quiet = "v"
else:
    quiet = ""

#
# Ensure the script is being run from the dist-@ver@ directory.
#
cwd = os.getcwd()

if not os.path.exists(os.path.join(distDir, "src", "windows", "LICENSE.rtf")):
    print sys.argv[0] + ": you must run makebindist.py from the dist-" + version + " directory created by makedist.py"
    sys.exit(1)

print "Building Ice " + version + " binary distribution (" + platform.getPackageName(version) + ".tar.gz)"
print "Using the following third party libraries:"
if not platform.checkAndPrintThirdParties():
    print "error: some required third party dependencies were not found"
    sys.exit(1)
    
#
# Ensure that the source archive or directory exists and create the build directory.
#
buildRootDir = os.path.join(cwd, os.path.join("build-" + platform.pkgname + "-" + version))
srcDir = os.path.join(buildRootDir, "Ice-" + version + "-src")
buildDir = os.path.join(buildRootDir, "Ice-" + version)
if forceclean or not os.path.exists(srcDir) or not os.path.exists(buildDir):
    if os.path.exists(buildRootDir):
        print "Removing previous build from " + os.path.join("build-" + platform.pkgname + "-" + version) + "...",
        sys.stdout.flush()
        shutil.rmtree(buildRootDir)
        print "ok"
    os.mkdir(buildRootDir)

    #
    # If we can't find the source archive in the current directory, ask its location
    #
    if not os.path.exists(os.path.join(cwd, "Ice-" + version + ".tar.gz")):
        print
        src = raw_input("Couldn't find Ice-" + version + ".tar.gz in current directory, please specify\n" + \
                        "where to download or copy the source distribution or hit enter to \n" + \
                        "download it from sun:/share/srcdists/" + version + ": ")
        if src == "":
            src = "sun:/share/srcdists/" + version + "/Ice-" + version + ".tar.gz"
        elif not src.endswith("Ice-" + version + ".tar.gz"):
            src = os.path.join(src, "Ice-" + version + ".tar.gz")

        if os.system("scp " + src + " ."):
            print sys.argv[0] + ": couldn't copy " + src
            sys.exit(1)
        
    print "Unpacking ./Ice-" + version + ".tar.gz ...",
    sys.stdout.flush()
    os.chdir(buildRootDir)
    if os.system("gunzip -c " + os.path.join(cwd, "Ice-" + version + ".tar.gz") + " | tar x" + quiet + "f -"):
        print sys.argv[0] + ": failed to unpack ./Ice-" + version + ".tar.gz"
        sys.exit(1)
    os.rename("Ice-" + version, srcDir)
    os.chdir(cwd)
    print "ok"

    os.mkdir(buildDir)

#
# Build and install each language mappings supported by this platform in the build directory.
#
for l in buildLanguages:
    print
    print "============= Building " + l + " sources ============="
    print

    os.chdir(os.path.join(srcDir, l))

    if l != "java":

        makeCmd = "gmake " + platform.getMakeEnvs(version, l) + " prefix=" + buildDir + " install"
        
        #
        # Copy the language source directory to a directory suffixed with -lp64 
        # if this platform supports a 64 bits build and the directory doesn't 
        # exist yet.
        #
        if l in platform.build_lp64 and not os.path.exists(os.path.join(srcDir, l + "-lp64")):
            copy(os.path.join(srcDir, l), os.path.join(srcDir, l + "-lp64"))

        #
        # 32 bits build
        #
        if os.system("LP64=no " + makeCmd) != 0:
            print sys.argv[0] + ": `" + l + "' build failed"
            os.chdir(cwd)
            sys.exit(1)

        #
        # 64 bits build on platform supporting it.
        #
        if l in platform.build_lp64:
            os.chdir(os.path.join(srcDir, l + "-lp64"))
            if os.system("LP64=yes " + makeCmd) != 0:
                print sys.argv[0] + ": `" + l + "' build failed"
                os.chdir(cwd)
                sys.exit(1)

    else:
        antCmd = platform.getAntEnv() + " ant " + platform.getAntOptions() + " -Dprefix=" + buildDir

        if os.system(antCmd + " -Dbuild.suffix=-java2 -Dice.mapping=java2 install") != 0 or \
           os.system(antCmd + " -Dbuild.suffix=-java5 -Dice.mapping=java5 install") != 0: 
           print sys.argv[0] + ": `" + l + "' build failed"
           os.chdir(cwd)
           sys.exit(1)

    os.chdir(os.path.join(cwd))
    print
    print "============= Finished buiding " + l + " sources ============="
    print

#
# Copy platform third party dependencies.
#
print "Copying third party dependencies..."
sys.stdout.flush()
platform.copyThirdPartyDependencies(buildDir)
platform.completeDistribution(buildDir, version)

#
# Copy platform specific files (README, SOURCES, etc)
#
print "Copying distribution files (README, SOURCES, etc)...",
sys.stdout.flush()
platform.copyDistributionFiles(distDir, buildDir)

copy(os.path.join(distDir, "src", "common","RELEASE_NOTES.txt"), os.path.join(buildDir, "RELEASE_NOTES.txt"))

#
# Everything should be clean now, we can create the binary distribution archive
# 
print "Archiving " + platform.getPackageName(version) + ".tar.gz ...",
sys.stdout.flush()
os.chdir(buildRootDir)
tarfile = os.path.join(cwd, platform.getPackageName(version)) + ".tar.gz"
os.system("tar c" + quiet + "f - Ice-" + version + " | gzip -9 - > " + tarfile)
os.chdir(cwd)
print "ok"

#
# Done.
#
if forceclean:
    print "Cleaning up...",
    sys.stdout.flush()
    shutil.rmtree(buildRootDir)
    print "ok"

os.chdir(cwd)
