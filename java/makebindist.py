#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
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
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print
    print "If no tag is specified, HEAD is used."

#
# Check arguments
#
tag = "-rHEAD"
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        tag = "-r" + x

if not os.environ.has_key("ICE_HOME"):
    print "The ICE_HOME environment variable is not set."
    sys.exit(1)

#
# Remove any existing distribution directory and create a new one.
#
distdir = "bindist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)
cwd = os.getcwd()

#
# Export Config.h from CVS.
#
os.system("cvs -z5 -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " ice/include/IceUtil/Config.h")

#
# Get Ice version.
#
config = open("ice/include/IceUtil/Config.h", "r")
version = re.search("ICE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

#
# Verify Ice version in CVS export matches the one in ICE_HOME.
#
config = open(os.environ["ICE_HOME"] + "/include/IceUtil/Config.h", "r")
version2 = re.search("ICE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

shutil.rmtree("ice")

if version != version2:
    print sys.argv[0] + ": the CVS version (" + version + ") does not match ICE_HOME (" + version2 + ")"
    sys.exit(1)

icever = "Ice-" + version
os.mkdir(icever)

#
# Get platform.
#
platform = ""
if sys.platform.startswith("win") or sys.platform.startswith("cygwin"):
    platform = "win32"
elif sys.platform.startswith("linux"):
    platform = "linux"
else:
    print "unknown platform (" + sys.platform + ")!"
    sys.exit(1)

#
# Copy executables and libraries.
#
icehome = os.environ["ICE_HOME"]
executables = [ ]
libraries = [ ]
symlinks = 0
if platform == "win32":
    winver = version.replace(".", "")
    debug = ""
    if not os.path.exists(icehome + "/bin/glacier" + winver + ".dll"):
        debug = "d"
    executables = [ \
        "glacierrouter.exe",\
        "icecpp.exe",\
        "slice2freezej.exe",\
        "slice2java.exe",\
        "slice2xsd.exe",\
        "glacier" + winver + debug + ".dll",\
        "icessl" + winver + debug + ".dll",\
        "ice" + winver + debug + ".dll",\
        "iceutil" + winver + debug + ".dll",\
        "slice" + winver + debug + ".dll",\
    ]
    libraries = [ \
    ]
else:
    executables = [ \
        "glacierrouter",\
        "glacierstarter",\
        "icecpp",\
        "slice2freezej",\
        "slice2java",\
        "slice2xsd",\
    ]
    libraries = [ \
        "libGlacier.so",\
        "libIceSSL.so",\
        "libIce.so",\
        "libIceUtil.so",\
        "libSlice.so",\
    ]
    symlinks = 1

bindir = icever + "/bin"
libdir = icever + "/lib"
os.mkdir(bindir)
os.mkdir(libdir)
for x in executables:
    shutil.copyfile(icehome + "/bin/" + x, bindir + "/" + x)
if symlinks:
    for x in libraries:
        libname = x + '.' + version
        shutil.copyfile(icehome + "/lib/" + libname, libdir + "/" + libname)
        os.chdir(libdir)
        os.symlink(libname, x)
        os.chdir(cwd)
else:
    for x in libraries:
        shutil.copyfile(icehome + "/lib/" + x, libdir + "/" + x)

#
# Create binary archives.
#
os.system("tar cvzf " + icever + "-bin-" + platform + ".tar.gz " + icever)
os.system("zip -9ry " + icever + "-bin-" + platform + ".zip " + icever)

#
# Copy files (README, etc.).
#

#
# Done.
#
shutil.rmtree(icever)
