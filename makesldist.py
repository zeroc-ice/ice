#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, glob, time, fileinput
from stat import *
from shutil import copytree, rmtree

#
# This is an explicit list of some SL specific files to remove. The
# other files are all removed by reversing the below list.
#
filesToRemove = [
    "sl/test/Ice/proxy",
    "sl/test/IceCS",
    "sl/allTests.py",
    "sl/config/TestUtil.py",
    "sl/config/Make.rules.cs",
    "sl/config/makedepend.py",
    "sl/test/Makefile.mak",
]

# List of files & subdirectories to keep, all others are removed.
filesToKeep = [
    "./config/Make.common.rules.mak",
    "./config/IceDevKey.snk",
    "./cpp/Makefile.mak",
    "./cpp/config/Make.rules.mak.icesl",
    "./cpp/config/Make.rules.msvc",
    "./cpp/config/Makefile.mak",
    "./cpp/include/IceUtil",
    "./cpp/include/Slice",
    "./cpp/src/Makefile.mak",
    "./cpp/src/IceUtil",
    "./cpp/src/Slice",
    "./cpp/src/slice2sl",
    "./cpp/bin",
    "./cpp/lib",
    "./sl",
]

def pathInList(p, l):
    for f in l:
        # Slower, but more accurate.
	#if os.path.samefile(p, f):
        if p == f:
            return True
    return False

# This takes a list of files to keep, and generates from that a list
# of files to remove.
def genRemoveList(l):
    files = []
    dirs = []
    for root, dirnames, filenames in os.walk('.'):
        mod = []
        for d in dirnames:
            if pathInList(os.path.join(root, d), filesToKeep):
                mod.append(d)
	for e in mod:
	    del dirnames[dirnames.index(e)]

        for f in filenames:
            if not pathInList(os.path.join(root, f), filesToKeep):
                files.append(os.path.join(root, f))

        for f in dirnames:
            dirs.append(os.path.join(root, f))
    dirs.reverse()
    files.extend(dirs)
    return files

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] tag"
    print
    print "Options:"
    print "-h    Show this message."
    print "-v    Be verbose."

#
# Remove file or directory, warn if it doesn't exist.
#
def remove(path, recurse = True):

    if not os.path.exists(path):
        print "warning: " + path + " doesn't exist"
        return

    if os.path.isdir(path):
        if recurse:
            rmtree(path)
        else:
            try:
                os.rmdir(path)
            except:
                pass
    else:
        os.remove(path)

#
# Copy srcpath file to destpath
#
def copy(srcpath, destpath):

    if not os.path.exists(srcpath):
        print "warning: " + srcpath + " doesn't exist"
        return

    if os.path.isdir(destpath):
        destpath = os.path.join(destpath, os.path.basename(srcpath))

    if os.path.exists(destpath):
        print "warning: overwritting " + destpath

    shutil.copy(srcpath, destpath)
    fixFilePermission(destpath)

def replace(srcpath, destpath):

    if not os.path.exists(srcpath):
        print "warning: " + srcpath + " doesn't exist"
        return

    if os.path.isdir(destpath):
        destpath = os.path.join(destpath, os.path.basename(srcpath))

    shutil.copy(srcpath, destpath)
    fixFilePermission(destpath)


#
# Copy files from srcpath and matching the given patterns to destpath
#
def copyMatchingFiles(srcpath, destpath, patterns):
    for p in patterns:
        for f in glob.glob(os.path.join(srcpath, p)):
            copy(f, os.path.join(destpath, os.path.basename(f)))


#
# Get the language mapping directory for a given suffix.
#
def getMappingDir(suffix, mapping):
    if mapping == "cpp":
        return suffix
    elif mapping == "java":
        return suffix + "j"
    else:
        return suffix + mapping

#
# Comment out rules in a Makefile.
#
def fixMakefile(file, base, ext):

    origfile = file + ".orig"
    os.rename(file, origfile)
    oldMakefile = open(origfile, "r")
    newMakefile = open(file, "w")
    origLines = oldMakefile.readlines()

    doComment = 0
    doCheck = 0
    newLines = []
    for x in origLines:
        #
        # If the rule contains the target string, then
        # comment out this rule.
        #
        if not x.startswith("\t") and x.find(base + ext) != -1:
            doComment = 1
        #
        # If the line starts with "clean::", then check
        # the following lines and comment out any that
        # contain the target string.
        #
        elif x.startswith("clean::"):
            doCheck = 1
        #
        # Stop when we encounter an empty line.
        #
        elif len(x.strip()) == 0:
            doComment = 0
            doCheck = 0

        if doComment or (doCheck and x.find(base) != -1):
            x = "#" + x
        newLines.append(x)

    newMakefile.writelines(newLines)
    newMakefile.close()
    oldMakefile.close()
    os.remove(origfile)

