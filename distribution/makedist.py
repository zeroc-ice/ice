#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit

sys.path.append(os.path.join(os.path.dirname(__file__), "lib"))
from DistUtils import *
import FixUtil

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
    "makewinrtcerts.py", \
    "winrt", \
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
gitRepoDir = os.path.join(os.getcwd(), os.path.dirname(__file__), "..")

# Restore git attributes and core.autocrlf on exit.
def restore():
    os.chdir(gitRepoDir)
    os.system("git config --unset core.autocrlf")
    os.system("git checkout .gitattributes")

atexit.register(restore)

os.chdir(gitRepoDir)

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
winSrcDir = os.path.join(distDir, "Ice")
rpmBuildDir = os.path.join(distDir, "Ice-rpmbuild-" + version)
distFilesDir = os.path.join(distDir, "distfiles-" + version)
winDistFilesDir = os.path.join(distDir, "distfiles")
os.mkdir(demoscriptDir)
os.mkdir(demoDir)
os.mkdir(winDemoDir)
os.mkdir(rpmBuildDir)

def createDistfiles(platform, whichDestDir):
    print "Creating " + platform + " distfiles git archive using " + tag + "...",
    sys.stdout.flush()
    os.system("git archive --worktree-attributes --prefix=tmp/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
    print "ok"

    move(os.path.join(distDir, "tmp", "distribution"), whichDestDir)
    remove(os.path.join(distDir, "tmp"))
    os.chdir(whichDestDir)

    print "Walking through distribution to fix permissions, versions, etc...",
    sys.stdout.flush()

    fixVersion(os.path.join("bin", "makebindist.py"), *versions)
    if platform == "UNIX":
	fixVersion(os.path.join("src", "rpm", "glacier2router.conf"), *versions)
	fixVersion(os.path.join("src", "rpm", "icegridregistry.conf"), *versions)
	fixVersion(os.path.join("src", "rpm", "RPM_README"), *versions)

    #
    # Fix OS X installer files.
    #
    for root, dirnames, filenames in os.walk('src/mac/Ice'):
        for f in filenames:
            if fnmatch.fnmatch(f, "*.txt") or fnmatch.fnmatch(f, "*.xml") or fnmatch.fnmatch(f, "*.sh"):
                filepath = os.path.join(root, f)
                fixVersion(filepath, *versions)

    for root, dirnames, filenames in os.walk('.'):
	for f in filenames:
	    filepath = os.path.join(root, f) 
	    # Fix version of README/INSTALL files
	    if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
		fixVersion(filepath, *versions)
	    fixFilePermission(filepath, verbose)
	for d in dirnames:
	    os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

    print "ok"

def fixGitAttributes(checkout, autocrlf, addText):
    os.chdir(gitRepoDir)
    if checkout:
	os.system("git checkout .gitattributes")
    if autocrlf:
	os.system("git config core.autocrlf true")
    else:
	os.system("git config --unset core.autocrlf")

    file = ".gitattributes"

    origfile = file + ".orig"
    os.rename(file, origfile)

    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    doComment = 0
    doCheck = 0
    newLines = []
    for x in origLines:
        #
        # If the rule contains the target string, then
        # comment out this rule.
        #
        if autocrlf and x.find("text=auto") != -1:
            x = "#" + x
        newLines.append(x)

    if addText:
	newLines.append("""
# THE FOLLOWING LINES WERE ADDED BY makedist.py
# DO NOT COMMIT
""")
	newLines.append(addText)

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

###### UNIX distfiles 
fixGitAttributes(True, False, """
/certs export-ignore
/config export-ignore
/cpp export-ignore
/cs export-ignore
/demoscript export-ignore
/dists export-ignore
/java export-ignore export-ignore
/php export-ignore
/py export-ignore
/rb export-ignore
/scripts export-ignore
/slice export-ignore
/vb export-ignore
/vsaddin export-ignore
""")

createDistfiles("UNIX", distFilesDir)

###### Windows distfiles 

# No copy this time. Use the same .gitattributes file as the UNIX distfiles dist
fixGitAttributes(False, True, "")
createDistfiles("Windows", winDistFilesDir)

###### UNIX source code distribution
fixGitAttributes(True, False, """
/distribution export-ignore
/vsaddin export-ignore

*.sln export-ignore
*.csproj export-ignore
*.vbproj export-ignore
*.vcproj export-ignore
Make*mak* export-ignore
Make.rules.msvc export-ignore
.depend.mak export-ignore
*.exe.config export-ignore
/cpp/demo/Ice/MFC export-ignore
""")

#
# Extract the sources with git archive using the given tag.
#
print "Creating UNIX git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --worktree-attributes --prefix=Ice-" + version + "/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
print "ok"

os.chdir(srcDir)

print "Walking through distribution to fix permissions, versions, etc...",
sys.stdout.flush()

fixVersion("RELEASE_NOTES", *versions)
fixVersion(os.path.join("cpp", "config", "glacier2router.cfg"), *versions)
fixVersion(os.path.join("cpp", "config", "icegridregistry.cfg"), *versions)

makefileFixList = []
for root, dirnames, filenames in os.walk('.'):

    for f in filenames:
	filepath = os.path.join(root, f) 
	if f == "expect.py":
	    move(filepath, os.path.join(distDir, demoscriptDir, filepath))
	else:
	    # Fix version of README/INSTALL files and keep track of bison/flex files for later processing
	    if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
		fixVersion(filepath, *versions)
	    elif fnmatch.fnmatch(f, "*.y") or fnmatch.fnmatch(f, "*.l"):
		makefileFixList.append(filepath)

	    fixFilePermission(filepath, verbose)
    
    for d in dirnames:
	os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

move("demoscript", os.path.join(demoscriptDir, "demoscript")) # Move the demoscript directory
# and the associated top level demo script.
move("allDemos.py", os.path.join(demoscriptDir, "demoscript", "allDemos.py"))

print "ok"

print "Fixing Makefile ...",
sys.stdout.flush()
for x in makefileFixList:
    (dir,file) = os.path.split(x)
    (base,ext) = os.path.splitext(file)
    fixMakefile(os.path.join(dir,"Makefile"), base, ext)
fixMakeRules(os.path.join("cpp", "config", "Make.rules"))
print "ok"

###### Windows source code distribution
fixGitAttributes(True, True, """
/distribution export-ignore
/demoscript export-ignore
allDemos.py export-ignore
""")

# Don't remove Makefile from the Windows distribution since the
# mingw build requires it.

#
# Extract the sources with git archive using the given tag.
#
print "Creating Windows git archive using " + tag + "...",
sys.stdout.flush()
os.system("git archive --worktree-attributes --prefix=Ice/ " + tag + " | ( cd " + distDir + " && tar xfm - )")
print "ok"

os.chdir(winSrcDir)

print "Walking through distribution to fix permissions, versions, etc...",
sys.stdout.flush()

fixVersion("RELEASE_NOTES", *versions)
fixVersion(os.path.join("cpp", "config", "glacier2router.cfg"), *versions)
fixVersion(os.path.join("cpp", "config", "icegridregistry.cfg"), *versions)
fixVersion(os.path.join("vsaddin", "config", "Ice-VS2008.AddIn"), *versions)
fixVersion(os.path.join("vsaddin", "config", "Ice-VS2010.AddIn"), *versions)
fixVersion(os.path.join("vsaddin", "config", "Ice-VS2012.AddIn"), *versions)
fixVersion(os.path.join("vsaddin", "config", "Ice.props"), *versions)

makefileFixList = []
for root, dirnames, filenames in os.walk('.'):

    for f in filenames:
        filepath = os.path.join(root, f) 
	if f == "expect.py":
	    remove(filepath)
	else:
	    # Fix version of README/INSTALL files and keep track of bison/flex files for later processing
	    if fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "INSTALL*"):
		fixVersion(filepath, *versions)
	    elif fnmatch.fnmatch(f, "*.y") or fnmatch.fnmatch(f, "*.l"):
		makefileFixList.append(filepath)

	    fixFilePermission(filepath, verbose)
    
    for d in dirnames:
        os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

