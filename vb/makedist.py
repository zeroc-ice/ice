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
os.chdir(distdir)

#
# Export VB and C++ sources from CVS.
#
# NOTE: Assumes that the C++ and C# trees will use the same tag.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export " + tag +
          " icevb icecs/src/Ice/AssemblyInfo.cs")

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icevb", "makedist.py"), \
    ]
filesToRemove.extend(find("icevb", ".dummy"))
for x in filesToRemove:
    os.remove(x)

#
# Get Ice version.
#
config = open(os.path.join("icecs", "src", "Ice", "AssemblyInfo.cs"), "r")
version = re.search("AssemblyVersion.*\"([0-9\.]*)\"", config.read()).group(1)
checkBeta = version.split('.')
if int(checkBeta[2]) > 50:
    version = "%s.%sb" % (checkBeta[0], checkBeta[1])
    beta = int(checkBeta[2]) - 50
    if beta > 1:
        version = version + str(beta)

print "Fixing version in README and INSTALL files..."
fixVersion(find("icevb", "README*"), version)
fixVersion(find("icevb", "INSTALL*"), version)

#
# Fix source dist demo project files.
#
hintPathSearch = r'(HintPath = "(\.\.\\)*)icecs(\\bin\\.*cs\.dll")'
hintPathReplace = r'\1IceCS-' + version + r'\3'
projectFiles = find(os.path.join("icevb", "demo"), "*.vbproj")
for x in projectFiles:
    if not x.endswith("D.vbproj"):
        sedFile(x, hintPathSearch, hintPathReplace)

#
# Create source archives.
#
print "Creating distribution archives..."
icever = "IceVB-" + version
os.rename("icevb", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r-w . " + icever)
os.system("find " + icever + " \\( -name \"*.vb\" -or -name \"*.ice\" \\) -exec chmod a-x {} \\;")
os.system("find " + icever + " \\( -name \"README*\" -or -name \"INSTALL*\" \\) -exec chmod a-x {} \\;")
os.system("find " + icever + " \\( -name \"*.xml\" \\) -exec chmod a-x {} \\;")
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
shutil.copyfile(os.path.join(icever, "CHANGES"), "IceVB-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("icecs")
print "Done."
