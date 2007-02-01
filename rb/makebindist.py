#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
# Check for ICE_HOME.
#
if not os.environ.has_key("ICE_HOME"):
    print "ICE_HOME must be defined."
    sys.exit(1)
iceHome = os.environ["ICE_HOME"]

#
# Remove any existing "bindist" directory and create a new one.
#
distdir = "bindist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)

#
# Export IceRuby sources from CVS.
#
print "Checking out Ruby sources using CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag + " icerb")

#
# Validate versions.
#
config = open(os.path.join("icerb", "config", "Make.rules"), "r")
version = re.search("^VERSION[ \t]+=[^\d]*([\d\.]+)", config.read(), re.M).group(1)
config = open(os.path.join(iceHome, "include", "IceUtil", "Config.h"), "r")
iceVersion = re.search("ICE_STRING_VERSION[ \t]+\"([\d\.]+)\"", config.read(), re.M).group(1)
if version != iceVersion:
    print "Version mismatch between icerb (" + version + ") and ice (" + iceVersion + ")!"
    sys.exit(1)

uncomment(os.path.join("icerb", "config", "Make.rules"), "^#OPTIMIZE")

os.environ["LD_LIBRARY_PATH"] = os.path.join(iceHome, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")
os.environ["PATH"] = os.path.join(iceHome, "bin") + ":" + os.getenv("PATH", "")

cwd = os.getcwd()
os.chdir("icerb")
os.system("gmake")
os.chdir(cwd)

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
filesToRemove.extend(find("icerb", ".dummy"))
for x in filesToRemove:
    os.remove(x)

shutil.rmtree(os.path.join("icerb", "config"))
shutil.rmtree(os.path.join("icerb", "src"))
shutil.rmtree(os.path.join("icerb", "test"))


#
# Create archives.
#
print "Creating distribution archives..."
icever = "IceRuby-" + version
archive = icever + "-bin-fc5"
os.rename("icerb", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icever)
os.system("find " + icever + " -type d -exec chmod a+x {} \\;")
os.system("find " + icever + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar c" + quiet + "f " + archive + ".tar " + icever)
os.system("gzip -9 " + archive + ".tar")
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9ry " + quiet + " " + archive + ".zip " + icever)

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
print "Done."
