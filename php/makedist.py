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
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
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
os.system("cvs -d cvs.zeroc.com:/home/cvsroot export " + tag + " icephp ice/config ice/include/IceUtil/Config.h")


#
# Copy Make.rules.Linux and Make.rules.msvc
#
shutil.copyfile(os.path.join("ice", "config", "Make.rules.Linux"),
                os.path.join("icephp", "config", "Make.rules.Linux"))

shutil.copyfile(os.path.join("ice", "config", "Make.rules.msvc"),
                os.path.join("icephp", "config", "Make.rules.msvc"))

#
# Remove files.
#
filesToRemove = [ \
    os.path.join("icephp", "makedist.py"), \
    ]
filesToRemove.extend(find("icephp", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Get Ice version.
#
config = open(os.path.join("ice", "include", "IceUtil", "Config.h"), "r")
version = re.search("ICE_STRING_VERSION \"([0-9\.b]*)\"", config.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icephp", "README*"), version)
fixVersion(find("icephp", "INSTALL*"), version)

#
# Create archives.
#
icephpver = "IcePHP-" + version
os.rename("icephp", icephpver)
os.system("chmod -R u+rw,go+r-w . " + icephpver)
os.system("find " + icephpver + " \\( -name \"*.php\" -or -name \"*.ice\" \\) -exec chmod a-x {} \\;")
os.system("find " + icephpver + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
os.system("find " + icephpver + " -type d -exec chmod a+x {} \\;")
os.system("find " + icephpver + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar cvf " + icephpver + ".tar " + icephpver)
os.system("gzip -9 " + icephpver + ".tar")
os.system("zip -9r " + icephpver + ".zip " + icephpver)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icephpver, "CHANGES"), "IcePHP-" + version + "-CHANGES")

#
# Done.
#
shutil.rmtree(icephpver)
shutil.rmtree("ice")
