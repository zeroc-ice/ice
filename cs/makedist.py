#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, fileinput

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
def fixVersion(files, version, dotnetversion):

    for file in files:
        origfile = file + ".orig"
        os.rename(file, origfile)
        oldFile = open(origfile, "r")
        newFile = open(file, "w")
        line = oldFile.read();
        line = re.sub("@ver@", version, line)
        line = re.sub("@dotnetver@", dotnetversion, line)
        newFile.write(line)
        newFile.close()
        oldFile.close()
        os.remove(origfile)


def editMakefileMak(file):
    makefile =  fileinput.input(file, True)
    for line in makefile:
        if line.startswith('!include'):
            print '!include $(top_srcdir)/config/Make.rules.mak.cs'
        else:
            print line.rstrip('\n')
    makefile.close()

#
# Are we on Windows?
#
win32 = sys.platform.startswith("win") or sys.platform.startswith("cygwin")

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

#
# Remove any existing "dist" directory and create a new one.
#
distdir = "dist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)

#
# Export C# and C++ sources from CVS.
#
# NOTE: Assumes that the C++ and C# trees will use the same tag.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
          " icecs ice/bin ice/config ice/include ice/lib ice/slice ice/src")

#
# Copy Slice directories.
#
print "Copying Slice directories..."
shutil.copytree(os.path.join("ice", "slice"), os.path.join("icecs", "slice"), 1)
for file in find(os.path.join("icecs", "slice"), "Makefile.mak"):
    editMakefileMak(file)
shutil.rmtree(os.path.join("icecs", "slice", "IceSSL"))

#
# Makefiles found in the slice directories are removed later
# on. 
#
shutil.rmtree("ice")

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icecs", "makedist.py"), \
    ]
filesToRemove.extend(find("icecs", ".dummy"))
filesToRemove.extend(find(os.path.join("icecs", "slice"), "Makefile"))
for x in filesToRemove:
    os.remove(x)

#
# Get Ice version.
#
config = open(os.path.join("icecs", "config", "Make.rules.cs"), "r")
version = re.search("VERSION[= \t]*([0-9\.b]+)", config.read()).group(1)

pcfg = open(os.path.join("icecs", "lib", "pkgconfig", "icecs.pc"), "r")
dotnetversion = re.search("version[= \t]*([0-9\.]+)", pcfg.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icecs", "README*"), version, dotnetversion)
fixVersion(find("icecs", "INSTALL*"), version, dotnetversion)

#
# Create source archives.
#
print "Creating distribution archives..."
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
