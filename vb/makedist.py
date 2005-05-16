#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
# Fix version in README, INSTALL files
#
def fixVersion(files, version):

    for file in files:
        origfile = file + ".orig"
        os.rename(file, origfile)
        oldFile = open(origfile, "r")
        newFile = open(file, "w")
        newFile.write(re.sub("@ver@", version, oldFile.read()))
        newFile.close()
        oldFile.close()
        os.remove(origfile)

#
# Do a search and replace on a file using regular expressions a la sed.
#
def sedFile(path, patt, replace):
    src = open(path, "r")
    srcLines = src.readlines()

    dstLines = []
    for x in srcLines:
	dstLines.append(re.sub(patt, replace, x))

    src.close()
    dst = open(path, "w")
    dst.writelines(dstLines)

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
# Export VB and C++ sources from CVS.
#
# NOTE: Assumes that the C++ and C# trees will use the same tag.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
          " icevb ice/bin ice/config ice/doc ice/include ice/lib ice/slice ice/src" +
          " ice/install/vc71/doc/KNOWN_ISSUES.txt icecs/src/Ice/AssemblyInfo.cs")

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
    "IcePatch2",\
    "IceStorm",\
]
os.mkdir(os.path.join("icevb", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icevb", "slice", x), 1)

#
# Copy KNOWN_ISSUES.txt
#
shutil.copyfile(os.path.join("ice", "install", "vc71", "doc", "KNOWN_ISSUES.txt"), os.path.join("icevb", "KNOWN_ISSUES.txt"))

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
    os.mkdir(os.path.join("icevb", "doc"))
    os.rename(os.path.join("ice", "doc", "reference"), os.path.join("icevb", "doc", "reference"))
    os.rename(os.path.join("ice", "doc", "README.html"), os.path.join("icevb", "doc", "README.html"))
    os.rename(os.path.join("ice", "doc", "images"), os.path.join("icevb", "doc", "images"))
shutil.rmtree("ice")

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icevb", "makedist.py"), \
    ]
filesToRemove.extend(find("icevb", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Get Ice version.
#
config = open(os.path.join("icecs", "src", "Ice", "AssemblyInfo.cs"), "r")
version = re.search("AssemblyVersion.*\"([0-9\.]*)\"", config.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icevb", "README*"), version)
fixVersion(find("icevb", "INSTALL*"), version)

#
# Fix source dist demo project files.
#
hintPathSearch = r'(HintPath = "(\.\.\\)*)icecs(\\bin\\.*cs\.dll")'
hintPathReplace = r'\1IceCS-' + version + r'\3'
projectFiles = find(os.path.join("icevb", "demo"), "*.vbproj")
for x in projectFiles:
    if not x.endswith("D.vbproj"):
	sedFile(x, hintPathSearch, hintPathReplace)

#
# Create source archives.
#
print "Creating distribution archives..."
icever = "IceVB-" + version
os.rename("icevb", icever)
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
shutil.copyfile(os.path.join(icever, "CHANGES"), "IceVB-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("icecs")
print "Done."
