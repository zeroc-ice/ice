#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, fileinput

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-v    Be verbose."

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
# Do a search and replace on a file using regular expressions a la sed.
#
def sedFile(path, patt, replace):
    src = open(path, "r")
    srcLines = src.readlines()

    dstLines = []
    for x in srcLines:
        dstLines.append(re.sub(patt, replace, x))

    src.close()
    dst = open(path, "w")
    dst.writelines(dstLines)

def editMakefileMak(file):
    makefile =  fileinput.input(file, True)
    for line in makefile:
        if line.startswith('!include'):
            print '!include $(top_srcdir)/config/Make.rules.mak.vb'
        else:
            print line.rstrip('\n')
    makefile.close()

#
# Are we on Windows?
#
win32 = sys.platform.startswith("win") or sys.platform.startswith("cygwin")

if os.path.exists("../.git"):
    print "Unable to run in repository! Exiting..."
    sys.exit(1)
    
#
# Check arguments
#
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

if win32 and not skipDocs:
    print sys.argv[0] + ": the documentation cannot be built on Windows."
    sys.exit(1)

#
# Remove any existing "dist" directory and create a new one.
#
distdir = "dist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)

icecsdir = os.path.join(os.getcwd(), "..", "cs")

#
# Get Ice version.
#
config = open(os.path.join(icecsdir, "src", "Ice", "AssemblyInfo.cs"), "r")
version = re.search("AssemblyVersion.*\"([0-9\.]*)\"", config.read()).group(1)
checkBeta = version.split('.')
if int(checkBeta[2]) > 50:
    version = "%s.%sb" % (checkBeta[0], checkBeta[1])
    beta = int(checkBeta[2]) - 50
    if beta > 1:
        version = version + str(beta)

icevbver = "IceVB-" + version
        
os.mkdir(os.path.join(distdir, icevbver))
 
if verbose:
    quiet = "-v"
else:
    quiet = ""

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ "makedist.py", "exclusions", "dist", "allDemos.py" ]
filesToRemove.extend(find(".", ".gitignore"))
filesToRemove.extend(find(".", "expect.py"))

exclusionFile = open("exclusions", "w")
for x in filesToRemove:
   exclusionFile.write("%s\n" % x)
exclusionFile.close()

os.system("tar c" + quiet + " -X exclusions . | ( cd " + os.path.join(distdir, icevbver) + " && tar xf - )")

os.chdir(distdir)

print "Fixing version in README and INSTALL files..."
fixVersion(find(icevbver, "README*"), version)
fixVersion(find(icevbver, "INSTALL*"), version)

#
# Fix source dist demo project files.
#
hintPathSearch = r'(HintPath = "(\.\.\\)*)icecs(\\bin\\.*cs\.dll")'
hintPathReplace = r'\1IceCS-' + version + r'\3'
projectFiles = find(os.path.join(icevbver, "demo"), "*.vbproj")
for x in projectFiles:
    if not x.endswith("D.vbproj"):
        sedFile(x, hintPathSearch, hintPathReplace)

#
# Create source archives.
#
print "Creating distribution archives..."
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icevbver)
os.system("find " + icevbver + " \\( -name \"*.vb\" -or -name \"*.ice\" \\) -exec chmod a-x {} \\;")
os.system("find " + icevbver + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
os.system("find " + icevbver + " \\( -name \"*.xml\" \\) -exec chmod a-x {} \\;")
os.system("find " + icevbver + " -type d -exec chmod a+x {} \\;")
os.system("find " + icevbver + " -perm +111 -exec chmod a+x {} \\;")
os.system("tar c" + quiet + "zf " + icevbver + ".tar.gz " + icevbver)
if verbose:
    quiet = ""
else:
    quiet = "-q"
os.system("zip -9 -r " + quiet + " " + icevbver + ".zip " + icevbver)

#
# Copy files (README, etc.).
#
shutil.copyfile(os.path.join(icevbver, "CHANGES"), "IceVB-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icevbver)
print "Done."
