#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, glob, time
from stat import *
from shutil import copytree, rmtree

#
# Sub-directories to keep to create the source distributions.
#
includeSubDirs = [ \
    "cpp", \
    "java", \
    "cs", \
    "php", \
    "py", \
    "vb", \
    "rb", \
    "config", \
    "certs", \
    "slice", \
    "distribution", \
    "demoscript", \
]

#
# Files to not include in the source distributions.
#
filesToRemove = [ \
    "fixCopyright.py", \
    "fixVersion.py", \
    "makedist.py", \
    "cpp/iceemakedist.py", \
    "cpp/iceslmakedist.py", \
    "cpp/config/makegitignore.py", \
    "cpp/config/Make.rules.icee", \
    "cpp/config/Make.rules.mak.icee", \
#    "cpp/config/Make.rules.icesl", \
    "cpp/config/Make.rules.mak.icesl", \
    "cpp/src/slice2cppe", \
    "cpp/src/slice2javae", \
    "cpp/src/slice2sl", \
    "rb/config/Make.rules.Darwin", \
]

#
# Files from the top-level, cpp, java and cs config directories to include in the demo 
# source distribution config directory.
#
configFiles = [ \
    "Make.*", \
    "common.xml", \
    "build.properties", \
]

#
# Files from the top-level certs directory to include in the demo distribution certs
# directory.
#
certsFiles = [ \
    "*.jks", \
    "*.pem", \
    "*.pfx", \
]

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
def remove(path):

    if not os.path.exists(path):
        print "warning: " + path + " doesn't exist"
        return

    if os.path.isdir(path):
        rmtree(path)
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

    shutil.copyfile(srcpath, destpath)
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
def fixVersion(file):

    global version, mmversion, libversion

    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    line = oldFile.read();
    line = re.sub("@ver@", version, line)
    line = re.sub("@mmver@", mmversion, line)
    line = re.sub("@libver@", libversion, line)
    newFile.write(line)
    newFile.close()
    oldFile.close()

    # Preserve the executable permission
    st = os.stat(origfile)
    if st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH): 
        os.chmod(file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
    os.remove(origfile)

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
# Get Ice version.
#
config = open(os.path.join("config", "Make.common.rules"), "r")
version = re.search("VERSION\s*=\s*([0-9\.b]*)", config.read()).group(1)
mmversion = re.search("([0-9]+\.[0-9b]+)[\.0-9]*", version).group(1)
libversion = mmversion.replace('.', '')
config.close()

#
# Remove any existing "dist-M.m.p" directory and create a new one
# and sub-directories for the each source distribution.
#
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "dist-" + version))
if os.path.exists(distDir):
    rmtree(distDir)
os.mkdir(distDir)

print "Creating " + version + " source distributions in " + distDir

demoscriptDistDir = os.path.join(distDir, "Ice-" + version + "-demo-scripts")
demoDistDir = os.path.join(distDir, "Ice-" + version + "-demos")
srcDistDir = os.path.join(distDir, "Ice-" + version)
os.mkdir(demoscriptDistDir)
os.mkdir(demoDistDir)
os.mkdir(os.path.join(demoDistDir, "config"))
os.mkdir(os.path.join(demoDistDir, "certs"))

#
# Extract the sources with git archive using the given tag.
#
print "Creating git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --prefix=Ice-" + version + "/ " + tag + " | ( cd " + distDir + " && tar xf - )")
print "ok"

os.chdir(os.path.join(srcDistDir))

#
# Remove or move non-public files out of source distribution.
#
print "Removing non-Ice directories and files...",
sys.stdout.flush()
for x in filesToRemove:
    remove(x)
for d in os.listdir('.'):
    if not d in includeSubDirs:
        if os.path.isdir(d):
            rmtree(d)
print "ok"

print "Walking through distribution to fix permissions, versions, etc...",
sys.stdout.flush()

fixVersion(os.path.join("distribution", "bin", "makebindist.py"))
fixVersion(os.path.join("cpp", "config", "glacier2router.cfg"))
fixVersion(os.path.join("cpp", "config", "icegridregistry.cfg"))
fixVersion(os.path.join("distribution", "src", "rpm", "glacier2router.conf"))
fixVersion(os.path.join("distribution", "src", "rpm", "icegridregistry.conf"))

bisonFiles = []
flexFiles = []
for root, dirnames, filesnames in os.walk('.'):

    for f in filesnames:
        filepath = os.path.join(root, f) 
        if f == ".gitignore":
            os.remove(filepath)
        elif f == "expect.py":
            if not os.path.exists(os.path.join(distDir, demoscriptDistDir, root)):
                os.makedirs(os.path.join(distDir, demoscriptDistDir, root))
            copy(filepath, os.path.join(distDir, demoscriptDistDir, filepath))
            os.remove(filepath)
        else:

            # Fix version of README/INSTALL files and keep track of bison/flex files for later processing
            if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
                fixVersion(filepath)
            elif fnmatch.fnmatch(f, "*.y"):
                bisonFiles.append(filepath)
            elif fnmatch.fnmatch(f, "*.l"):
                flexFiles.append(filepath)

            fixFilePermission(filepath)
    
    for d in dirnames:
        os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

