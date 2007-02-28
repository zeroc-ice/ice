#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h       Show this message."
    print "-t       Skip building translators and use the ones in PATH."
    print "-f       Keep going if precondition checks fail."
    print "-v       Be verbose."
    print
    print "If no tag is specified, HEAD is used."

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
if sys.platform.startswith("win") or sys.platform.startswith("cygwin"):
    print sys.argv[0] + ": this script must be run on a Unix platform."
    sys.exit(1)

#
# Check arguments
#
tag = "-rHEAD"
skipTranslators = False 
verbose = False 
keepGoing = False
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-t":
        print "skipping translators"
        skipTranslators = True
    elif x == "-v":
        verbose = True 
    elif x == "-f":
        keepGoing = True
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
# Export Java and C++ sources from CVS.
#
# NOTE: Assumes that the C++ and Java trees will use the same tag.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
          " icej ice/bin ice/config ice/include ice/lib ice/slice ice/src")

#
# Check known preconditions for proper distribution building. Failed
# checks do not result in immediate failure. Error messages are
# displayed and the precondition checks continue to aide in identifying
# other problems. If any of the precondition checks fail, the script
# terminates.
#
f = file(os.path.join("icej", "config", "build.properties"))
buildProperties = f.readlines();
f.close()

errorOut = False
for p in buildProperties:
    checkFilename = None
    d = p.split("=")
    if d[0].strip() in ["jgoodies.looks", "jgoodies.forms", "berkeleydb.jar"]:
        if not os.path.exists(d[1].strip()):
            print "ERROR: %s is not in configured location. IceGridGUI.jar will not build correctly!" % d[1].strip()
            errorOut = True

if not os.environ.has_key("CLASSPATH"):
    print "ERROR: No CLASSPATH, unable to find ProGuard jar file."
    errorOut = True
else:
    classpath = os.environ["CLASSPATH"]
    found = False
    for e in classpath.split(os.pathsep):
        if e.find("proguard.jar") != -1:
            if os.path.exists(e):
                found = True
                break
    if not found:
        print "ERROR: Unable to find ProGuard in CLASSPATH"
        errorOut = True

if errorOut:
    print "Failed precondition checks! See above messages."
    if not keepGoing:
        sys.exit(1)

#
# Copy Slice directories.
#
print "Copying Slice directories..."
slicedirs = [
    "Freeze",
    "Glacier2",
    "Ice",
    "IceBox",
    "IcePatch2",
    "IceStorm",
    "IceGrid"
]
os.mkdir(os.path.join("icej", "slice"))
for x in slicedirs:
    shutil.copytree(os.path.join("ice", "slice", x), os.path.join("icej", "slice", x), 1)

#
# Build slice2java and slice2freezej.
#
if not skipTranslators:
    print "Building translators..."
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
    os.chdir(os.path.join("ice", "src", "slice2java"))
    os.system("gmake")
    os.chdir(cwd)
    os.chdir(os.path.join("ice", "src", "slice2freezej"))
    os.system("gmake")
    os.chdir(cwd)

    os.environ["PATH"] = os.path.join(cwd, "ice", "bin") + ":" + os.getenv("PATH", "")

    if isHpUx():
        os.environ["SHLIB_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("SHLIB_PATH", "")
    elif isDarwin():
        os.environ["DYLD_LIBRARY_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("DYLD_LIBRARY_PATH", "")
    elif isAIX():
        os.environ["LIBPATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("LIBPATH", "")
    else:
        os.environ["LD_LIBRARY_PATH"] = os.path.join(cwd, "ice", "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")

    if os.environ.has_key("ICE_HOME"):
        del os.environ["ICE_HOME"]

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icej", "makedist.py"), \
    ]
filesToRemove.extend(find("icej", ".dummy"))
for x in filesToRemove:
    os.remove(x)

cwd = os.getcwd()
os.chdir("icej")

#
# Build sources.
#
print "Compiling Java sources..."

if verbose:
    quiet = ""
else:
    quiet = " -q"
os.system("ant" + quiet)

distroSuffix = "java2"
#
# Clean out the lib directory but save the jar files.
#
os.rename(os.path.join("lib", "Ice.jar"), "Ice.jar")
if os.path.exists(os.path.join("lib", "IceGridGUI.jar")):
    print "Found IceGridGUI, is this the Java 2 targeted source distro?"
    os.rename(os.path.join("lib", "IceGridGUI.jar"), "IceGridGUI.jar")
else:
    print "No IceGridGUI, is this the Java 5 targeted source distro?"
    distroSuffix = "java5"

shutil.rmtree("lib")
os.mkdir("lib")
os.rename("Ice.jar", os.path.join("lib", "Ice.jar"))
if os.path.exists(os.path.join("IceGridGUI.jar")):
    os.rename("IceGridGUI.jar", os.path.join("lib", "IceGridGUI.jar"))
else:
    os.remove("THIRD_PARTY_LICENSE")
    os.remove("THIRD_PARTY_SOURCES")

#
# Remove "generated" subdirectories.
#
filesToRemove = find(".", "*generated") # generated, cgenerated, sgenerated
for x in filesToRemove:
    shutil.rmtree(x)

#
# Remove other unnecessary subdirectories.
#
#shutil.rmtree("admin")
shutil.rmtree("depcache")

os.chdir(cwd)

#
# Get Ice version.
#
config = open(os.path.join("icej", "src", "IceUtil", "Version.java"), "r")
version = re.search("ICE_STRING_VERSION = \"([0-9\.b]*)\"", config.read()).group(1)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icej", "README*"), version)
fixVersion(find("icej", "INSTALL*"), version)

#
# Create source archives.
#
print "Creating distribution archives..."
icever = "IceJ-" + version + "-" + distroSuffix
os.rename("icej", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icever)
os.system("find " + icever + " \\( -name \"*.java\" -or -name \"*.ice\" \\) -exec chmod a-x {} \\;")
os.system("find " + icever + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
os.system("find " + icever + " \\( -name \"*.xml\" -or -name \"*.html\" \\) -exec chmod a-x {} \\;")
os.system("find " + icever + " -type d -exec chmod a+x {} \\;")
os.system("find " + icever + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar c" + quiet + "zf " + icever + ".tar.gz " + icever)
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9 -r " + quiet + " " + icever + ".zip " + icever)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icever, "CHANGES"), "IceJ-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("ice")
print "Done."
