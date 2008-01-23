#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
tag = "HEAD"
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
        tag = x

#
# Remove any existing "dist" directory and create a new one.
#
distdir = "dist"
if os.path.exists(distdir):
    shutil.rmtree(distdir)
os.mkdir(distdir)
os.mkdir(os.path.join(distdir, "icesl"))
tmpdir = "tmp"
os.mkdir(os.path.join(distdir, tmpdir))

#
# Export sources from git.
#
print "Checking out sources " + tag + "..."
if verbose:
    quiet = "-v"
else:
    quiet = ""
os.system("git archive " + quiet + " " + tag + " . | (cd dist/icesl && tar xf -)")

os.chdir(os.path.join("..", "sl", "src"))
os.system("git archive " + quiet + " " + tag + " . | (cd ../../cpp/dist/tmp && tar xf -)")

os.chdir(os.path.join("..", "..", "distribution", "src", "icesl"))
os.system("git archive " + quiet + " " + tag + " . | (cd ../../../cpp/dist/tmp && tar xf -)")

os.chdir(os.path.join("..", "..", "..", "cpp", distdir))

#
# Copy IceSL specific install files.
#
print "Copying icesl install files..."
shutil.copyfile(os.path.join("tmp", "ICE_LICENSE"), os.path.join("icesl", "ICE_LICENSE"))
shutil.copyfile(os.path.join("tmp", "README.txt"), os.path.join("icesl", "README.txt"))
shutil.copyfile(os.path.join("tmp", "INSTALL.txt"), os.path.join("icesl", "INSTALL.txt"))

#
# Move Make.rules.mak.icesl
#
shutil.move(os.path.join("icesl", "config", "Make.rules.mak.icesl"), os.path.join("icesl", "config", "Make.rules.mak"))

#
# Generate bison files.
#
print "Generating bison files..."
cwd = os.getcwd()
grammars = find("icesl", "*.y")
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
    fixMakefile("Makefile.mak", base)

    os.chdir(cwd)

#
# Generate flex files.
#
print "Generating flex files..."
scanners = find("icesl", "*.l")
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
    fixMakefile("Makefile.mak", base)

    os.chdir(cwd)

#
# Remove files.
#
print "Removing unnecessary files..."
filesToRemove = [ \
    os.path.join("icesl", "CHANGES"), \
    os.path.join("icesl", "README"), \
    os.path.join("icesl", "INSTALL.HP-UX"), \
    os.path.join("icesl", "INSTALL.MACOSX"), \
    os.path.join("icesl", "INSTALL.SOLARIS"), \
    os.path.join("icesl", "INSTALL.WINDOWS"), \
    os.path.join("icesl", "INSTALL.LINUX"), \
    os.path.join("icesl", "iceemakedist.py"), \
    os.path.join("icesl", "iceslmakedist.py"), \
    os.path.join("icesl", "WINDOWS_SERVICE.txt"), \
    os.path.join("icesl", "makedist.py"), \
    os.path.join("icesl", "fixCopyright.py"), \
    os.path.join("icesl", "fixVersion.py"), \
    os.path.join("icesl", "allTests.py"), \
    os.path.join("icesl", "allDemos.py"), \
    os.path.join("icesl", "config", "convertssl.py"), \
    os.path.join("icesl", "config", "findSliceFiles.py"), \
    os.path.join("icesl", "config", "glacier2router.cfg"), \
    os.path.join("icesl", "config", "ice_ca.cnf"), \
    os.path.join("icesl", "config", "icegridnode.cfg"), \
    os.path.join("icesl", "config", "icegridregistry.cfg"), \
    os.path.join("icesl", "config", "icegrid-slice.3.1.ice.gz"), \
    os.path.join("icesl", "config", "makedepend.py"), \
    os.path.join("icesl", "config", "makegitignore.py"), \
    os.path.join("icesl", "config", "Make.rules"), \
    os.path.join("icesl", "config", "Make.rules.AIX"), \
    os.path.join("icesl", "config", "Make.rules.bcc"), \
    os.path.join("icesl", "config", "Make.rules.Darwin"), \
    os.path.join("icesl", "config", "Make.rules.FreeBSD"), \
    os.path.join("icesl", "config", "Make.rules.HP-UX"), \
    os.path.join("icesl", "config", "Make.rules.Linux"), \
    os.path.join("icesl", "config", "Make.rules.OSF1"), \
    os.path.join("icesl", "config", "Make.rules.SunOS"), \
    os.path.join("icesl", "config", "Make.rules.icee"), \
    os.path.join("icesl", "config", "Make.rules.mak.icee"), \
    os.path.join("icesl", "config", "templates.xml"), \
    os.path.join("icesl", "config", "upgradeicegrid.py"), \
    os.path.join("icesl", "config", "upgradeicestorm.py"), \
    ]
