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
# We need to provide slice2rb and libSlice. The statements below export sources
# from version 3.1.0 and then add the Ruby-related code from ice/HEAD.
#
print "Checking out translator sources using CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
#os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
#          " ice/config ice/include/Slice ice/src/Makefile ice/src/Slice ice/src/slice2rb")
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export -rR3_1_0" +
          " ice/config ice/slice ice/include/Slice ice/src/Makefile ice/src/Slice")
os.remove(os.path.join("ice", "src", "Makefile"))
os.remove(os.path.join("ice", "src", "Slice", "Makefile"))
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export -rHEAD ice/include/Slice/RubyUtil.h" +
	  " ice/src/Makefile ice/src/Slice/Makefile ice/src/Slice/RubyUtil.cpp ice/src/slice2rb")

os.mkdir(os.path.join("ice", "bin"))
os.mkdir(os.path.join("ice", "lib"))

fixMakeRules(os.path.join("ice", "config", "Make.rules"), "^CPPFLAGS[ \\t]*=", " -I$(ICE_HOME)/include")
fixMakeRules(os.path.join("ice", "config", "Make.rules"), "^LDFLAGS[ \\t]*=", " -L$(ICE_HOME)/lib")
uncomment(os.path.join("ice", "config", "Make.rules"), "^#OPTIMIZE")

cwd = os.getcwd()
os.chdir(os.path.join("ice", "src"))
os.system("gmake depend")
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

icePath = os.path.abspath("ice")
fixMakeVar(os.path.join("icerb", "config", "Make.rules"), "^CPPFLAGS[ \\t]*=", " -I" + icePath + "/include")
fixMakeVar(os.path.join("icerb", "config", "Make.rules"), "^LDFLAGS[ \\t]*=", " -L" + icePath + "/lib")
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

if isHpUx():
    os.environ["SHLIB_PATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("SHLIB_PATH", "")
elif isDarwin():
    os.environ["DYLD_LIBRARY_PATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("DYLD_LIBRRARY_PATH", "")
elif isAIX():
    os.environ["LIBPATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("LIBPATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = os.path.join(icePath, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")

os.environ["PATH"] = os.path.join(icePath, "bin") + ":" + os.getenv("PATH", "")

for x in glob.glob(os.path.join("ice", "config", "Make.rules.*")):
    if not os.path.exists(os.path.join("icerb", "config", os.path.basename(x))):
	shutil.copyfile(x, os.path.join("icerb", "config", os.path.basename(x)))

os.rename(os.path.join("ice", "slice"), os.path.join("icerb", "slice"))

cwd = os.getcwd()
os.chdir("icerb")
os.system("gmake")
os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("ice", "config", "Make.rules"), "r")
version = re.search("^VERSION[ \t]+=[^\d]*([\d\.]+)", config.read(), re.M).group(1)

shutil.rmtree(os.path.join("ice", "config"))
shutil.rmtree(os.path.join("ice", "include"))
shutil.rmtree(os.path.join("ice", "src"))
shutil.rmtree(os.path.join("icerb", "src"))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icerb", "makebindist.py"), \
    ]
filesToRemove.extend(find("icerb", "*.o"))
filesToRemove.extend(find("icerb", "*.dsp"))
filesToRemove.extend(find("icerb", "*.dsw"))
for x in filesToRemove:
    os.remove(x)

#
# Create archives.
#
print "Creating distribution archives..."
icever = "IceRuby-" + version
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
shutil.rmtree(icever)
shutil.rmtree("ice")
print "Done."