#
# Comment out rules in VC project.
#
def fixProject(file, target):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldProject = open(origfile, "r")
    newProject = open(file, "w")
    origLines = oldProject.readlines()

    #
    # Find a Source File declaration containing SOURCE=<target>
    # and comment out the entire declaration.
    #
    expr = re.compile("SOURCE=.*" + target.replace(".", "\\.") + ".*")
    inSource = 0
    doComment = 0
    newLines = []
    source = []
    for x in origLines:
        if x.startswith("# Begin Source File"):
            inSource = 1

        if inSource:
            if not doComment and expr.match(x) != None:
                doComment = 1
            source.append(x)
        else:
            newLines.append(x)

        if x.startswith("# End Source File"):
            inSource = 0
            for s in source:
                if doComment:
                    newLines.append('#xxx#' + s)
                else:
                    newLines.append(s)
            doComment = 0
            source = []

    newProject.writelines(newLines)
    newProject.close()
    oldProject.close()
    os.remove(origfile)

#
# Comment out implicit parser/scanner rules in config/Make.rules.
#
def fixMakeRules(file):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    doComment = 0
    newLines = []
    for x in origLines:
        if x.find("%.y") != -1 or x.find("%.l") != -1:
            doComment = 1
        #
        # Stop when we encounter an empty line.
        #
        elif len(x.strip()) == 0:
            doComment = 0

        if doComment:
            x = "#" + x
        newLines.append(x)

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

#
# Fix version in README, INSTALL files
#
def fixFilePermission(file):

    patterns = [ \
        "*.h", \
        "*.cpp", \
        "*.ice", \
        "README*", \
        "INSTALL*", \
        "*.xml", \
        "*.mc", \
        "Makefile", \
        "Makefile.mak", \
        "*.dsp", \
        ]

    st = os.stat(file)

    if st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH):
        for p in patterns:
            if fnmatch.fnmatch(file, p):
                if verbose:
                    print "removing exec permissions on: " + file
                break
        else:
            os.chmod(file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
            return

    os.chmod(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) # rw-r--r--

#
# Generate bison files and comment out the Makefile rule
#
def generateBisonFile(file):

    #
    # Change to the directory containing the file.
    #
    (dir,file) = os.path.split(x)
    os.chdir(dir)
    (base,ext) = os.path.splitext(file)

    #
    # Run gmake to create the output files.
    #
    if verbose:
        quiet = ""
    else:
        quiet = "-s"
    if file == "cexp.y":
        os.system("gmake " + quiet + " cexp.c")
    else:
        os.system("gmake " + quiet + " " + base + ".cpp")

    #
    # Edit the Makefile to comment out the grammar rules.
    #
    fixMakefile("Makefile", base, ext)
    fixMakefile("Makefile.mak", base, ext)

    #
    # Edit the project file(s) to comment out the grammar rules.
    #
    for p in glob.glob("*.dsp"):
        fixProject(p, file)

    os.chdir(srcDistDir)

#
# Generate flex files and comment out the Makefile rule
#
def generateFlexFile(file):

    #
    # Change to the directory containing the file.
    #
    (dir,file) = os.path.split(file)
    os.chdir(dir)
    (base,ext) = os.path.splitext(file)

    #
    # Run gmake to create the output files.
    #
    if verbose:
        quiet = ""
    else:
        quiet = "-s"
    os.system("gmake " + quiet + " " + base + ".cpp")

    #
    # Edit the Makefile to comment out the flex rules.
    #
    fixMakefile("Makefile", base, ext)
    fixMakefile("Makefile.mak", base, ext)

    #
    # Edit the project file(s) to comment out the flex rules.
    #
    for p in glob.glob("*.dsp"):
        fixProject(p, file)

    os.chdir(srcDistDir)

def regexpEscape(expr):
    escaped = ""
    for c in expr:
        # TODO: escape more characters?
        if c in ".\\/":
            escaped += "\\" + c
        else:
            escaped += c
    return escaped
            

def substitute(file, regexps):
    for line in fileinput.input(file, True):
        for (expr, text) in regexps:
	    if not expr is re:
		expr = re.compile(expr)
            line = expr.sub(text, line)
        print line,

#
# Check arguments
#
verbose = 0
tag = "HEAD"
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
        tag = x

if verbose:
    quiet = "v"
else:
    quiet = ""

cwd = os.getcwd()
os.chdir(os.path.dirname(__file__))

#
# Get IceSL version.
#
config = open(os.path.join("sl", "src", "Ice", "AssemblyInfo.cs"), "r")
version = re.search("AssemblyVersion.*\"([0-9\.]*)\".*", config.read()).group(1)

#
# Remove any existing "distsl-M.m.p" directory and create a new one
# and sub-directories for the each source distribution.
#
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "distsl-" + version))
if os.path.exists(distDir):
    rmtree(distDir)
os.mkdir(distDir)

print "Creating " + version + " source distributions in " + distDir

srcDistDir = os.path.join(distDir, "IceSL-" + version)

#
# Extract the sources with git archive using the given tag.
#
print "Creating git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --prefix=IceSL-" + version + "/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
print "ok"

os.chdir(os.path.join(srcDistDir))

print "Fixing makefiles...",

