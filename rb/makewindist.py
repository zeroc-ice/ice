#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# NOTE: This script is only temporary!
#


import os, sys, shutil, fnmatch, re, glob

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [build-dir]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-v    Be verbose."
    print
    print "Where build-dir is the directory containing a source build."
    print "If not specified, the current directory is used."

#
# Find files matching a pattern.
#
def find(path, patt):
    result = [ ]
    files = os.listdir(path)
    for x in files:
        fullpath = os.path.join(path, x);
        if fnmatch.fnmatch(x, patt):
            result.append(fullpath)
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.extend(find(fullpath, patt))
    return result

#
# Are we on Windows?
#
win32 = sys.platform.startswith("win") or sys.platform.startswith("cygwin")
if not win32:
    print "This script is only for Windows!"
    sys.exit(1)

#
# Check arguments
#
buildDir = "."
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
        buildDir = x

if not os.path.exists(buildDir):
    print "error: " + buildDir + " not found."
    usage()
    sys.exit(1)

if not os.path.exists(os.path.join(buildDir, "README.txt")):
    print "error: " + buildDir + " must contain a compiled CVS repository or export directory."
    sys.exit(1)

#
# Get Ice version.
#
config = open(os.path.join(buildDir, "config", "Make.rules"), "r")
version = re.search("^VERSION[ \t]+=[^\d]*([\d\.]+)", config.read(), re.M).group(1)

distDir = "IceRuby-" + version
archive = distDir + "-bin-win32.zip"
if os.path.exists(distDir):
    shutil.rmtree(distDir)
if os.path.exists(archive):
    os.remove(archive)
os.mkdir(distDir)

shutil.copytree(os.path.join(buildDir, "bin"), os.path.join(distDir, "bin"))
shutil.copytree(os.path.join(buildDir, "certs"), os.path.join(distDir, "certs"))
shutil.copytree(os.path.join(buildDir, "demo"), os.path.join(distDir, "demo"))
shutil.copytree(os.path.join(buildDir, "ruby"), os.path.join(distDir, "ruby"))

shutil.copyfile(os.path.join(buildDir, "README.txt"), os.path.join(distDir, "README.txt"))
shutil.copyfile(os.path.join(buildDir, "LICENSE"), os.path.join(distDir, "LICENSE"))
shutil.copyfile(os.path.join(buildDir, "ICE_LICENSE"), os.path.join(distDir, "ICE_LICENSE"))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = []
filesToRemove.extend(find(distDir, "Makefile"))
filesToRemove.extend(find(distDir, "*.mak"))
filesToRemove.extend(find(distDir, "*.dsp"))
filesToRemove.extend(find(distDir, "*.dsw"))
filesToRemove.extend(find(distDir, ".dummy"))
filesToRemove.extend(find(distDir, "*.plg"))
for x in filesToRemove:
    os.remove(x)
for x in find(distDir, "CVS"):
    shutil.rmtree(x)

#
# Create archives.
#
print "Creating distribution archive..."
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9ry " + quiet + " " + archive + " " + distDir)

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(distDir)
print "Done."
