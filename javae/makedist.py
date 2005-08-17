#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
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
        elif os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.extend(find(fullpath, patt))
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
# Export Ice-EJ sources from CVS.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag + " iceje")

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("iceje", "makedist.py"), \
    ]
filesToRemove.extend(find("iceje", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Build sources (for JDK and MIDP).
#
print "Compiling..."
cwd = os.getcwd()
os.chdir("iceje")
if verbose:
    quiet = ""
else:
    quiet = " -q"
os.system("ant" + quiet + " -Dmidp=on -Doptimize=on -Ddebug=off")

#
# Clean out the jdk/lib directory but save IceE.jar.
# Preserve the classes in midp/lib.
#
os.rename(os.path.join("jdk", "lib", "IceE.jar"), "IceE.jar")
shutil.rmtree(os.path.join("jdk", "lib"))
os.mkdir(os.path.join("jdk", "lib"))
os.rename("IceE.jar", os.path.join("jdk", "lib", "IceE.jar"))

#
# Clean out the translator test directory.
#
os.chdir(os.path.join("test", "IceE", "translator"))
os.system("ant" + quiet + " clean")

os.chdir(cwd)

#
# Remove all generated directories.
#
generatedDirs = find("iceje", "generated")
for x in generatedDirs:
    shutil.rmtree(x)
shutil.rmtree(os.path.join("iceje", "depcache"))

#
# Get Ice version.
#
config = open(os.path.join("iceje", "src", "IceUtil", "Version.java"), "r")
version = re.search("ICEE_STRING_VERSION = \"([0-9\.]*)\"", config.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("iceje", "README*"), version)
fixVersion(find("iceje", "INSTALL*"), version)

#
# Copy KNOWN_ISSUES.txt
#
#shutil.copyfile(os.path.join("ice", "install", "vc71", "doc", "KNOWN_ISSUES.txt"), os.path.join("icej", "KNOWN_ISSUES.txt"))

#
# Create source archives.
#
print "Creating distribution..."
icever = "IceJE-" + version
os.rename("iceje", icever)
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
#shutil.copyfile(os.path.join(icever, "CHANGES"), "IceJ-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
print "Done."
