#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt

sys.path.append(os.path.join(os.path.dirname(__file__), "lib"))
from DistUtils import *
import FixUtil

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
    "vsaddin", \
    "config", \
    "scripts", \
    "certs", \
    "slice", \
    "distribution", \
    "demoscript", \
]

#
# Files to not include in the source distributions.
#
filesToRemove = [ \
    "distribution/bin/fixCopyright.py", \
    "distribution/bin/fixVersion.py", \
    "distribution/lib/FixUtil.py", \
    "distribution/makedist.py", \
    "cpp/config/makegitignore.py", \
    "rb/config/Make.rules.Darwin", \
    "cpp/src/slice2confluence", \
]

#
# Files from the top-level, cpp, java and cs config directories to include in the demo 
# source distribution config directory.
#
configFiles = [ \
    "Make.*", \
    "common.xml", \
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
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h      Show this message."
    print "-v      Be verbose."
    print "-c DIR  Compare distribution to the one from DIR and"
    print "        save the result in the README file"

#
# Check arguments
#
try:
    opts, args = getopt.getopt(sys.argv[1:], "hvc:")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)

tag = "HEAD"
if len(args) > 1:
    usage()
elif len(args) == 1:
    tag = args[0]

verbose = 0
compareToDir = None
for (o, a) in opts:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-v":
        verbose = 1
    elif o == "-c":
        compareToDir = a

cwd = os.getcwd()
os.chdir(os.path.join(os.path.dirname(__file__), ".."))

#
# Get Ice versions.
#
config = open(os.path.join("config", "Make.common.rules"), "r")
version = re.search("VERSION\s*=\s*([0-9\.b]*)", config.read()).group(1)
mmversion = re.search("([0-9]+\.[0-9b]+)[\.0-9]*", version).group(1)
libversion = mmversion.replace('.', '')
versions = (version, mmversion, libversion)
config.close()

#
# Remove any existing "dist-" directory and create a new one
# and sub-directories for the each source distribution.
#
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "dist-" + tag.replace('/', '-')))
if os.path.exists(distDir):
    remove(distDir)
os.mkdir(distDir)

print "Creating " + version + " source distributions in " + distDir

demoscriptDir = os.path.join(distDir, "Ice-" + version + "-demo-scripts")
demoDir = os.path.join(distDir, "Ice-" + version + "-demos")
winDemoDir = os.path.join(distDir, "demos")
srcDir = os.path.join(distDir, "Ice-" + version)
rpmBuildDir = os.path.join(distDir, "Ice-rpmbuild-" + version)
distFilesDir = os.path.join(distDir, "distfiles-" + version)
os.mkdir(demoscriptDir)
os.mkdir(demoDir)
os.mkdir(winDemoDir)
os.mkdir(rpmBuildDir)

#
# Extract the sources with git archive using the given tag.
#
print "Creating git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --prefix=Ice-" + version + "/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
print "ok"

os.chdir(os.path.join(srcDir))

#
# Remove or move non-public files out of source distribution.
#
print "Removing non-Ice directories and files...",
sys.stdout.flush()
for x in filesToRemove:
    remove(x)
for d in os.listdir('.'):
    if os.path.isdir(d) and not d in includeSubDirs:
        remove(d)
print "ok"

print "Walking through distribution to fix permissions, versions, etc...",
sys.stdout.flush()

fixVersion("RELEASE_NOTES", *versions)
fixVersion(os.path.join("distribution", "bin", "makebindist.py"), *versions)
fixVersion(os.path.join("cpp", "config", "glacier2router.cfg"), *versions)
fixVersion(os.path.join("cpp", "config", "icegridregistry.cfg"), *versions)
fixVersion(os.path.join("distribution", "src", "rpm", "glacier2router.conf"), *versions)
fixVersion(os.path.join("distribution", "src", "rpm", "icegridregistry.conf"), *versions)
fixVersion(os.path.join("distribution", "src", "rpm", "RPM_README"), *versions)
fixVersion(os.path.join("vsaddin", "config", "Ice-VS2008.AddIn"), *versions)
fixVersion(os.path.join("vsaddin", "config", "Ice-VS2010.AddIn"), *versions)
fixVersion(os.path.join("vsaddin", "config", "ice.vsprops"), *versions)
fixVersion(os.path.join("vsaddin", "config", "ice.props"), *versions)

