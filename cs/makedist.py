#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-d    Skip SGML documentation conversion."
    print "-v    Be verbose."
    print
    print "If no tag is specified, HEAD is used."

#
# Find files matching a pattern.
#
def find(path, patt):
    result = [ ]
    files = os.listdir(path)
    for x in files:
        fullpath = os.path.join(path, x);
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.extend(find(fullpath, patt))
        elif fnmatch.fnmatch(x, patt):
            result.append(fullpath)
    return result

#
# Are we on Windows?
#
win32 = sys.platform.startswith("win") or sys.platform.startswith("cygwin")

#
# Check arguments
#
tag = "-rHEAD"
skipDocs = 0
verbose = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-d":
        skipDocs = 1
    elif x == "-v":
        verbose = 1
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        tag = "-r" + x

if win32 and not skipDocs:
    print sys.argv[0] + ": the documentation cannot be built on Windows."
    sys.exit(1)

#
# Remove any existing "dist" directory and create a new one.
#
distdir = "dist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)

#
# Export C# sources from CVS.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " icecs")

#
# Export C++ sources.
#
# NOTE: Assumes that the C++ and C# trees will use the same tag.
#
os.system("cvs " + quiet + " -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " ice")
#
# Copy Slice directories.
#
print "Copying Slice directories..."
slicedirs = [\
    "Freeze",\
    "Glacier2",\
    "Ice",\
    "IceBox",\
    "IcePack",\
    "IcePatch",\
    "IceStorm",\
]
os.mkdir(os.path.join("icecs", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icecs", "slice", x), 1)
#
# Generate HTML documentation. We need to build icecpp
# and slice2docbook first.
#
if not skipDocs:
    print "Generating documentation..."
    cwd = os.getcwd()
    os.chdir(os.path.join("ice", "src", "icecpp"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "src", "IceUtil"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "src", "Slice"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "src", "slice2docbook"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "doc"))
    os.system("gmake")
    os.chdir(cwd)
    os.mkdir(os.path.join("icecs", "doc"))
    os.rename(os.path.join("ice", "doc", "manual"), os.path.join("icecs", "doc", "manual"))
shutil.rmtree("ice")

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icecs", "makedist.py"), \
    ]
filesToRemove.extend(find("icecs", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Get Ice version.
#
config = open(os.path.join("icecs", "src", "Ice", "AssemblyInfo.cs"), "r")
version = re.search("AssemblyVersion.*\"([0-9\.]*)\"", config.read()).group(1)

#
# Create source archives.
#
print "Creating distribution..."
icever = "IceCS-" + version
os.rename("icecs", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("tar c" + quiet + "zf " + icever + ".tar.gz " + icever)
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9 -r " + quiet + " " + icever + ".zip " + icever)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icever, "CHANGES"), "IceCS-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
print "Done."
