#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
# Export Python and C++ sources from CVS.
#
# NOTE: Assumes that the C++ and Python trees will use the same tag.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
          " icepy ice/bin ice/config ice/doc ice/include ice/lib ice/slice ice/src")

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
os.mkdir(os.path.join("icepy", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icepy", "slice", x), 1)
for x in glob.glob(os.path.join("ice", "config", "Make.rules.*")):
    if not os.path.exists(os.path.join("icepy", "config", os.path.basename(x))):
	shutil.copyfile(x, os.path.join("icepy", "config", os.path.basename(x)))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icepy", "makedist.py"), \
    os.path.join("icepy", "makebindist.py"), \
    ]
filesToRemove.extend(find("icepy", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Generate HTML documentation. We need to build icecpp
# and slice2docbook first.
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
    os.chdir(os.path.join("ice", "src", "slice2docbook"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "doc"))
    os.system("gmake")
    os.chdir(cwd)
    os.mkdir(os.path.join("icepy", "doc"))
    os.rename(os.path.join("ice", "doc", "reference"), os.path.join("icepy", "doc", "reference"))
    os.rename(os.path.join("ice", "doc", "README.html"), os.path.join("icepy", "doc", "README.html"))
    os.rename(os.path.join("ice", "doc", "images"), os.path.join("icepy", "doc", "images"))

#
# Taken from ice/config/TestUtil.py
#
# If having this duplicated is really a problem we should split these
# methods out into their own module.
#
def isHpUx():

   if sys.platform == "hp-ux11":
        return 1
   else:
        return 0

def isDarwin():

   if sys.platform == "darwin":
        return 1
   else:
        return 0

def isAIX():
   if sys.platform in ['aix4', 'aix5']:
        return 1
   else:
        return 0

#
# Build slice2py.
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
    os.chdir(os.path.join("ice", "src", "slice2py"))
    os.system("gmake")
    os.chdir(cwd)

    os.environ["PATH"] = os.path.join(cwd, "ice", "bin") + ":" + os.getenv("PATH", "")

    if isHpUx():
	os.environ["SHLIB_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("SHLIB_PATH", "")
    elif isDarwin():
	os.environ["DYLD_LIBRARY_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("DYLD_LIBRRARY_PATH", "")
    elif isAIX():
	os.environ["LIBPATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("LIBPATH", "")
    else:
	os.environ["LD_LIBRARY_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")

    os.environ["ICE_HOME"] = os.path.join(cwd, "ice")

#
# Translate Slice files.
#
print "Generating Python code..."
cwd = os.getcwd()
os.chdir(os.path.join("icepy", "python"))
if verbose:
    quiet = ""
else:
    quiet = " -s"
os.system("gmake" + quiet)
os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("icepy", "config", "Make.rules"), "r")
version = re.search("^VERSION[ \t]+=[^\d]*([\d\.]+)", config.read(), re.M).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icepy", "README*"), version)
fixVersion(find("icepy", "INSTALL*"), version)

#
# Create source archives.
#
print "Creating distribution archives..."
icever = "IcePy-" + version
os.rename("icepy", icever)
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
shutil.copyfile(os.path.join(icever, "CHANGES"), "IcePy-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("ice")
print "Done."
