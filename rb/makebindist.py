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
# Modify the given Make.rules file to uncomment a line containing the given pattern.
#
def uncomment(file, patt):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    expr = re.compile(patt)
    doComment = 0
    newLines = []
    for x in origLines:
	if expr.match(x) != None and x[0] == '#':
	    newLines.append(x[1:])
	else:
	    newLines.append(x)

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

#
# Modify the given Make.rules file to add a flag if ICE_HOME is defined.
#
def fixMakeRules(file, patt, text):
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
	    newLines.append('ifneq ($(ICE_HOME),)\n')
	    newLines.append(x.rstrip() + text + "\n")
	    newLines.append('else\n')
	    newLines.append(x)
	    newLines.append('endif\n')
	else:
	    newLines.append(x)

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

#
# Modify the given Make.rules file to insert text in a variable value.
#
def fixMakeVar(file, patt, text):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    expr = re.compile(patt)
    doComment = 0
    newLines = []
    for x in origLines:
	m = expr.match(x)
	if m != None:
	    newLines.append(m.group() + text + x[m.end():])
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
# We need to provide slice2rb and libSliceRuby. The statements below export sources
# from branch icerb_preview_branch.
#
print "Checking out translator sources using CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export -ricerb_preview_branch" +
          " ice/config ice/slice ice/include ice/src/Makefile ice/src/IceUtil ice/src/icecpp ice/src/Slice" +
	  " ice/src/SliceRuby ice/src/slice2rb ice/src/slice2cpp ice/src/Ice")

os.mkdir(os.path.join("ice", "bin"))
os.mkdir(os.path.join("ice", "lib"))

uncomment(os.path.join("ice", "config", "Make.rules"), "^#OPTIMIZE")

cwd = os.getcwd()
os.chdir(os.path.join("ice", "src"))
os.system("gmake")
os.chdir(cwd)

#
# Export IceRuby sources from CVS.
#
print "Checking out IceRuby sources using CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag + " icerb")

uncomment(os.path.join("icerb", "config", "Make.rules"), "^#OPTIMIZE")

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

icePath = os.path.abspath("ice")

if isHpUx():
    os.environ["SHLIB_PATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("SHLIB_PATH", "")
elif isDarwin():
    os.environ["DYLD_LIBRARY_PATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("DYLD_LIBRRARY_PATH", "")
elif isAIX():
    os.environ["LIBPATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("LIBPATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")

os.environ["PATH"] = os.path.join(icePath, "bin") + ":" + os.getenv("PATH", "")

os.environ["ICE_HOME"] = icePath

cwd = os.getcwd()
os.chdir("icerb")
os.system("gmake")
os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("ice", "include", "IceUtil", "Config.h"), "r")
version = re.search("ICE_STRING_VERSION[ \t]+\"([\d\.]+)\"", config.read(), re.M).group(1)

os.rename(os.path.join("icerb", "README.Linux"), os.path.join("icerb", "README"))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icerb", "allTests.py"), \
    os.path.join("icerb", "makedist.py"), \
    os.path.join("icerb", "makebindist.py"), \
    os.path.join("icerb", "makewindist.py"), \
    ]
filesToRemove.extend(find("icerb", "*.o"))
filesToRemove.extend(find("icerb", "*.dsp"))
filesToRemove.extend(find("icerb", "*.dsw"))
filesToRemove.extend(find("icerb", "Makefile"))
filesToRemove.extend(find("icerb", "*.mak"))
filesToRemove.extend(find("icerb", "README.*"))
filesToRemove.extend(find("icerb", "INSTALL.*"))
filesToRemove.extend(find(os.path.join("ice", "bin"), "icecpp"))
filesToRemove.extend(find(os.path.join("ice", "bin"), "slice2cpp"))
filesToRemove.extend(find(os.path.join("ice", "lib"), "libIce*"))
filesToRemove.extend(find(os.path.join("ice", "lib"), "libSlice.*"))
for x in filesToRemove:
    os.remove(x)

shutil.rmtree(os.path.join("icerb", "config"))
shutil.rmtree(os.path.join("icerb", "src"))
shutil.rmtree(os.path.join("icerb", "test"))


#
# Create archives.
#
print "Creating distribution archives..."
icever = "IceRuby-" + version + "-bin-fc5"
os.rename("icerb", icever)
os.rename(os.path.join("ice", "bin"), os.path.join(icever, "bin"))
os.rename(os.path.join("ice", "lib"), os.path.join(icever, "lib"))
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
os.system("zip -9ry " + quiet + " " + icever + ".zip " + icever)

#
# Done.
#
print "Cleaning up..."
#shutil.rmtree(icever)
#shutil.rmtree("ice")
print "Done."
