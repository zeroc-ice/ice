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
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-b    Create a platform-specific binary archives. If not specified,"
    print "      only source archives will be created."
    print "-d    Skip SGML documentation conversion."
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
# Check arguments
#
tag = "-rHEAD"
binary = 0
skipDocs = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-b":
        if not os.environ.has_key("ICE_HOME"):
            print "The ICE_HOME environment variable is not set."
            sys.exit(1)
        binary = 1
    elif x == "-d":
        skipDocs = 1
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
# Export Java sources from CVS.
#
os.system("cvs -z5 -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " icej")

#
# Export C++ sources.
#
# NOTE: Assumes that the C++ and Java trees will use the same tag.
#
os.system("cvs -z5 -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " ice")
#
# Copy Slice directories.
#
slicedirs = [\
    "Freeze",\
    "Glacier",\
    "Ice",\
    "IceBox",\
    "IcePack",\
    "IceStorm",\
]
os.mkdir(os.path.join("icej", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icej", "slice", x), 1)
#
# Generate HTML documentation. We need to build icecpp
# and slice2docbook first.
#
if not skipDocs:
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
    os.mkdir(os.path.join("icej", "doc"))
    os.rename(os.path.join("ice", "doc", "manual"), os.path.join("icej", "doc", "manual"))
shutil.rmtree("ice")

#
# Remove files.
#
filesToRemove = [ \
    os.path.join("icej", "makedist.py"), \
    ]
filesToRemove.extend(find("icej", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Get Ice version.
#
config = open(os.path.join("icej", "src", "IceUtil", "Version.java"), "r")
version = re.search("ICE_STRING_VERSION = \"(.*)\"", config.read()).group(1)

#
# Create source archives.
#
icever = "IceJ-" + version
os.rename("icej", icever)
os.system("tar cvzf " + icever + ".tar.gz " + icever)
os.system("zip -9 -r " + icever + ".zip " + icever)

#
# Copy files (README, etc.).
#

#
# Create binary archives if requested.
#
if binary:
    cwd = os.getcwd()
    os.chdir(icever)

    #
    # Build classes.
    #
    os.system("ant")

    os.chdir(cwd)

    #
    # Get platform.
    #
    platform = ""
    if sys.platform.startswith("win") or sys.platform.startswith("cygwin"):
        platform = "win32"
    elif sys.platform.startswith("linux"):
        platform = "linux"
    else:
        print "unknown platform!"
        sys.exit(1)

    #
    # Copy executables and libraries.
    #
    executables = [ ]
    libraries = [ ]
    symlinks = 0
    if platform == "win32":
        winver = version.replace(".", "")
        executables = [ \
            "glacierrouter.exe",\
            "glacierstarter.exe",\
            "icecpp.exe",\
            "slice2freezej.exe",\
            "slice2java.exe",\
            "slice2xsd.exe",\
        ]
        libraries = [ \
            "glacier" + winver + ".dll",\
            "icessl" + winver + ".dll",\
            "ice" + winver + ".dll",\
            "iceutil" + winver + ".dll",\
            "slice" + winver + ".dll",\
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

    bindir = os.path.join(icever, "bin")
    libdir = os.path.join(icever, "lib")
    os.mkdir(bindir)
    icehome = os.environ["ICE_HOME"]
    for x in executables:
        shutil.copyfile(os.path.join(icehome, "bin", x), os.path.join(bindir, x))
    if symlinks:
        for x in libraries:
            libname = x + '.' + version
            shutil.copyfile(os.path.join(icehome, "lib", libname), os.path.join(libdir, libname))
            os.chdir(libdir)
            os.symlink(libname, x)
            os.chdir(cwd)
    else:
        for x in libraries:
            shutil.copyfile(os.path.join(icehome, "lib", x), os.path.join(libdir, x))

    #
    # Create binary archives.
    #
    os.system("tar cvzf " + icever + "-bin-" + platform + ".tar.gz " + icever)
    os.system("zip -9ry " + icever + "-bin-" + platform + ".zip " + icever)

#
# Done.
#
shutil.rmtree(icever)