print "ok"

print "Fixing Makefile.mak ...",
sys.stdout.flush()
for x in makefileFixList:
    (dir,file) = os.path.split(x)
    (base,ext) = os.path.splitext(file)
    fixMakefile(os.path.join(dir,"Makefile.mak"), base, ext)
fixMakeRules(os.path.join("cpp", "config", "Make.rules"))
print "ok"

for root, dirnames, filesnames in os.walk("."):
    for f in filesnames:

        #
        # Change text based file extension to .txt
        #
        for name in ["README", "CHANGES", "LICENSE", "ICE_LICENSE", "RELEASE_NOTES", "INSTALL*"]:
            if fnmatch.fnmatch(f, name) and not fnmatch.fnmatch(f, name + ".txt") :
                oldname = os.path.join(root, f)
                newname = oldname + ".txt"
                os.rename(oldname, newname)

os.chdir(srcDir)

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

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        copy(os.path.join(d, "demo"), os.path.join(demoDir, getMappingDir("demo", d)))

configSubstituteExprs = [(re.compile(regexpEscape("../../certs")), "../certs")]
for root, dirnames, filesnames in os.walk(demoDir):
    for f in filesnames:
        if fnmatch.fnmatch(f, "config*"):
            substitute(os.path.join(root, f), configSubstituteExprs)

