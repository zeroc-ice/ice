#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
# Files that will be excluded from all source distributions.
#
excludeFiles = [ \
    "/certs/cakey.pem",
    "/distribution",
    "/protobuf",
    "*.iml",
    "/vsaddin/*.sln",
    "/vsaddin/src/*.csproj"
]

#
# Windows files that will be excluded from Unix source distributions.
#
excludeWindowsFiles = [ \
    "/vsaddin/",
    "*.rc",
    "*.sln",
    "*.csproj",
    "*.vbproj",
    "*.vcproj",
    "*.vcxproj",
    "*.vcxproj.filters",
    "Make*mak*",
    "Make.rules.msvc",
    ".depend.mak",
    "*.exe.config",
    "MSG00001.bin",
    "/cpp/test/WinRT",
    "/cpp/demo/**/generated",
    "/cpp/demo/**/MFC",
    "/**/winrt",
    "/cs/**/compact",
    "/cs/**/cf",
    "/cs/**/sl"
]

#
# Unix files that will be excluded from Windows source distributions.
#
# Don't remove C++ source Makefile from the Windows distribution since
# the mingw build requires it.
#
excludeUnixFiles = [ \
    "/Makefile",
]
for l in ["/java", "/py", "/php", "/cs", "/cpp/demo", "/js"]:
    excludeUnixFiles += [
        l + "/**/Makefile",
        l + "/**/Make.rules",
        l + "/**/Make.rules.cs",
        l + "/**/Make.rules.php",
        l + "/**/Make.rules",
        l + "/**/Make.rules.Darwin",
        l + "/**/Make.rules.Linux",
        l + "/**/.depend"
    ]

#
# Files from the top-level, cpp, java, cs and js config directories to include in the demo
# source distribution config directory.
#
demoConfigFiles = [ \
    "Make.*", \
    "build.js", \
    "makebundle.js", \
]

#
# Files from the top-level certs directory to include in the demo distribution certs
# directory.
#
demoCertsFiles = [ \
    "client.bks", \
    "*.jks", \
    "*.pem", \
    "*.pfx", \
    "*.der", \
    "ImportKey.class", \
    "makecerts.py", \
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
    opts, args = getopt.getopt(sys.argv[1:], "hvt:c:")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)

tag = "HEAD"
if len(args) > 1:
    usage()
    sys.exit(1)
elif len(args) == 1:
    tag = args[0]

checkGitVersion() # Ensure we're using the right git version

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
if compareToDir and not os.path.isabs(compareToDir):
    compareToDir = os.path.join(cwd, compareToDir)

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

majorVersion = FixUtil.majorVersion(version)
minorVersion = FixUtil.minorVersion(version)
patchVersion = FixUtil.patchVersion(version) if FixUtil.patchVersion(version) != "51" else "0"

debmmversion = majorVersion + "." + minorVersion
debversion = majorVersion + "." + minorVersion + "." + patchVersion

versions = (version, mmversion, libversion, debversion, debmmversion, majorVersion, minorVersion)
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
winDemoscriptDir = os.path.join(distDir, "demo-scripts")
demoDir = os.path.join(distDir, "Ice-" + version + "-demos")
winDemoDir = os.path.join(distDir, "demos")
srcDir = os.path.join(distDir, "Ice-" + version)
winSrcDir = os.path.join(distDir, "Ice")
rpmBuildDir = os.path.join(distDir, "Ice-rpmbuild-" + version)
debSrcDir = os.path.join(distDir, "zeroc-ice%s-%s" % (debmmversion, debversion))
distFilesDir = os.path.join(distDir, "distfiles-" + version)
winDistFilesDir = os.path.join(distDir, "distfiles")
os.mkdir(demoscriptDir)
os.mkdir(demoDir)
os.mkdir(winDemoDir)
os.mkdir(rpmBuildDir)

