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
# Program usage.
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

if os.path.exists("../.git"):
    print "Unable to run in repository! Exiting..."
    sys.exit(1)

verbose = False

#
# Check arguments
#
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-v":
        verbose = True
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

#
# Get Ice version.
#
config = open(os.path.join(icedir, "include", "IceUtil", "Config.h"), "r")
version = re.search("ICE_STRING_VERSION \"([0-9\.b]*)\"", config.read()).group(1)
icephpver = "IcePHP-" + version

os.mkdir(os.path.join(distdir, icephpver))

if verbose:
    quiet = "v"
else:
    quiet = ""

#
# Remove files.
#
filesToRemove = [ "makedist.py", "exclusions", "dist", "allDemos.py" ]
filesToRemove.extend(find(".", ".gitignore"))
filesToRemove.extend(find(".", "expect.py"))

exclusionFile = open("exclusions", "w")
for x in filesToRemove:
    exclusionFile.write("%s\n" % x)
exclusionFile.close()

os.system("tar c" + quiet + " -X exclusions . | ( cd " + os.path.join(distdir, icephpver) + " && tar xf - )")

os.chdir(distdir)

#
# Copy Make.rules.Linux and Make.rules.msvc
#
shutil.copyfile(os.path.join(icedir, "config", "Make.rules.Linux"),
                os.path.join(icephpver, "config", "Make.rules.Linux"))

shutil.copyfile(os.path.join(icedir, "config", "Make.rules.msvc"),
                os.path.join(icephpver, "config", "Make.rules.msvc"))


print "Fixing version in README and INSTALL files..."
fixVersion(find(icephpver, "README*"), version)
fixVersion(find(icephpver, "INSTALL*"), version)

#
# Create archives.
#
os.system("chmod -R u+rw,go+r-w . " + icephpver)
os.system("find " + icephpver + " \\( -name \"*.php\" -or -name \"*.ice\" \\) -exec chmod a-x {} \\;")
os.system("find " + icephpver + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
os.system("find " + icephpver + " -type d -exec chmod a+x {} \\;")
os.system("find " + icephpver + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar cf " + icephpver + ".tar " + icephpver)
os.system("gzip -9 " + icephpver + ".tar")
os.system("zip -9rq " + icephpver + ".zip " + icephpver)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icephpver, "CHANGES"), "IcePHP-" + version + "-CHANGES")

#
# Done.
#
shutil.rmtree(icephpver)