#
# vb directory in Unix source distribution only needed to copy demo scripts.
#
remove(os.path.join(srcDir, 'vb'))

# Windows demo distribution
copy(os.path.join(winDistFilesDir, "src", "common", "README.DEMOS.txt"), os.path.join(winDemoDir, "README.txt"))

copyMatchingFiles(os.path.join(winSrcDir, "certs"), os.path.join(winDemoDir, "certs"), certsFiles)

os.mkdir(os.path.join(winDemoDir, "config"))

copy(os.path.join(winSrcDir, "config", "Make.common.rules.mak"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winSrcDir, "cpp", "config", "Make.rules.msvc"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winSrcDir, "java", "config", "common.xml"), os.path.join(winDemoDir, "config"), False)

copy(os.path.join(winDistFilesDir, "src", "common", "build.properties"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winDistFilesDir, "src", "common", "Make.rules.mak"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winDistFilesDir, "src", "common", "Make.rules.mak.php"), os.path.join(winDemoDir, "config"), False)

# Consolidate demo distribution with files from each language mapping
for sd in os.listdir(winSrcDir):
    d = os.path.join(winSrcDir, sd)
    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        copy(os.path.join(d, "demo"), os.path.join(winDemoDir, getMappingDir("demo", sd)))

rmFiles = []

for root, dirnames, filesnames in os.walk(winDemoDir):
    for f in filesnames:

        if fnmatch.fnmatch(f, "README") and not fnmatch.fnmatch(f, "README.txt"):
            oldreadme = os.path.join(root, f)
            newreadme = oldreadme + ".txt"
            os.rename(oldreadme, newreadme)

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

            for m in [ "*.csproj", "*.vbproj" ]:
                if fnmatch.fnmatch(f, m):
                    FixUtil.fileMatchAndReplace(os.path.join(root, f),
		    				[(re.escape("PublicKeyToken=1f998c50fec78381"),
						  "PublicKeyToken=cdd571ade22f2f16")], False)

for d in ["demo", "democs", "demovb"]:
    for root, dirnames, filesnames in os.walk(os.path.join(winDemoDir, d)):
        for f in filesnames:
            for m in [ "*.vcproj", "*.vcxproj", "*.vcxproj.filters", "*.csproj", "*.vbproj" ]:
                if fnmatch.fnmatch(f, m):
                    FixUtil.fileMatchAndReplace(os.path.join(root, f), [(re.escape('"README"'), '"README.txt"')], False)
                    FixUtil.fileMatchAndReplace(os.path.join(root, f), 
                                                [(re.escape("..\\..\\..\\..\\..\\certs\\cacert.pem"), 
                                                  "..\\..\\..\\..\\certs\\cacert.pem")], False)

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

for (dir, archiveDir) in [(winDistFilesDir, "distfiles-" + version)]:
    zipArchive(dir, verbose, archiveDir)
os.rename(os.path.join(distDir, "distfiles.zip"), os.path.join(distDir, "distfiles-" + version + ".zip"))

for (dir, archiveDir) in [(winSrcDir, "Ice-" + version)]:
    zipArchive(dir, verbose, archiveDir)
os.rename(os.path.join(distDir, "Ice.zip"), os.path.join(distDir, "Ice-" + version + ".zip"))

for (dir, archiveDir) in [(winDemoDir, "Ice-" + version + "-demos")]:
    zipArchive(dir, verbose, archiveDir)
os.rename(os.path.join(distDir, "demos.zip"), os.path.join(distDir, "Ice-" + version + "-demos.zip"))

#
# Write source distribution report in README file.
#
writeSrcDistReport("Ice", version, compareToDir, [srcDir, demoDir, winDemoDir, distFilesDir, winDistFilesDir, rpmBuildDir, demoscriptDir])

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
remove(srcDir)
remove(winSrcDir)
remove(demoDir)
remove(winDemoDir)
remove(demoscriptDir)
remove(rpmBuildDir)
remove(distFilesDir)
remove(winDistFilesDir)
print "ok"