bisonFiles = []
flexFiles = []
for root, dirnames, filenames in os.walk('.'):

    for f in filenames:
        filepath = os.path.join(root, f) 
        if f == ".gitignore":
            remove(filepath)
        elif f == "expect.py":
            move(filepath, os.path.join(distDir, demoscriptDir, filepath))
        else:
            # Fix version of README/INSTALL files and keep track of bison/flex files for later processing
            if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
                fixVersion(filepath, *versions)
            elif fnmatch.fnmatch(f, "*.y"):
                bisonFiles.append(filepath)
            elif fnmatch.fnmatch(f, "*.l"):
                flexFiles.append(filepath)

            fixFilePermission(filepath, verbose)
    
    for d in dirnames:
        os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

move("distribution", distFilesDir) # Move the distribution directory to the top-level
move("demoscript", os.path.join(demoscriptDir, "demoscript")) # Move the demoscript directory
# and the associated top level demo script.
move("allDemos.py", os.path.join(demoscriptDir, "demoscript", "allDemos.py"))

print "ok"

#
# Generate bison & flex files.
#
print "Generating bison and flex files...",
sys.stdout.flush()
for x in bisonFiles:
    generateBisonFile(x, verbose)
for x in flexFiles:
    generateFlexFile(x, verbose)
fixMakeRules(os.path.join("cpp", "config", "Make.rules"))
print "ok"

#
# Consolidate demo, demo scripts distributions.
#
print "Consolidating demo and demo scripts distributions...",
sys.stdout.flush()

# Unix demo distribution
copy("ICE_LICENSE", demoDir)
copy(os.path.join(distFilesDir, "src", "common", "README.DEMOS"), demoDir)

copyMatchingFiles(os.path.join("certs"), os.path.join(demoDir, "certs"), certsFiles)
for d in ["", "cpp", "java", "cs"]:
    copyMatchingFiles(os.path.join(d, "config"), os.path.join(demoDir, "config"), configFiles)

copy(os.path.join(distFilesDir, "src", "common", "Make.rules"), os.path.join(demoDir, "config"), False)
copy(os.path.join(distFilesDir, "src", "common", "Make.rules.cs"), os.path.join(demoDir, "config"), False)
copy(os.path.join(distFilesDir, "src", "common", "Make.rules.php"), os.path.join(demoDir, "config"), False)
copy(os.path.join(distFilesDir, "src", "common", "build.properties"), os.path.join(demoDir, "config"), False)
copy(os.path.join(srcDir, "scripts"), os.path.join(demoscriptDir, "scripts"))

# Consolidate demoscript and demo distribution with files from each language mapping
for d in os.listdir('.'):

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "allDemos.py")):
        md = os.path.join(demoscriptDir, getMappingDir("demo", d))
        move(os.path.join(demoscriptDir, d, "demo"), md)
        move(os.path.join(d, "allDemos.py"), os.path.join(md, "allDemos.py"))
        os.rmdir(os.path.join(demoscriptDir, d))

    if d == "vb":
        continue

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        copy(os.path.join(d, "demo"), os.path.join(demoDir, getMappingDir("demo", d)))

rmFiles = []
configSubstituteExprs = [(re.compile(regexpEscape("../../certs")), "../certs")]
for root, dirnames, filesnames in os.walk(demoDir):
    for f in filesnames:

        if fnmatch.fnmatch(f, "config*"):
            substitute(os.path.join(root, f), configSubstituteExprs)

        for m in [ "*.sln", "*.csproj", "*.vbproj", "*.vcproj", "Make*mak*", "Make.rules.msvc", "Make.rules.bcc",
                   ".depend.mak", "*.exe.config"]:
            if fnmatch.fnmatch(f, m):
                rmFiles.append(os.path.join(root[len(demoDir) + 1:], f))

for f in rmFiles: remove(os.path.join(demoDir, f))

# Windows demo distribution
copy(os.path.join(distFilesDir, "src", "common", "README.DEMOS.txt"), os.path.join(winDemoDir, "README.txt"))

copyMatchingFiles(os.path.join("certs"), os.path.join(winDemoDir, "certs"), certsFiles)

os.mkdir(os.path.join(winDemoDir, "config"))

