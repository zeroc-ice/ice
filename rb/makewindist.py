#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    print "Usage: " + sys.argv[0] + " [options] build-dir"
    print
    print "Options:"
    print "-h    Show this message."
    print "-v    Be verbose."
    print
    print "Where build-dir is the directory containing a source build."
    print
    print "NOTE: ICE_HOME must point to the patched Ice build."

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

if not os.environ.has_key("ICE_HOME"):
    print "ICE_HOME is not defined!"
    sys.exit(1)

if not os.path.exists(os.path.join(os.environ["ICE_HOME"], "bin", "slice2rb.exe")):
    print "slice2rb not found in %ICE_HOME%\bin"
    sys.exit(1)

#
# Check arguments
#
buildDir = None
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

if not buildDir:
    usage()
    sys.exit(1)

if not os.path.exists(buildDir):
    print "error: " + buildDir + " not found."
    sys.exit(1)

if not os.path.exists(os.path.join(buildDir, "README.txt")):
    print "error: " + buildDir + " must contain a compiled CVS repository or export directory."
    sys.exit(1)

if not os.path.exists(os.path.join(os.environ["ICE_HOME"], "bin", "slice2rb.exe")):
    print "error: slice2rb not found in %ICE_HOME%\bin."
    sys.exit(1)

#
# Get Ice version.
#
config = open(os.path.join(os.environ["ICE_HOME"], "include", "IceUtil", "Config.h"), "r")
version = re.search("ICE_STRING_VERSION[ \t]+\"([\d\.]+)\"", config.read(), re.M).group(1)

distDir = "IceRuby-" + version + "-bin-win32"
if os.path.exists(distDir):
    shutil.rmtree(distDir)
if os.path.exists(distDir + ".zip"):
    os.remove(distDir + ".zip")
os.mkdir(distDir)

os.mkdir(os.path.join(distDir, "bin"))

shutil.copytree(os.path.join(buildDir, "certs"), os.path.join(distDir, "certs"))
shutil.copytree(os.path.join(buildDir, "demo"), os.path.join(distDir, "demo"))
shutil.copytree(os.path.join(buildDir, "ruby"), os.path.join(distDir, "ruby"))

shutil.copyfile(os.path.join(buildDir, "README.txt"), os.path.join(distDir, "README.txt"))
shutil.copyfile(os.path.join(os.environ["ICE_HOME"], "bin", "slice2rb.exe"), os.path.join(distDir, "bin", "slice2rb.exe"))
shutil.copyfile(os.path.join(os.environ["ICE_HOME"], "bin", "sliceruby31.dll"), os.path.join(distDir, "bin", "sliceruby31.dll"))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = []
filesToRemove.extend(find(distDir, "Makefile"))
filesToRemove.extend(find(distDir, "*.mak"))
filesToRemove.extend(find(distDir, "*.dsp"))
filesToRemove.extend(find(distDir, "*.dsw"))
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
os.system("zip -9ry " + quiet + " " + distDir + ".zip " + distDir)

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(distDir)
print "Done."
