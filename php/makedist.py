#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

if not os.environ.has_key("ICE_HOME"):
    print "The ICE_HOME environment variable is not set."
    sys.exit(1)

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
os.system("cvs -d cvs.zeroc.com:/home/cvsroot export " + tag + " icephp")

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
# Get IcePHP version.
#
config = open(os.path.join("icephp", "src", "ice", "php_ice.h"), "r")
version = re.search("ICEPHP_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icephp", "README*"), version)
fixVersion(find("icephp", "INSTALL*"), version)

#
# Create archives.
#
icephpver = "IcePHP-" + version
os.rename("icephp", icephpver)
os.system("tar cvf " + icephpver + ".tar " + icephpver)
os.system("gzip -9 " + icephpver + ".tar")
os.system("zip -9r " + icephpver + ".zip " + icephpver)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icever, "CHANGES"), "IcePHP-" + version + "-CHANGES")

#
# Done.
#
shutil.rmtree(icephpver)
