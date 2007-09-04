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
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-v    Be verbose."

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

if os.path.exists("../.git"):
    print "Unable to run in repository! Exiting..."
    sys.exit(1)

#
# Check arguments
#
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

#
# Remove any existing "dist" directory and create a new one.
#
distdir = "dist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)

#
# Get Ice version.
#
config = open(os.path.join("config", "Make.rules.cs"), "r")
version = re.search("VERSION[= \t]*([0-9\.b]+)", config.read()).group(1)

pcfg = open(os.path.join("lib", "pkgconfig", "icecs.pc"), "r")
dotnetversion = re.search("version[= \t]*([0-9\.]+)", pcfg.read()).group(1)

icever = "IceCS-" + version

os.mkdir(os.path.join(distdir, icever))

if verbose:
    quiet = "v"
else:
    quiet = ""

print "Copying Slice directories..."
os.system(" cd ../cpp && tar c" + quiet + " " + " slice -C ../cpp --exclude Makefile | (cd ../cs/dist/" + icever + " && tar xf -)")

#
# Remove files.
#
print "Creating exclusion file..."
filesToRemove = [ "makedist.py", "allDemos.py", "dist", "exclusions" ]
filesToRemove.extend(find(".", ".gitignore"))
filesToRemove.extend(find(".", "expect.py"))

exclusionFile = open("exclusions", "w")
for x in filesToRemove:
    exclusionFile.write("%s\n" % x)
exclusionFile.close()

os.system("tar c" + quiet + " -X exclusions . | ( cd " + os.path.join(distdir, icever) + " && tar xf - )")
os.chdir(distdir)

#
# Copy Slice directories.
#
for file in find(os.path.join(icever, "slice"), "Makefile.mak"):
    editMakefileMak(file)

#
# Get Ice version.
#
config = open(os.path.join(icever, "config", "Make.rules.cs"), "r")
version = re.search("VERSION[= \t]*([0-9\.b]+)", config.read()).group(1)

pcfg = open(os.path.join(icever, "lib", "pkgconfig", "icecs.pc"), "r")
dotnetversion = re.search("version[= \t]*([0-9\.]+)", pcfg.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find(icever, "README*"), version, dotnetversion)
fixVersion(find(icever, "INSTALL*"), version, dotnetversion)

#
# Create source archives.
#
print "Creating distribution archives..."
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
