#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt

sys.path.append(os.path.join(os.path.dirname(__file__), "distribution", "lib"))
from DistUtils import *

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] tag"
    print
    print "Options:"
    print "-h          Show this message."
    print "-v          Be verbose."
    print "-c DIR      Compare distribution to the one from DIR"

#
# Check arguments
#
verbose = 0
tag = "HEAD"
compareToDir = None
server = None

try:
    opts, args = getopt.getopt(sys.argv[1:], "hvc:")
except getopt.GetoptError:
    usage()
    sys.exit(1)

for o, a in opts:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-v":
        verbose = 1
    elif o == "-c":
        compareToDir = a

if len(args) > 1:
    usage()
    sys.exit(1)

if len(args) == 1:
    tag = args[0]


cwd = os.getcwd()
os.chdir(cwd)

#
# Get version.
#
config = open(os.path.join("vsplugin", "config", "Make.rules.mak"), "r")
version = re.search("VERSION\s*=\s*([0-9\.]*)", config.read()).group(1)
config.close()

#
# Remove any existing "distvsplugin-" directory and create a new one
# and sub-directories for the each source distribution.
#
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "distvsplugin-" + tag.replace('/', '-')))
if os.path.exists(distDir):
    remove(distDir)
os.mkdir(distDir)

print "Creating " + version + " source distributions in " + distDir

srcDir = os.path.join(distDir, "IceVisualStudioExtension-" + version)
distFilesDir = os.path.join(distDir, "distfiles-" + version)
archDir = os.path.join(distDir, "ice")

#
# Extract the sources with git archive using the given tag.
#
print "Creating git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --prefix=ice/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
print "ok"

os.mkdir(srcDir)
os.chdir(os.path.join(srcDir))

print "Copying files from repository archive... ",
sys.stdout.flush()

# Copy files
for f in os.listdir(os.path.join(archDir, "vsplugin")):
    if f != "action.vbs" and f != "IceVisualStudioExtension.aip" and f.endswith(".csproj") != True and f.endswith(".sln") != True:
        copy(os.path.join(archDir, "vsplugin", f), f)

move(os.path.join(archDir, "distribution"), distFilesDir) # Move the distribution directory to the top-level

for root, dirnames, filesnames in os.walk('.'):

    for f in filesnames:
        filepath = os.path.join(root, f) 
        if f == ".gitignore":
            os.remove(filepath)
        else:
            # Fix version of README/INSTALL files and keep track of bison/flex files for later processing
            if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
                substitute(filepath, [("@ver@", version)])

            fixFilePermission(filepath, verbose)
    
    for d in dirnames:
        os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

rmFiles = []
for root, dirnames, filesnames in os.walk('.'):
    for f in filesnames:
        for m in ["*.sln", "*.csproj"]:
            if fnmatch.fnmatch(f, m):
                rmFiles.append(os.path.join("src", f))

for f in rmFiles: remove(f)

print "ok"

#
# Everything should be clean now, we can create the source distributions archives
# 
print "Archiving..."
sys.stdout.flush()
os.chdir(distDir)

for d in [srcDir, distFilesDir]:
    tarArchive(d, verbose)

for d in [srcDir]:
    zipArchive(d, verbose)

#
# Write source distribution report in README file.
#
writeSrcDistReport("IceVisualStudioExtension", version, compareToDir, [srcDir, distFilesDir])

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
remove(srcDir)
remove(archDir)
remove(distFilesDir)
print "ok"

os.chdir(cwd)
