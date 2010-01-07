#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, xml.dom.minidom

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "distribution", "lib"))
from DistUtils import *

#
# Subdirectories to keep in the source distributions.
#
includeSubDirs = [ \
    "Slice2Java", \
    "Slice2javaPlugin", \
]

#
# Files to not include in the source distributions.
#
filesToRemove = [ \
    "makedist.py", \
]

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h      Show this message."
    print "-v      Be verbose."

#
# Check arguments
#
try:
    opts, args = getopt.getopt(sys.argv[1:], "hv")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)

tag = "HEAD"
if len(args) > 1:
    usage()
elif len(args) == 1:
    tag = args[0]

verbose = 0
for (o, a) in opts:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-v":
        verbose = 1

cwd = os.getcwd()
os.chdir(os.path.dirname(__file__))

#
# Get the plug-in version.
#
dom = xml.dom.minidom.parse(os.path.join("Slice2Java", "feature.xml"))
version = str(dom.documentElement.getAttribute("version"))
idx = version.find(".qualifier") # Version may be of the form "X.Y.Z.qualifier"
if idx != -1:
    version = version[0:idx]

#
# Remove any existing "dist-" directory and create a new one
# and sub-directories for the each source distribution.
#
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "dist-" + tag.replace('/', '-')))
if os.path.exists(distDir):
    remove(distDir)
os.mkdir(distDir)
srcDir = "Slice2JavaPlugin-" + version

print "Creating " + version + " source distributions in " + distDir

#
# Extract the sources with git archive using the given tag.
#
print "Creating git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --prefix=" + srcDir + "/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
print "ok"

os.chdir(os.path.join(distDir, srcDir))

#
# Remove or move non-public files out of source distribution.
#
print "Removing directories and files...",
sys.stdout.flush()
for x in filesToRemove:
    remove(x)
for d in os.listdir('.'):
    if os.path.isdir(d) and not d in includeSubDirs:
        remove(d)
print "ok"

for root, dirnames, filenames in os.walk('.'):

    for f in filenames:
        filepath = os.path.join(root, f)
        if f == ".gitignore":
            remove(filepath)

    for d in dirnames:
        os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

print "ok"

#
# Everything should be clean now, we can create the source distributions archives
#
print "Archiving..."
sys.stdout.flush()
os.chdir(distDir)
tarArchive(os.path.join(distDir, srcDir), verbose)
zipArchive(os.path.join(distDir, srcDir), verbose)

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
remove(srcDir)
print "ok"

os.chdir(cwd)
