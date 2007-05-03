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
# Program usage.
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
# Comment out rules in a Makefile.
#
def fixMakefile(file, target):
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
        if not x.startswith("\t") and x.find(target) != -1 and x.find(target + ".o") == -1:
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

        if doComment or (doCheck and x.find(target) != -1):
            x = "#" + x
        newLines.append(x)

    newMakefile.writelines(newLines)
    newMakefile.close()
    oldMakefile.close()
    os.remove(origfile)

#
# Remove lines containing a keyword from a file.
#
def editFile(file, target):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    newLines = []
    for x in origLines:
        if x.find(target) == -1:
            newLines.append(x)

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
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
# Remove any existing "dist" directory and create a new one.
#
distdir = "dist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.chdir(distdir)

#
# Export sources from CVS.
#
print "Checking out CVS tag " + tag + "..."
if verbose:
    quiet = ""
else:
    quiet = "-Q"
os.system("cvs " + quiet + " -d cvs.zeroc.com:/home/cvsroot export -l " + tag + " " +
          "ice " +
          "ice/bin " +
          "ice/config/Make.rules.icee " +
          "ice/config/Make.rules.mak.icee " +
          "ice/config/Make.rules.Linux " +
          "ice/config/Make.rules.msvc " +
          "ice/include " +
          "ice/include/IceUtil " +
          "ice/include/Slice " +
          "ice/install " +
          "ice/install/icee " +
          "ice/lib " +
          "ice/src " +
          "ice/src/icecpp " +
          "ice/src/IceUtil " +
          "ice/src/Slice " +
          "ice/src/Slice/dummyinclude " +
          "ice/src/slice2cppe " +
          "ice/src/slice2javae " +
          "icee/include/IceE/Config.h "
         )

#
# Copy Ice-E specific install files.
#
print "Copying icee install files..."
shutil.copyfile(os.path.join("ice", "install", "icee", "ICE_LICENSE"), os.path.join("ice", "ICE_LICENSE"))
shutil.copyfile(os.path.join("ice", "install", "icee", "README"), os.path.join("ice", "README"))
shutil.copyfile(os.path.join("ice", "install", "icee", "INSTALL.LINUX"), os.path.join("ice", "INSTALL.LINUX"))
shutil.copyfile(os.path.join("ice", "install", "icee", "INSTALL.WINDOWS"), os.path.join("ice", "INSTALL.WINDOWS"))

#
# Move Make.rules*icee
#
shutil.move(os.path.join("ice", "config", "Make.rules.icee"), os.path.join("ice", "config", "Make.rules"))
shutil.move(os.path.join("ice", "config", "Make.rules.mak.icee"), os.path.join("ice", "config", "Make.rules.mak"))

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("ice", "CHANGES"), \
    os.path.join("ice", "INSTALL.HP-UX"), \
    os.path.join("ice", "INSTALL.MACOSX"), \
    os.path.join("ice", "INSTALL.SOLARIS"), \
    os.path.join("ice", "iceemakedist.py"), \
    os.path.join("ice", "WINDOWS_SERVICE.txt"), \
    os.path.join("ice", "makedist.py"), \
    os.path.join("ice", "makebindist.py"), \
    os.path.join("ice", "fixCopyright.py"), \
    os.path.join("ice", "fixVersion.py"), \
    os.path.join("ice", "allTests.py"), \
    ]
filesToRemove.extend(find("ice", ".dummy"))
for x in filesToRemove:
    os.remove(x)
shutil.rmtree(os.path.join("ice", "install"))

#
# Generate bison files.
#
print "Generating bison files..."
cwd = os.getcwd()
grammars = find("ice", "*.y")
for x in grammars:
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
    fixMakefile("Makefile", base)
    fixMakefile("Makefile.mak", base)

    os.chdir(cwd)

#
# Generate flex files.
#
print "Generating flex files..."
scanners = find("ice", "*.l")
for x in scanners:
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
    os.system("gmake " + quiet + " " + base + ".cpp")
    #
    # Edit the Makefile to comment out the flex rules.
    #
    fixMakefile("Makefile", base)
    fixMakefile("Makefile.mak", base)

    os.chdir(cwd)

#
# Get Ice-E version.
#
config = open(os.path.join("icee", "include", "IceE", "Config.h"), "r")
version = re.search("ICEE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)


print "Fixing makefiles..."


for makeRulesName in [os.path.join("ice", "config", "Make.rules"), \
                      os.path.join("ice", "config", "Make.rules.mak")]:
    fixMakeRules(makeRulesName)
    makeRules = open(makeRulesName, "r")
    lines = makeRules.readlines()
    makeRules.close()
    for i in range(len(lines)):
        if lines[i].find("prefix") == 0:
            lines[i] = lines[i].replace("IceE-$(VERSION)", "IceE-" + version)
    makeRules = open(makeRulesName, "w")
    makeRules.writelines(lines)
    makeRules.close()


#
# Change SUBDIRS and INSTALL_SUBDIRS in top-level Makefile.
#
for makeFileName in [os.path.join("ice", "Makefile"), \
                     os.path.join("ice", "Makefile.mak")]:
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
for makeFileName in [os.path.join("ice", "src", "IceUtil", "Makefile"), \
                     os.path.join("ice", "src", "IceUtil", "Makefile.mak"), \
                     os.path.join("ice", "src", "Slice", "Makefile"), \
                     os.path.join("ice", "src", "Slice", "Makefile.mak")]:
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

#
# Fix versions in README and INSTALL files.
#
print "Fixing version in README and INSTALL files..."
fixVersion(find("ice", "README*"), version)
fixVersion(find("ice", "INSTALL*"), version)

#
# Create archives.
#
print "Creating distribution..."
icever = "IceE-trans-" + version
os.rename("ice", icever)
if verbose:
    quiet = "v"
else:
    quiet = ""
os.system("chmod -R u+rw,go+r . " + icever)
os.system("find " + icever + " -type d -exec chmod a+x {} \\;")
os.system("tar c" + quiet + "f " + icever + ".tar " + icever)
os.system("gzip -9 " + icever + ".tar")
if verbose:
    quiet = ""
else:
    quiet = "q"
os.system("zip -9r" + quiet + " " + icever + ".zip " + icever)

#
# Copy files (README, etc.).
#
#shutil.copyfile(os.path.join(icever, "CHANGES"), "Ice-" + version + "-CHANGES")

#
# Done.
#
print "Cleaning up..."
shutil.rmtree(icever)
shutil.rmtree("icee")
print "Done."