os.rename("distribution", os.path.join("..", "distribution")) # Move the distribution directory to the top-level
os.rename("demoscript", os.path.join(demoscriptDistDir, "demoscript")) # Move the demoscript directory

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
fixMakeRules(os.path.join("cpp", "config", "Make.rules"))
print "ok"

#
# Consolidate demo, demo scripts distributions.
#
print "Consolidating demo and demo scripts distributions...",
sys.stdout.flush()

# Demo distribution
copy("ICE_LICENSE", demoDistDir)
copy(os.path.join(distDir, "distribution", "src", "common", "README.DEMOS"), os.path.join(demoDistDir))

copyMatchingFiles(os.path.join("certs"), os.path.join(demoDistDir, "certs"), certsFiles)
copyMatchingFiles(os.path.join("config"), os.path.join(demoDistDir, "config"), configFiles)
copyMatchingFiles(os.path.join("cpp", "config"), os.path.join(demoDistDir, "config"), configFiles)
copyMatchingFiles(os.path.join("java", "config"), os.path.join(demoDistDir, "config"), configFiles)
copyMatchingFiles(os.path.join("cs", "config"), os.path.join(demoDistDir, "config"), configFiles)

# Consolidate demoscript and demo distribution with files from each language mapping
for d in os.listdir('.'):

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "allDemos.py")):
        os.rename(os.path.join(d, "allDemos.py"), os.path.join(demoscriptDistDir, d, "allDemos.py"))
        os.rename(os.path.join(demoscriptDistDir, d), os.path.join(demoscriptDistDir, getMappingDir("demo", d)))

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        copytree(os.path.join(d, "demo"), os.path.join(demoDistDir, getMappingDir("demo", d)))

# Remove Windows project files and configuration files from the demo distribution
for root, dirnames, filesnames in os.walk(demoDistDir):
    for f in filesnames:
        for m in [ "*.dsp", "*.dsw", "*.sln", "*.csproj", "*.vbproj", "*.exe.config"]:
            if fnmatch.fnmatch(f, m):
                os.remove(os.path.join(root, f))

print "ok"

#
# Copy CHANGES and RELEASE_NOTES.txt
#
copy(os.path.join(srcDistDir, "cpp", "CHANGES"), os.path.join(distDir, "Ice-" + version + "-CHANGES"))
copy(os.path.join(distDir, "distribution", "src", "common", "RELEASE_NOTES.txt"), distDir)

#
# Everything should be clean now, we can create the source distributions archives
# 
print "Archiving..."
sys.stdout.flush()
os.chdir(distDir)
os.rename("distribution", "distfiles-" + version)
for d in [srcDistDir, demoDistDir, demoscriptDistDir, "distfiles-" + version]:
    dist = os.path.basename(d)
    print "   creating " + dist + ".tar.gz ...",
    sys.stdout.flush()
    os.system("tar c" + quiet + "f - " + dist + " | gzip -9 - > " + dist + ".tar.gz")
    print "ok"

for d in [srcDistDir, demoDistDir]:
    dist = os.path.basename(d)
    print "   creating " + dist + ".zip ...",
    sys.stdout.flush()
    if verbose:
        os.system("zip -9r " + dist + ".zip " + dist)
    else:
        os.system("zip -9rq " + dist +".zip " + dist)
    print "ok"

readme = open("README.txt", "w")
print >>readme, "This directory contains the source distributions of Ice " + version + ".\n"
print >>readme, "Creation time: " + time.strftime("%a %b %d %Y, %I:%M:%S %p (%Z)")
(sysname, nodename, release, ver, machine) = os.uname();
print >>readme, "Host: " + nodename
print >>readme, "Platform: " + sysname + " " + release
if os.path.exists("/etc/redhat-release"):
    f = open("/etc/redhat-release")
    print >>readme, "Linux distribution: " + f.readline().strip()
    f.close()
else:
    print >>readme, "Not created on a Linux distribution"
print >>readme, "User: " + os.environ["USER"]
print >>readme, ""
print >>readme, "" + \
"The archive distfiles-" + version + ".tar.gz contains the sources for building the\n" + \
"binary distributions.\n"
readme.close()

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
rmtree(os.path.join(distDir, "distfiles-" + version))
rmtree(srcDistDir)
rmtree(demoDistDir)
rmtree(demoscriptDistDir)
print "ok"

os.chdir(cwd)