def createDistfilesDist(platform, whichDestDir):
    print "Creating " + platform + " distfiles git archive using " + tag + "...",
    sys.stdout.flush()
    os.mkdir(whichDestDir)
    os.system("git archive --worktree-attributes " + tag + ":distribution/ | ( cd " + whichDestDir + " && tar xfm - )")
    print "ok"

    os.chdir(whichDestDir)

    print "Walking through distribution to fix permissions, versions, etc...",
    sys.stdout.flush()

    fixVersion(os.path.join("bin", "makebindist.py"), *versions)
    fixVersion(os.path.join("lib", "DistUtils.py"), *versions)
    fixVersion(os.path.join("bin", "makeubuntupackages.py"), *versions)
    fixVersion(os.path.join("bin", "makeubunturepo.py"), *versions)
    if platform == "UNIX":
        fixVersion(os.path.join("src", "rpm", "icegridregistry.conf"), *versions)
        fixVersion(os.path.join("src", "rpm", "RPM_README"), *versions)

        for root, dirnames, filenames in os.walk('src/deb'):
            for f in filenames:
                fixVersion(os.path.join(root, f), *versions)

    with open(os.path.join(srcDir, "ICE_LICENSE"), "r") as license:
        FixUtil.fileMatchAndReplace(os.path.join("src", "deb", "debian", "copyright"),
                                [(re.compile("@ice-license@"), license.read().replace("\n", "\n "))],
                                False)
    #
    # Fix OS X installer files.
    #
    for root, dirnames, filenames in os.walk('src/mac/'):
        for f in filenames:
            if (fnmatch.fnmatch(f, "*.txt") or
                fnmatch.fnmatch(f, "*.xml") or
                fnmatch.fnmatch(f, "*.sh") or
                fnmatch.fnmatch(f, "*.py")):
                filepath = os.path.join(root, f)
                fixVersion(filepath, *versions)

    for root, dirnames, filenames in os.walk('.'):
        for f in filenames:
            filepath = os.path.join(root, f)
            # Fix version of README files
            if fnmatch.fnmatch(f, "README*"):
                fixVersion(filepath, *versions)
            fixFilePermission(filepath, verbose)

        for d in dirnames:
            os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x

    print "ok"

#
# Install node http-proxy and esprima.
#
os.chdir(distDir)
for m in ["http-proxy", "esprima"]:
    if os.system("npm install --prefix %s %s" % (distDir, m)) != 0:
        print("Error executing command `npm install %s'" % m)

