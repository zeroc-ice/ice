#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-d    Skip SGML documentation conversion."
    print
    print "If no tag is given, HEAD is used."

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
# Comment out rules in a Makefile.
#
def fixMakefile(file, target):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldMakefile = open(origfile, "r")
    newMakefile = open(file, "w")
    origLines = oldMakefile.readlines()

    doComment = 0
    doCheck = 0
    newLines = []
    for x in origLines:
        #
        # If the line starts with the target string, then
        # comment out this make target.
        #
        if x.startswith(target) and not x.startswith(target + ".o"):
            doComment = 1
        #
        # If the line starts with "clean::", then check
        # the following lines and comment out any that
        # contain the target string.
        #
        elif x.startswith("clean::"):
            doCheck = 1
        #
        # Stop when we encounter an empty line.
        #
        elif len(x.strip()) == 0:
            doComment = 0
            doCheck = 0

        if doComment or (doCheck and x.find(target) != -1):
            x = "#" + x
        newLines.append(x)

    newMakefile.writelines(newLines)
    newMakefile.close()
    oldMakefile.close()
    os.remove(origfile)

#
# Check arguments
#
tag = "-rHEAD"
skipDocs = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-d":
        skipDocs = 1
    elif x.startswith("-"):
        print "Unknown option `" + x + "'"
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
# Export sources from CVS.
#
os.system("cvs -z5 -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " ice")

#
# Remove files.
#
filesToRemove = [ \
    "ice/makedist.py", \
    ]
for x in filesToRemove:
    os.remove(x)

#
# Generate bison files.
#
cwd = os.getcwd()
grammars = find("ice", "*.y")
for x in grammars:
    #
    # Change to the directory containing the file.
    #
    (dir,file) = os.path.split(x)
    os.chdir(dir)
    (base,ext) = os.path.splitext(file)
    #
    # Run gmake to create the output files.
    #
    if file == "cexp.y":
        os.system("gmake cexp.c")
    else:
        os.system("gmake " + base + ".cpp")
    #
    # Edit the Makefile to comment out the grammar rules.
    #
    fixMakefile("Makefile", base)
    os.chdir(cwd)

#
# Generate flex files.
#
scanners = find("ice", "*.l")
for x in scanners:
    #
    # Change to the directory containing the file.
    #
    (dir,file) = os.path.split(x)
    os.chdir(dir)
    (base,ext) = os.path.splitext(file)
    #
    # Run gmake to create the output files.
    #
    os.system("gmake " + base + ".cpp")
    #
    # Edit the Makefile to comment out the flex rules.
    #
    fixMakefile("Makefile", base)
    os.chdir(cwd)

#
# Generate HTML documentation. We need to build icecpp
# and slice2docbook first.
#
if not skipDocs:
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

    #
    # Clean the source directory.
    #
    os.chdir(os.path.join("ice", "src"))
    os.system("gmake clean")
    os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("ice", "include", "IceUtil", "Config.h"), "r")
version = re.search("ICE_STRING_VERSION \"(.*)\"", config.read()).group(1)

#
# Create archives.
#
icever = "Ice-" + version
os.rename("ice", icever)
os.system("tar cvzf " + icever + ".tar.gz " + icever)
os.system("zip -9r " + icever + ".zip " + icever)

#
# Copy files (README, etc.).
#

#
# Done.
#
shutil.rmtree(icever)
