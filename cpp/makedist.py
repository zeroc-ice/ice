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
# Check arguments
#
tag = "-rHEAD"
for x in sys.argv[1:]:
    if x == "-h":
        print "usage: " + sys.argv[0] + " [-h] [tag]"
        sys.exit(0)
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
    (dir,file) = os.path.split(x)
    os.chdir(dir)
    (base,ext) = os.path.splitext(file)
    if file == "cexp.y":
        os.system("gmake cexp.c")
    else:
        os.system("gmake " + base + ".cpp")
    os.chdir(cwd)

#
# Generate flex files.
#
cwd = os.getcwd()
scanners = find("ice", "*.l")
for x in scanners:
    (dir,file) = os.path.split(x)
    os.chdir(dir)
    (base,ext) = os.path.splitext(file)
    os.system("gmake " + base + ".cpp")
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