def createSourceDist(platform, destDir):
    if platform == "UNIX":
        prefix = "Ice-" + version
    else:
        prefix = "Ice"

    print "Creating " + platform + " git archive using " + tag + "...",
    sys.stdout.flush()
    os.system("git archive --worktree-attributes --prefix=" + prefix + "/ " + tag +
              " | ( cd " + destDir + " && tar xfm - )")
    print "ok"

    print "Walking through distribution to fix permissions, versions, etc...",
    sys.stdout.flush()

    current = os.getcwd()
    os.chdir(os.path.join(destDir, prefix))

    if os.path.exists("vsaddin"):
        fixVersion(os.path.join("vsaddin", "config", "Ice-VS2012.AddIn"), *versions)
        fixVersion(os.path.join("vsaddin", "config", "Ice-VS2013.AddIn"), *versions)
        fixVersion(os.path.join("vsaddin", "config", "Ice.props"), *versions)

    if os.path.exists(os.path.join("cpp", "config", "Make.rules")):
        fixMakeRules(os.path.join("cpp", "config", "Make.rules"))

    for root, dirnames, filenames in os.walk('.'):
        for f in filenames:
            filepath = os.path.join(root, f)
            if f == "expect.py":
                move(filepath, os.path.join(distDir, demoscriptDir, filepath))
                continue
            elif fnmatch.fnmatch(f, "README*") or fnmatch.fnmatch(f, "*.Addin"):
                fixVersion(filepath, *versions)
            elif fnmatch.fnmatch(f, "*.y") or fnmatch.fnmatch(f, "*.l"):
                fixMakefileForFile(filepath)
            elif f == "Grammar.cpp":
                checkBisonVersion(filepath)
            elif f == "Scanner.cpp":
                checkFlexVersion(filepath)

            if platform == "Windows":
                for name in ["README", "CHANGES", "LICENSE", "ICE_LICENSE"]:
                    if fnmatch.fnmatch(f, name) and not fnmatch.fnmatch(f, name + ".txt"):
                        os.rename(filepath, filepath + ".txt")
                        filepath = filepath + ".txt"
                        f = f + ".txt"

            fixFilePermission(filepath, verbose)

        for d in dirnames:
            os.chmod(os.path.join(root, d), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
    copy(os.path.join(distDir, "node_modules"), os.path.join(destDir, prefix, "js", "node_modules"));

    os.chdir(current)
    print "ok"

def checkBisonVersion(filename):
    f = open(filename, "r")
    for line in f.readlines():
        if re.search("#define YYBISON_VERSION", line) and not re.search("2.4.1", line):
            print "Bison version mistmatch in `" + filename + "'"
            print "required Bison 2.4.1"
            print "Found " + line
            sys.exit(1)

def checkFlexVersion(filename):
    f = open(filename, "r")
    for line in f.readlines():
	if re.search("#define YY_FLEX_MAJOR_VERSION", line) and not re.search("2", line):
            print "Flex version mistmatch in `" + filename + "'"
            print "required Flex 2.5"
            print "Found " + line
            sys.exit(1)

        if re.search("#define YY_FLEX_MINOR_VERSION", line) and not re.search("5", line):
            print "Flex version mistmatch in `" + filename + "'"
            print "required Flex 2.5"
            print "Found " + line
            sys.exit(1)

def fixGitAttributes(checkout, autocrlf, excludes):
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

    if len(excludes) > 0:
        newLines.append("""
# THE FOLLOWING LINES WERE ADDED BY makedist.py
# DO NOT COMMIT
""")

        for e in excludes:
            newLines.append(e + " export-ignore\n")

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

###### UNIX source code distribution
fixGitAttributes(True, False, excludeFiles + excludeWindowsFiles)
createSourceDist("UNIX", distDir)

###### UNIX distfiles
excludeForDistFiles = [ "fixCopyright.py", "fixVersion.py", "makedist.py" ]
fixGitAttributes(True, False, excludeForDistFiles)
createDistfilesDist("UNIX", distFilesDir)

###### Windows distfiles
fixGitAttributes(False, True, []) # No copy this time. Use the same .gitattributes file as the UNIX distfiles dist
createDistfilesDist("Windows", winDistFilesDir)

# Move the demoscript directory and the associated top level demo script.
os.chdir(srcDir)
move("demoscript", os.path.join(demoscriptDir, "demoscript"))
move("allDemos.py", os.path.join(demoscriptDir, "demoscript", "allDemos.py"))

###### Windows source code distribution
fixGitAttributes(True, True, excludeFiles + excludeUnixFiles + ["/demoscript", "expect.py", "allDemos.py"])
createSourceDist("Windows", distDir)

remove(os.path.join(distDir, "node_modules"))

#
# Consolidate demo, demo scripts distributions.
#
print "Consolidating demo and demo scripts distributions...",
os.chdir(srcDir)
sys.stdout.flush()

# Unix demo distribution
copy("ICE_LICENSE", demoDir)
copy(os.path.join(distFilesDir, "src", "common", "README.DEMOS"), demoDir)

copyMatchingFiles(os.path.join(srcDir, "certs"), os.path.join(demoDir, "certs"), demoCertsFiles)
for d in ["", "cpp", "java", "js"]:
    copyMatchingFiles(os.path.join(d, "config"), os.path.join(demoDir, "config"), demoConfigFiles)

copy(os.path.join(distFilesDir, "src", "common", "Make.rules"), os.path.join(demoDir, "config"), False)
copy(os.path.join(distFilesDir, "src", "common", "Make.rules.php"), os.path.join(demoDir, "config"), False)
copy(os.path.join(srcDir, "scripts"), os.path.join(demoscriptDir, "scripts"))

# Consolidate demoscript and demo distribution with files from each language mapping
for d in os.listdir('.'):
    if os.path.isdir(d) and os.path.exists(os.path.join(d, "allDemos.py")):
        md = os.path.join(demoscriptDir, d)
        td = os.path.join(demoscriptDir, d + "tmp")
        move(md, td)
        move(os.path.join(td, "demo"), md)
        move(os.path.join(d, "allDemos.py"), os.path.join(md, "allDemos.py"))
        os.rmdir(td)

    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        if d == "cs" or d == "vb" or d == "android":
            continue
        copy(os.path.join(d, "demo"), os.path.join(demoDir, d))

# Copy android demos into the java directory
copy(os.path.join("android", "demo"), os.path.join(demoDir, "java", "android"))

copy(os.path.join(srcDir, "js", "bin"), os.path.join(demoDir, "js", "bin"))
copy(os.path.join(srcDir, "js", "assets"), os.path.join(demoDir, "js", "assets"))
copy(os.path.join(srcDir, "js", "node_modules", "http-proxy"),
     os.path.join(demoDir, "js", "node_modules", "http-proxy"));

FixUtil.fileMatchAndReplace(os.path.join(demoDir, "js", "assets", "Makefile"),
                            [(re.compile("top_srcdir.*= .."), "top_srcdir      = ../..")],
                            False)
os.chdir(os.path.join(demoDir, "js", "assets"))
if os.system("MAKEDIST=yes make > /dev/null") != 0:
    print "Error building JS assets"
    sys.exit(1)
os.chdir(srcDir)

configSubstituteExprs = [(re.compile(regexpEscape("../../certs")), "../certs")]
for root, dirnames, filesnames in os.walk(demoDir):
    for f in filesnames:
        if fnmatch.fnmatch(f, "config*"):
            substitute(os.path.join(root, f), configSubstituteExprs)

remove(os.path.join(srcDir, 'vb')) # vb directory in Unix source distribution only needed to copy demo scripts.

# Fix up the Java build files
os.mkdir(os.path.join(demoDir, "java", "gradle"))
copy(os.path.join(srcDir, "java", "gradlew"), os.path.join(demoDir, "java"), False)
copy(os.path.join(srcDir, "java", "gradle", "wrapper"), os.path.join(demoDir, "java", "gradle", "wrapper"), False)
copy(os.path.join(srcDir, "java", "gradle", "GRADLE_LICENSE"), os.path.join(demoDir, "java", "gradle", "GRADLE_LICENSE"), False)
copy(os.path.join(distFilesDir, "src", "common", "ice.gradle"), os.path.join(demoDir, "java", "gradle"), False)
copy(os.path.join(distFilesDir, "src", "common", "gradle.properties"), os.path.join(demoDir, "java"), False)
copy(os.path.join(distFilesDir, "src", "common", "build.gradle"), os.path.join(demoDir, "java"), False)
copy(os.path.join(distFilesDir, "src", "common", "settings.gradle"), os.path.join(demoDir, "java"), False)

gradleSubstituteExprs = [(re.compile(re.escape("../java/gradle/ice.gradle")), "../gradle/ice.gradle"),
                         (re.compile(re.escape("project(\":demo/")), "project(\":")]
for root, dirnames, filesnames in os.walk(os.path.join(demoDir, "java")):
    for f in filesnames:
        if fnmatch.fnmatch(f, "build.gradle"):
            FixUtil.fileMatchAndReplace(os.path.join(root, f), gradleSubstituteExprs, False)

# Fix up the Android build files
os.mkdir(os.path.join(demoDir, "java", "android", "gradle"))
copy(os.path.join(srcDir, "android", "gradlew"), os.path.join(demoDir, "java", "android"), False)
copy(os.path.join(srcDir, "android", "gradle", "wrapper"), os.path.join(demoDir, "java", "android", "gradle", "wrapper"), False)
copy(os.path.join(srcDir, "android", "gradle", "GRADLE_LICENSE"), os.path.join(demoDir, "java", "android", "gradle", "GRADLE_LICENSE"), False)
copy(os.path.join(srcDir, "android", "build.gradle"), os.path.join(demoDir, "java", "android"), False)
copy(os.path.join(distFilesDir, "src", "common", "gradle.properties.android"), os.path.join(demoDir, "java", "android", "gradle.properties"), False)
copy(os.path.join(distFilesDir, "src", "common", "settings.gradle.android"), os.path.join(demoDir, "java", "android", "settings.gradle"), False)
copy(os.path.join(distFilesDir, "src", "common", "props.gradle"), os.path.join(demoDir, "java", "android", "gradle"), False)

gradleSubstituteExprs = [(re.compile(re.escape("apply plugin: 'slice'")), ""),
                         (re.compile(re.escape("../certs/client.bks")), "../../certs/client.bks")]
for root, dirnames, filesnames in os.walk(os.path.join(demoDir, "java", "android")):
    for f in filesnames:
        if fnmatch.fnmatch(f, "build.gradle"):
            FixUtil.fileMatchAndReplace(os.path.join(root, f), gradleSubstituteExprs, False)

copy(demoscriptDir, winDemoscriptDir)
shutil.rmtree(os.path.join(demoscriptDir, "cs"))
shutil.rmtree(os.path.join(demoscriptDir, "vb"))

# Windows demo distribution

copy(os.path.join(winDistFilesDir, "src", "common", "README.DEMOS.txt"), os.path.join(winDemoDir, "README.txt"))

copyMatchingFiles(os.path.join(winSrcDir, "certs"), os.path.join(winDemoDir, "certs"), demoCertsFiles)

os.mkdir(os.path.join(winDemoDir, "config"))

copy(os.path.join(winSrcDir, "config", "Make.common.rules.mak"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winSrcDir, "cpp", "config", "Make.rules.msvc"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winSrcDir, "js", "config", "build.js"), os.path.join(winDemoDir, "config"), False)
copy(os.path.join(winSrcDir, "js", "config", "Make.rules.mak.js"), os.path.join(winDemoDir, "config"), False)

copy(os.path.join(winDistFilesDir, "src", "common", "Make.rules.mak.php"), os.path.join(winDemoDir, "config"), False)

# Consolidate demo distribution with files from each language mapping
for sd in os.listdir(winSrcDir):
    if sd == "android":
        continue
    d = os.path.join(winSrcDir, sd)
    if os.path.isdir(d) and os.path.exists(os.path.join(d, "demo")):
        copy(os.path.join(d, "demo"), os.path.join(winDemoDir, sd))

# Copy android demos into the java directory
copy(os.path.join("android", "demo"), os.path.join(winDemoDir, "java", "android"))

copy(os.path.join(winSrcDir, "js", "bin"), os.path.join(winDemoDir, "js", "bin"))
copy(os.path.join(winSrcDir, "js", "assets"), os.path.join(winDemoDir, "js", "assets"))
copy(os.path.join(winSrcDir, "js", "node_modules", "http-proxy"),
     os.path.join(winDemoDir, "js", "node_modules", "http-proxy"));

for f in ["common.min.js", "common.min.js.gz", "common.css", "common.css.gz"]:
    copy(os.path.join(demoDir, "js", "assets", f),
         os.path.join(winDemoDir, "js", "assets", f))


FixUtil.fileMatchAndReplace(os.path.join(winDemoDir, "js", "assets", "Makefile.mak"),
                            [(re.compile("top_srcdir.*= .."), "top_srcdir      = ..\\..")],
                            False)

rmFiles = []

projectSubstituteExprs = [(re.compile(re.escape('"README"')), '"README.txt"'),
                          (re.compile(re.escape("PublicKeyToken=1f998c50fec78381")), "PublicKeyToken=cdd571ade22f2f16"),
                          (re.compile(re.escape("..\\..\\..\\..\\..\\certs\\cacert.der")),
                           "..\\..\\..\\..\\certs\\cacert.der")]

for root, dirnames, filesnames in os.walk(winDemoDir):
    for f in filesnames:

        if fnmatch.fnmatch(f, "README") and not fnmatch.fnmatch(f, "README.txt"):
            os.rename(os.path.join(root, f), os.path.join(root, f + ".txt"))
        elif fnmatch.fnmatch(f, "config*"):
            substitute(os.path.join(root, f), configSubstituteExprs)

        for m in [ "*.vcproj", "*.vcxproj", "*.vcxproj.filters", "*.csproj", "*.vbproj" ]:
            if fnmatch.fnmatch(f, m):
                FixUtil.fileMatchAndReplace(os.path.join(root, f), projectSubstituteExprs, False)

        for m in [ "Makefile", ".depend", "*.exe.config" ]:
            if fnmatch.fnmatch(f, m):
                rmFiles.append(os.path.join(root[len(winDemoDir) + 1:], f))

for d in ["cpp", "cs", "vb"]:
    for root, dirnames, filesnames in os.walk(os.path.join(winDemoDir, d)):
        for f in filesnames:
            for m in [ "Makefile.mak", ".depend.mak" ]:
                if fnmatch.fnmatch(f, m):
                    rmFiles.append(os.path.join(root[len(winDemoDir) + 1:], f))

for f in rmFiles: remove(os.path.join(winDemoDir, f))

# Fix up the Java build files
os.mkdir(os.path.join(winDemoDir, "java", "gradle"))
copy(os.path.join(srcDir, "java", "gradlew.bat"), os.path.join(winDemoDir, "java"), False)
copy(os.path.join(srcDir, "java", "gradle", "wrapper"), os.path.join(winDemoDir, "java", "gradle", "wrapper"), False)
copy(os.path.join(srcDir, "java", "gradle", "GRADLE_LICENSE"), os.path.join(winDemoDir, "java", "gradle", "GRADLE_LICENSE"), False)
copy(os.path.join(distFilesDir, "src", "common", "ice.gradle"), os.path.join(winDemoDir, "java", "gradle"), False)
copy(os.path.join(distFilesDir, "src", "common", "gradle.properties"), os.path.join(winDemoDir, "java"), False)
copy(os.path.join(distFilesDir, "src", "common", "build.gradle"), os.path.join(winDemoDir, "java"), False)
copy(os.path.join(distFilesDir, "src", "common", "settings.gradle"), os.path.join(winDemoDir, "java"), False)

gradleSubstituteExprs = [(re.compile(re.escape("../java/gradle/ice.gradle")), "../gradle/ice.gradle"),
                         (re.compile(re.escape("project(\":demo/")), "project(\":")]
for root, dirnames, filesnames in os.walk(os.path.join(winDemoDir, "java")):
    for f in filesnames:
        if fnmatch.fnmatch(f, "build.gradle"):
            FixUtil.fileMatchAndReplace(os.path.join(root, f), gradleSubstituteExprs, False)

# Fix up the Android build files
os.mkdir(os.path.join(winDemoDir, "java", "android", "gradle"))
copy(os.path.join(srcDir, "android", "gradlew.bat"), os.path.join(winDemoDir, "java", "android"), False)
copy(os.path.join(srcDir, "android", "gradle", "wrapper"), os.path.join(winDemoDir, "java", "android", "gradle", "wrapper"), False)
copy(os.path.join(srcDir, "android", "gradle", "GRADLE_LICENSE"), os.path.join(winDemoDir, "java", "android", "gradle", "GRADLE_LICENSE"), False)
copy(os.path.join(srcDir, "android", "build.gradle"), os.path.join(winDemoDir, "java", "android"), False)
copy(os.path.join(distFilesDir, "src", "common", "gradle.properties.android"), os.path.join(winDemoDir, "java", "android", "gradle.properties"), False)
copy(os.path.join(distFilesDir, "src", "common", "settings.gradle.android"), os.path.join(winDemoDir, "java", "android", "settings.gradle"), False)
copy(os.path.join(distFilesDir, "src", "common", "props.gradle"), os.path.join(winDemoDir, "java", "android", "gradle"), False)

gradleSubstituteExprs = [(re.compile(re.escape("apply plugin: 'slice'")), ""),
                         (re.compile(re.escape("../certs/client.bks")), "../../certs/client.bks")]
for root, dirnames, filesnames in os.walk(os.path.join(winDemoDir, "java", "android")):
    for f in filesnames:
        if fnmatch.fnmatch(f, "build.gradle"):
            FixUtil.fileMatchAndReplace(os.path.join(root, f), gradleSubstituteExprs, False)

print "ok"

#
# Create the Ice-rpmbuild archive
#
rpmBuildFiles = [ \
    os.path.join("src", "rpm", "*.conf"), \
    os.path.join("src", "rpm", "*.suse"), \
    os.path.join("src", "rpm", "*.redhat"), \
    os.path.join("src", "rpm", "*.amazon"), \
    os.path.join("src", "rpm", "*.service"), \
    os.path.join("src", "rpm", "ice.pth"), \
    os.path.join("src", "unix", "*Linux*"), \
    os.path.join("src", "unix", "*_LICENSE"), \
    os.path.join("src", "thirdparty", "php", "ice.ini"), \
]
copyMatchingFiles(os.path.join(distFilesDir), rpmBuildDir, rpmBuildFiles)

###### Debian source code distribution
copy(srcDir, debSrcDir, False)

#
# Copy CHANGES
#
copy(os.path.join(srcDir, "CHANGES"), os.path.join(distDir, "Ice-" + version + "-CHANGES"))
copy(os.path.join(distFilesDir, "src", "rpm", "RPM_README"), \
        os.path.join(distDir, "Ice-" + version + "-RPM_README"))
copy(os.path.join(distFilesDir, "src", "deb", "DEB_README"), \
        os.path.join(distDir, "Ice-" + version + "-DEB_README"))

#
# Everything should be clean now, we can create the source distributions archives
#
print "Archiving..."
sys.stdout.flush()
os.chdir(distDir)
for d in [srcDir, demoDir, distFilesDir, rpmBuildDir, debSrcDir]:
    tarArchive(d, verbose)

move(os.path.join(distDir, "zeroc-ice" + debmmversion + "-" + debversion + ".tar.gz"), \
     os.path.join(distDir, "zeroc-ice" + debmmversion + "_" + debversion + ".orig.tar.gz"))

for (dir, archiveDir) in [(demoscriptDir, "Ice-" + version + "-demos")]:
    tarArchive(dir, verbose, archiveDir)

for (dir, archiveDir) in [(winDemoscriptDir, "Ice-" + version + "-demos")]:
    zipArchive(dir, verbose, archiveDir)
os.rename(os.path.join(distDir, "demo-scripts.zip"), os.path.join(distDir, "Ice-" + version + "-demo-scripts.zip"))

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
writeSrcDistReport("Ice", version, tag, gitRepoDir, compareToDir,
                   [(srcDir + ".tar.gz", srcDir),
                    (demoDir + ".tar.gz", demoDir),
                    (rpmBuildDir + ".tar.gz", rpmBuildDir),
                    ("zeroc-ice" + debmmversion + "_" + debversion + ".orig.tar.gz", debSrcDir),
                    (demoscriptDir + ".tar.gz", demoscriptDir),
                    (os.path.join(distDir, "distfiles-" + version + ".zip"), winDistFilesDir),
                    (os.path.join(distDir, "Ice-" + version + ".zip"), winSrcDir),
                    (os.path.join(distDir, "Ice-" + version + "-demos.zip"), winDemoDir),
                    ])

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
remove(winDemoscriptDir)
remove(rpmBuildDir)
remove(distFilesDir)
remove(winDistFilesDir)
remove(debSrcDir)
print "ok"