filesToRemove.extend(find("icesl", ".gitignore"))
filesToRemove.extend(find("icesl", "Makefile"))
for x in filesToRemove:
    os.remove(x)
shutil.rmtree(os.path.join("icesl", "demo"))
shutil.rmtree(os.path.join("icesl", "doc"))
shutil.rmtree(os.path.join("icesl", "test"))
shutil.rmtree(os.path.join("icesl", "include", "Freeze"))
shutil.rmtree(os.path.join("icesl", "include", "Glacier2"))
shutil.rmtree(os.path.join("icesl", "include", "Ice"))
shutil.rmtree(os.path.join("icesl", "include", "IceBox"))
shutil.rmtree(os.path.join("icesl", "include", "IceGrid"))
shutil.rmtree(os.path.join("icesl", "include", "IcePatch2"))
shutil.rmtree(os.path.join("icesl", "include", "IceSSL"))
shutil.rmtree(os.path.join("icesl", "include", "IceStorm"))
shutil.rmtree(os.path.join("icesl", "include", "IceXML"))
shutil.rmtree(os.path.join("icesl", "src", "ca"))
shutil.rmtree(os.path.join("icesl", "src", "Freeze"))
shutil.rmtree(os.path.join("icesl", "src", "FreezeScript"))
shutil.rmtree(os.path.join("icesl", "src", "Glacier2"))
shutil.rmtree(os.path.join("icesl", "src", "Ice"))
shutil.rmtree(os.path.join("icesl", "src", "IceBox"))
shutil.rmtree(os.path.join("icesl", "src", "IceGrid"))
shutil.rmtree(os.path.join("icesl", "src", "IcePatch2"))
shutil.rmtree(os.path.join("icesl", "src", "iceserviceinstall"))
shutil.rmtree(os.path.join("icesl", "src", "IceSSL"))
shutil.rmtree(os.path.join("icesl", "src", "IceStorm"))
shutil.rmtree(os.path.join("icesl", "src", "IceXML"))
shutil.rmtree(os.path.join("icesl", "src", "slice2cpp"))
shutil.rmtree(os.path.join("icesl", "src", "slice2cppe"))
shutil.rmtree(os.path.join("icesl", "src", "slice2cs"))
shutil.rmtree(os.path.join("icesl", "src", "slice2docbook"))
shutil.rmtree(os.path.join("icesl", "src", "slice2freeze"))
shutil.rmtree(os.path.join("icesl", "src", "slice2freezej"))
shutil.rmtree(os.path.join("icesl", "src", "slice2html"))
shutil.rmtree(os.path.join("icesl", "src", "slice2java"))
shutil.rmtree(os.path.join("icesl", "src", "slice2javae"))
shutil.rmtree(os.path.join("icesl", "src", "slice2py"))
shutil.rmtree(os.path.join("icesl", "src", "slice2rb"))

#
# Get IceSL version.
#
config = open(os.path.join("tmp", "Ice", "AssemblyInfo.cs"), "r")
version = re.search("AssemblyVersion.*\"([0-9\.]*)\".*", config.read()).group(1)

print "Fixing makefiles..."

for makeRulesName in [os.path.join("icesl", "config", "Make.rules.mak")]:
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
for makeFileName in [os.path.join("icesl", "Makefile.mak")]:
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
for makeFileName in [os.path.join("icesl", "src", "IceUtil", "Makefile.mak"), \
                     os.path.join("icesl", "src", "Slice", "Makefile.mak")]:
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
fixVersion(find("icesl", "README*"), version)
fixVersion(find("icesl", "INSTALL*"), version)

#
# Create archives.
#
print "Creating distribution..."
icever = "IceSL-trans-" + version
os.rename("icesl", icever)
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
shutil.rmtree(tmpdir)
print "Done."