for makeRulesName in [os.path.join("cpp", "config", "Make.rules.mak")]:
    fixMakeRules(makeRulesName)
    makeRules = open(makeRulesName, "r")
    lines = makeRules.readlines()
    makeRules.close()
    for i in range(len(lines)):
        if lines[i].find("prefix") == 0:
            lines[i] = lines[i].replace("IceSL-$(VERSION)", "IceSL-" + version)
    makeRules = open(makeRulesName, "w")
    makeRules.writelines(lines)
    makeRules.close()

#
# Change SUBDIRS and INSTALL_SUBDIRS in top-level Makefile.
#
for makeFileName in [os.path.join("cpp", "Makefile.mak")]:
    makeFile = open(makeFileName, "r")
    lines = makeFile.readlines()
    makeFile.close()
    for i in range(len(lines)):
        if lines[i].find("SUBDIRS") == 0:
            lines[i] = "SUBDIRS = src\n"
        if lines[i].find("INSTALL_SUBDIRS") == 0:
            lines[i] = "INSTALL_SUBDIRS = $(install_bindir) $(install_libdir)\n"
    makeFile = open(makeFileName, "w")
    makeFile.writelines(lines)
    makeFile.close()

#
# Disable install targets for libIceUtil, libSlice.
#
for makeFileName in [os.path.join("cpp", "src", "IceUtil", "Makefile.mak"), \
                     os.path.join("cpp", "src", "Slice", "Makefile.mak")]:
    makeFile = open(makeFileName, "r")
    lines = makeFile.readlines()
    makeFile.close()

    doComment = 0
    for i in range(len(lines)):
        if lines[i].find("install::") == 0:
            doComment = 1
        elif len(lines[i].strip()) == 0:
            doComment = 0
        elif doComment:
            lines[i] = "#" + lines[i]

    makeFile = open(makeFileName, "w")
    makeFile.writelines(lines)
    makeFile.close()

print "ok"

print "Walking through distribution to fix permissions, versions, etc...",
sys.stdout.flush()

bisonFiles = []
flexFiles = []
for root, dirnames, filesnames in os.walk('.'):

    for f in filesnames:
        filepath = os.path.join(root, f) 
        if f == ".gitignore":
            os.remove(filepath)
        else:

            # Fix version of README/INSTALL files and keep track of bison/flex files for later processing
            if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
                substitute(filepath, [("@ver@", version)])
            elif fnmatch.fnmatch(f, "*.y"):
                bisonFiles.append(filepath)
            elif fnmatch.fnmatch(f, "*.l"):
                flexFiles.append(filepath)

            fixFilePermission(filepath)
    
    for d in dirnames:
        os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

# Fix the version on cpp/include/IceUtil/Config.h
#
# Note that this "fixes" ICE_STRING_VERSION, but not ICE_INT_VERSION
# since that is only used by the C++ code generators.
#
substitute("cpp/include/IceUtil/Config.h",
    [('ICE_STRING_VERSION "([0-9a-z\.]+)"', 'ICE_STRING_VERSION "%s"' % version)])
substitute("config/Make.common.rules.mak", [('^VERSION\s+=\s*[a-z0-9\.]+', 'VERSION = %s' % version)])

print "ok"

#
# Generate bison & flex files.
#
print "Generating bison and flex files...",
sys.stdout.flush()
for x in bisonFiles:
    generateBisonFile(x)
for x in flexFiles:
    generateFlexFile(x)
print "ok"

#
# Remove or move non-public files out of source distribution.
#
print "Removing non-Ice directories and files...",
sys.stdout.flush()
for x in genRemoveList(filesToKeep):
    remove(x, False)

for x in filesToRemove:
    remove(x)

for root, dirnames, filesnames in os.walk('.'):
    for f in filesnames:
        if f == "Makefile":
            os.remove(os.path.join(root, f))
print "ok"

#
# Copy IceSL specific install files.
#
print "Copying icesl install files...",
shutil.move(os.path.join("sl", "ICE_LICENSE"), os.path.join("ICE_LICENSE"))
shutil.move(os.path.join("sl", "LICENSE"), os.path.join("LICENSE"))
shutil.move(os.path.join("sl", "README.txt"), os.path.join("README.txt"))
shutil.move(os.path.join("sl", "INSTALL.txt"), os.path.join("INSTALL.txt"))

#
# Move *.icesl to the correct names.
#
shutil.move(os.path.join("cpp", "config", "Make.rules.mak.icesl"), os.path.join("cpp", "config", "Make.rules.mak"))

print "ok"

#
# Everything should be clean now, we can create the source distributions archives
# 
print "Archiving..."
sys.stdout.flush()
os.chdir(distDir)

for d in [srcDistDir]:
    dist = os.path.basename(d)
    print "   creating " + dist + ".tar.gz ...",
    sys.stdout.flush()
    os.system("tar c" + quiet + "f - " + dist + " | gzip -9 - > " + dist + ".tar.gz")
    print "ok"

for d in [srcDistDir]:
    dist = os.path.basename(d)
    print "   creating " + dist + ".zip ...",
    sys.stdout.flush()
    if verbose:
        os.system("zip -9r " + dist + ".zip " + dist)
    else:
        os.system("zip -9rq " + dist +".zip " + dist)
    print "ok"

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
#rmtree(srcDistDir)
print "ok"

os.chdir(cwd)
