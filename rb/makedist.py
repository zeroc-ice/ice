#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, glob

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
        if fnmatch.fnmatch(x, patt):
            result.append(fullpath)
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
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

icedir = os.path.join(os.getcwd(), "..", "cpp")
config = open(os.path.join("config", "Make.rules"), "r")
version = re.search("VERSION[= \t]*([0-9\.b]+)", config.read()).group(1)
icerbver = "IceRuby-" + version

os.mkdir(os.path.join(distdir, icerbver))

if verbose:
    quiet = "v"
else:
    quiet = ""
    
#
# Remove files.
#
print "Creating exclusion file..."
filesToRemove = [ "makedist.py", "makebindist.py", "makewindist.py", "exclusions", "dist", "allDemos.py"]
filesToRemove.extend(find(".", ".gitignore"))
filesToRemove.extend(find(".", "expect.py"))

exclusionFile = open("exclusions", "w")
for x in filesToRemove:
    exclusionFile.write("%s\n" % x)
exclusionFile.close()

os.system("tar c" + quiet + " -X exclusions . | ( cd " + os.path.join(distdir, icerbver) + " && tar xf - )")

os.chdir(distdir)

#
# Copy Make.rules.Linux and Make.rules.msvc
#
shutil.copyfile(os.path.join(icedir, "config", "Make.rules.Linux"),
                os.path.join(icerbver, "config", "Make.rules.Linux"))

shutil.copyfile(os.path.join(icedir, "config", "Make.rules.msvc"),
                os.path.join(icerbver, "config", "Make.rules.msvc"))


print "Fixing version in README and INSTALL files..."
fixVersion(find(icerbver, "README*"), version)
fixVersion(find(icerbver, "INSTALL*"), version)

#
# Create source archives.
#
print "Creating distribution archives..."
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icerbver)
os.system("find " + icerbver + " \\( -name \"*.ice\" -or -name \"*.xml\" \\) -exec chmod a-x {} \\;")
os.system("find " + icerbver + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
os.system("find " + icerbver + " -type d -exec chmod a+x {} \\;")
os.system("find " + icerbver + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar c" + quiet + "f " + icerbver + ".tar " + icerbver)
os.system("gzip -9 " + icerbver + ".tar")
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9 -r " + quiet + " " + icerbver + ".zip " + icerbver)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icerbver, "CHANGES"), "IceRuby-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icerbver)
print "Done."