copy(os.path.join(srcDir, "config", "Make.common.rules.mak"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(srcDir, "cpp", "config", "Make.rules.bcc"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(srcDir, "cpp", "config", "Make.rules.msvc"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(srcDir, "java", "config", "common.xml"), os.path.join(winDemoDir, "config"), False)

copy(os.path.join(distFilesDir, "src", "common", "build.properties"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(distFilesDir, "src", "common", "Make.rules.mak"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(distFilesDir, "src", "common", "Make.rules.mak.php"), os.path.join(winDemoDir, "config"), False)

# Consolidate demo distribution with files from each language mapping
for d in os.listdir('.'):

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        copy(os.path.join(d, "demo"), os.path.join(winDemoDir, getMappingDir("demo", d)))

rmFiles = []

for root, dirnames, filesnames in os.walk(winDemoDir):
    for f in filesnames:

        if fnmatch.fnmatch(f, "README"):
	    oldreadme = os.path.join(root, f)
	    newreadme = oldreadme + ".txt"
	    os.rename(oldreadme, newreadme)
	    os.system('unix2dos -q ' + newreadme)

        if fnmatch.fnmatch(f, "config*"):
            substitute(os.path.join(root, f), configSubstituteExprs)

        for m in [ "Makefile", ".depend", "*.exe.config" ]:
            if fnmatch.fnmatch(f, m):
                rmFiles.append(os.path.join(root[len(winDemoDir) + 1:], f))

for d in ["democs", "demovb"]:
    for root, dirnames, filesnames in os.walk(os.path.join(winDemoDir, d)):
        for f in filesnames:
            for m in [ "Makefile.mak", ".depend.mak" ]:
                if fnmatch.fnmatch(f, m):
                    rmFiles.append(os.path.join(root[len(winDemoDir) + 1:], f))

for d in ["demo", "democs", "demovb"]:
    for root, dirnames, filesnames in os.walk(os.path.join(winDemoDir, d)):
        for f in filesnames:
            for m in [ "*.vcproj", "*.vcxproj", "*.vcxproj.filters", "*.csproj", "*.vbproj" ]:
                if fnmatch.fnmatch(f, m):
		    FixUtil.fileMatchAndReplace(os.path.join(root, f), [("(README)", "README.txt")], False)

for f in rmFiles: remove(os.path.join(winDemoDir, f))

print "ok"

#
# Create the Ice-rpmbuild archive
#
rpmBuildFiles = [ \
    os.path.join("src", "rpm", "*.conf"), \
    os.path.join("src", "rpm", "*.suse"), \
    os.path.join("src", "rpm", "*.redhat"), \
    os.path.join("src", "rpm", "ice.pth"), \
    os.path.join("src", "unix", "*Linux*"), \
    os.path.join("src", "thirdparty", "php", "ice.ini"), \
]
copyMatchingFiles(os.path.join(distFilesDir), rpmBuildDir, rpmBuildFiles)

#
# Copy CHANGES and RELEASE_NOTES
#
copy(os.path.join(srcDir, "CHANGES"), os.path.join(distDir, "Ice-" + version + "-CHANGES"))
copy(os.path.join(srcDir, "RELEASE_NOTES"), os.path.join(distDir, "Ice-" + version + "-RELEASE_NOTES"))
copy(os.path.join(distFilesDir, "src", "rpm", "RPM_README"), \
        os.path.join(distDir, "Ice-" + version + "-RPM_README"))

#
# Everything should be clean now, we can create the source distributions archives
# 
print "Archiving..."
sys.stdout.flush()
os.chdir(distDir)
for d in [srcDir, demoDir, distFilesDir, rpmBuildDir]:
    tarArchive(d, verbose)

for (dir, archiveDir) in [(demoscriptDir, "Ice-" + version + "-demos")]:
    tarArchive(dir, verbose, archiveDir)

zipArchive(srcDir, verbose)

for (dir, archiveDir) in [(winDemoDir, "Ice-" + version + "-demos")]:
    zipArchive(dir, verbose, archiveDir)

os.rename(os.path.join(distDir, "demos.zip"), os.path.join(distDir, "Ice-" + version + "-demos.zip"))

#
# Write source distribution report in README file.
#
writeSrcDistReport("Ice", version, compareToDir, [srcDir, demoDir, winDemoDir, distFilesDir, rpmBuildDir, demoscriptDir])

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
remove(srcDir)
remove(demoDir)
remove(winDemoDir)
remove(demoscriptDir)
remove(rpmBuildDir)
remove(distFilesDir)
print "ok"

os.chdir(cwd)

