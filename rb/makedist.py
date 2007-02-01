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
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-d    Skip SGML documentation conversion."
    print "-t    Skip building translator and use the one in PATH."
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
skipDocs = 0
skipTranslator = 0
verbose = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-d":
        skipDocs = 1
    elif x == "-t":
        skipTranslator = 1
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
# Export Ruby and C++ sources from CVS.
#
# NOTE: Assumes that the C++ and Ruby trees will use the same tag.
#
print "Checking out Ruby sources using CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag + " icerb")
print "Checking out C++ sources using CVS tag " + tag + "..."
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag + " ice/slice")
if not skipDocs or not skipTranslator:
    os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
              " ice/bin ice/config ice/doc ice/include ice/lib ice/src")

#
# Copy Slice directories.
#
print "Copying Slice directories..."
slicedirs = [\
    "Glacier2",\
    "Ice",\
    "IceBox",\
    "IceGrid",\
    "IcePatch2",\
    "IceStorm",\
]
os.mkdir(os.path.join("icerb", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icerb", "slice", x), 1)

#
# Copy any platform-specific Make.rules files.
#
#for x in glob.glob(os.path.join("ice", "config", "Make.rules.*")):
#    if not os.path.exists(os.path.join("icerb", "config", os.path.basename(x))):
#       shutil.copyfile(x, os.path.join("icerb", "config", os.path.basename(x)))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icerb", "makedist.py"), \
    os.path.join("icerb", "makebindist.py"), \
    os.path.join("icerb", "makewindist.py"), \
    os.path.join("icerb", "README.txt"), \
    os.path.join("icerb", "README.Linux"), \
    ]
filesToRemove.extend(find("icerb", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Generate HTML documentation. We need to build icecpp
# and slice2html first.
#
if not skipDocs:
    print "Generating documentation..."
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
    os.chdir(os.path.join("ice", "src", "slice2html"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "doc"))
    os.system("gmake")
    os.chdir(cwd)
    os.environ["ICE_HOME"] = os.path.join(cwd, "ice")
    os.mkdir(os.path.join("icerb", "doc"))
    os.rename(os.path.join("ice", "doc", "reference"), os.path.join("icerb", "doc", "reference"))
    os.rename(os.path.join("ice", "doc", "README.html"), os.path.join("icerb", "doc", "README.html"))
    os.rename(os.path.join("ice", "doc", "images"), os.path.join("icerb", "doc", "images"))

#
# Build slice2rb.
#
if not skipTranslator:
    print "Building translator..."
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
    os.chdir(os.path.join("ice", "src", "slice2rb"))
    os.system("gmake")
    os.chdir(cwd)

    os.environ["PATH"] = os.path.join(cwd, "ice", "bin") + ":" + os.path.join(cwd, "icerb", "bin") + ":" + os.getenv("PATH", "")
    os.environ["LD_LIBRARY_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.path.join(cwd, "icerb", "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")

#
# Translate Slice files.
#
print "Generating Ruby code..."
cwd = os.getcwd()
os.chdir(os.path.join("icerb", "ruby"))
if verbose:
    quiet = ""
else:
    quiet = " -s"
os.system("gmake" + quiet)
os.chdir(cwd)

#
# Clean up after build.
#
if not skipTranslator:
    cwd = os.getcwd()
    os.chdir(os.path.join("icerb", "src"))
    if verbose:
        quiet = ""
    else:
        quiet = " -s"
    os.system("gmake" + quiet + " clean")
    os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("icerb", "config", "Make.rules"), "r")
version = re.search("^VERSION[ \t]+=[^\d]*([\d\.]+)", config.read(), re.M).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icerb", "README*"), version)
fixVersion(find("icerb", "INSTALL*"), version)

#
# Create source archives.
#
print "Creating distribution archives..."
icever = "IceRuby-" + version
os.rename("icerb", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icever)
os.system("find " + icever + " \\( -name \"*.ice\" -or -name \"*.xml\" \\) -exec chmod a-x {} \\;")
os.system("find " + icever + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
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
# Copy files (README, etc.).
#
#shutil.copyfile(os.path.join(icever, "CHANGES"), "IceRuby-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("ice")
print "Done."
