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
        if fnmatch.fnmatch(x, patt):
            result.append(fullpath)
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.extend(find(fullpath, patt))
    return result

#
# Add some text to any line that matches the given pattern.
#
def addToLine(file, patt, text):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    expr = re.compile(patt)
    doComment = 0
    newLines = []
    for x in origLines:
	if expr.match(x) != None:
	    newLines.append(x.rstrip() + text + "\n")
	else:
	    newLines.append(x)

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

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
# Remove any existing "bindist" directory and create a new one.
#
distdir = "bindist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)

#
# Export C++ sources from CVS.
#
# We need to provide slice2rb and libSlice. The statements below export sources
# from version 3.1.0 and then add the Ruby-related code from ice/HEAD.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
#os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
#          " ice/config ice/include/Slice ice/src/Makefile ice/src/Slice ice/src/slice2rb")
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export -rR3_1_0" +
          " ice/config ice/include/Slice ice/src/Makefile ice/src/Slice")
os.remove(os.path.join("ice", "src", "Makefile"))
os.remove(os.path.join("ice", "src", "Slice", "Makefile"))
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export -rHEAD ice/include/Slice/RubyUtil.h" +
	  " ice/src/Makefile ice/src/Slice/Makefile ice/src/Slice/RubyUtil.cpp ice/src/slice2rb")

os.mkdir(os.path.join("ice", "bin"))
os.mkdir(os.path.join("ice", "lib"))

addToLine(os.path.join("ice", "config", "Make.rules"), "^CPPFLAGS", " -I$(ICE_HOME)/include")
addToLine(os.path.join("ice", "config", "Make.rules"), "^LDFLAGS", " -L$(ICE_HOME)/lib")

cwd = os.getcwd()
os.chdir(os.path.join("ice", "src"))
os.system("gmake depend")
os.system("gmake")
os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("ice", "config", "Make.rules"), "r")
version = re.search("^VERSION[ \t]+=[^\d]*([\d\.]+)", config.read(), re.M).group(1)

shutil.rmtree(os.path.join("ice", "config"))
shutil.rmtree(os.path.join("ice", "include"))
shutil.rmtree(os.path.join("ice", "src"))

#
# Create archives.
#
print "Creating distribution archives..."
icever = "IceRuby-" + version + "-bin-linux"
os.rename("ice", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icever)
os.system("find " + icever + " -type d -exec chmod a+x {} \\;")
os.system("find " + icever + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar c" + quiet + "f " + icever + ".tar " + icever)
os.system("gzip -9 " + icever + ".tar")
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9 -r " + quiet + " " + icever + ".zip " + icever)

#
# Done.
#
print "Cleaning up..."
#shutil.rmtree(icever)
print "Done."
