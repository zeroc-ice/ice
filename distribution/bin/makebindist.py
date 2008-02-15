#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, glob, fnmatch, string, re
from stat import *

version = "3.3.0"
#version = "@ver@"
distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(distDir, "lib"))
from DistUtils import *

#
# Defines which languges are supported on a given platform
#
languages = { \
    'SunOS' : ['cpp', 'java'], \
    'HP-UX' : ['cpp', 'java'], \
    'Darwin' : ['cpp', 'java', 'py'], \
    'Linux' : ['cpp', 'java', 'cs', 'py', 'rb', 'php'], \
}

#
# Defines third party dependencies for each supported platform and their default 
# location.
#
bzip2 = { 'HP-UX' : '/usr/local' }

berkeleydb = { \
    'SunOS' : '/opt/db', \
    'Darwin' : '/opt/db', \
    'HP-UX' : '/opt/db', \
}

berkeleydbjar = { \
    'Linux' : '/usr/share/java/db46/db.jar', \
}

expat = { \
    'SunOS' : '/usr/sfw', \
    'HP-UX' : '/usr/local', \
    'Darwin' : '/opt/expat', \
}

openssl = { \
    'SunOS' : '/usr/sfw', \
    'HP-UX' : '/opt/openssl', \
}

mcpp = { 
    'SunOS' : '/opt/mcpp', \
    'HP-UX' : '/opt/mcpp', \
    'Darwin' : '/opt/mcpp' 
}

jgoodies_looks = { \
    'SunOS' : '/share/opt/looks-2.1.4/looks-2.1.4.jar', \
    'HP-UX' : '/share/opt/looks-2.1.4/looks-2.1.4.jar', \
    'Darwin' : '/opt/looks-2.1.4/looks-2.1.4.jar', \
    'Linux' : '/opt/looks-2.1.4/looks-2.1.4.jar', \
}

jgoodies_forms = { \
    'SunOS' : '/share/opt/forms-1.1.0/forms-1.1.0.jar', \
    'HP-UX' : '/share/opt/forms-1.1.0/forms-1.1.0.jar', \
    'Darwin' : '/opt/forms-1.1.0/forms-1.1.0.jar', \
    'Linux' : '/opt/forms-1.1.0/forms-1.1.0.jar', \
}

proguard = { \
    'SunOS' : '/share/opt/proguard4.1/lib/proguard.jar', \
    'HP-UX' : '/share/opt/proguard4.1/lib/proguard.jar', \
    'Darwin' : '/opt/proguard/lib/proguard.jar', \
    'Linux' : '/opt/proguard/lib/proguard.jar', \
}    

class BerkeleyDB(ThirdParty):
    def __init__(self, platform, locations, jarlocations):
        ThirdParty.__init__(self, platform, "BerkeleyDB", locations, ["cpp", "java"], None, "DB_HOME")
        if not self.location:
            self.languages = ["java"]
            self.location = jarlocations.get(str(platform), None)

    def getJar(self):
        if self.location:
            if self.location.endswith(".jar"):
                return self.location
            else:
                return os.path.join(self.location, "lib", "db.jar")

    def getFiles(self, platform):
        files = [ os.path.join("lib", "db.jar"), os.path.join("bin", "db_*") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join("lib", "*"))
        files += platform.getSharedLibraryFiles(self.location, os.path.join("lib", "*"), "jnilib")
        return files

class Bzip2(ThirdParty):
    def __init__(self, platform, locations):
        ThirdParty.__init__(self, platform, "Bzip2", locations, ["cpp"])

    def getFiles(self, platform):
        return platform.getSharedLibraryFiles(self.location, os.path.join("lib", "*"))

class Expat(ThirdParty):
    def __init__(self, platform, locations):
        ThirdParty.__init__(self, platform, "Expat", locations, ["cpp"])

    def getFiles(self, platform):
        return platform.getSharedLibraryFiles(self.location, os.path.join("lib", "*"))

class OpenSSL(ThirdParty):
    def __init__(self, platform, locations):
        ThirdParty.__init__(self, platform, "OpenSSL", locations, ["cpp"])

    def getFiles(self, platform):
        files = [ os.path.join("bin", "openssl") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join("lib", "*"))
        return files

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] tag"
    print
    print "Options:"
    print "-c    Clean previous build"
    print "-n    Don't build any language mapping"
    print "-h    Show this message."
    print "-v    Be verbose."

#
# Instantiate the gobal platform object
#
(sysname, nodename, release, ver, machine) = os.uname();
if not languages.has_key(sysname):
    print sys.argv[0] + ": error: `" + sysname + "' is not a supported system"
platform = eval(sysname.replace("-", ""))(sysname, languages[sysname])

#
# Instantiate the third party libraries
#
thirdParties = [ \
    Bzip2(platform, bzip2), \
    BerkeleyDB(platform, berkeleydb, berkeleydbjar), \
    Expat(platform, expat), \
    OpenSSL(platform, openssl), \
    ThirdParty(platform, "Mcpp", mcpp, ["cpp"]), \
    ThirdParty(platform, "JGoodiesLooks", jgoodies_looks, ["java"], "jgoodies.looks"), \
    ThirdParty(platform, "JGoodiesForms", jgoodies_forms, ["java"], "jgoodies.forms"), \
    ThirdParty(platform, "Proguard", proguard, ["java"]), \
]

#
# Check arguments
#
verbose = 0
forceclean = 0
nobuild = 0
languages = [ ]
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-v":
        verbose = 1
    elif x == "-c":
        forceclean = 1
    elif x == "-n":
        nobuild = 1
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        if not x in platform.languages:
            print sys.argv[0] + ": language `" + x + "' not supported on this platform"
            print
            usage()
            sys.exit(1)
        languages.append(x)

if len(languages) == 0:
    languages = platform.languages

if nobuild:
    languages = []

if verbose:
    quiet = "v"
else:
    quiet = ""

#
# Ensure the script is being run from the dist-@ver@ directory.
#
cwd = os.getcwd()

if not os.path.exists(os.path.join(distDir, "src", "windows", "LICENSE.rtf")):
    print sys.argv[0] + ": you must run makebindist.py from the dist-" + version + " directory created by makedist.py"
    sys.exit(1)

print "Building Ice " + version + " binary distribution (" + platform.getPackageName(version) + ".tar.gz)"
print "Using the following third party libraries:"
if not platform.checkAndPrintThirdParties():
    print "error: some required third party dependencies were not found"
    sys.exit(1)
    
#
# Ensure that the source archive or directory exists and create the build directory.
#
buildRootDir = os.path.join(cwd, os.path.join("build-" + platform.pkgname + "-" + version))
srcDir = os.path.join(buildRootDir, "Ice-" + version + "-src")
buildDir = os.path.join(buildRootDir, "Ice-" + version)
if forceclean or not os.path.exists(srcDir) or not os.path.exists(buildDir):
    if os.path.exists(buildRootDir):
        print "Removing previous build from " + os.path.join("build-" + platform.pkgname + "-" + version) + "...",
        sys.stdout.flush()
        shutil.rmtree(buildRootDir)
        print "ok"
    os.mkdir(buildRootDir)

    #
    # If we can't find the source archive in the current directory, ask its location
    #
    if not os.path.exists(os.path.join(cwd, "Ice-" + version + ".tar.gz")):
        print
        src = raw_input("Couldn't find Ice-" + version + ".tar.gz in current directory, please specify\n" + \
                        "where to download or copy the source distribution or hit enter to \n" + \
                        "download it from sun:/share/srcdists/" + version + ": ")
        if src == "":
            src = "sun:/share/srcdists/" + version + "/Ice-" + version + ".tar.gz"
        elif not src.endswith("Ice-" + version + ".tar.gz"):
            src = os.path.join(src, "Ice-" + version + ".tar.gz")

        if os.system("scp " + src + " ."):
            print sys.argv[0] + ": couldn't copy " + src
            sys.exit(1)
        
    print "Unpacking ./Ice-" + version + ".tar.gz ...",
    sys.stdout.flush()
    os.chdir(buildRootDir)
    if os.system("gunzip -c " + os.path.join(cwd, "Ice-" + version + ".tar.gz") + " | tar x" + quiet + "f -"):
        print sys.argv[0] + ": failed to unpack ./Ice-" + version + ".tar.gz"
        sys.exit(1)
    os.rename("Ice-" + version, srcDir)
    os.chdir(cwd)
    print "ok"

    os.mkdir(buildDir)

#
# Build and install each language mappings supported by this platform in the build directory.
#
for l in languages:
    print
    print "============= Building " + l + " sources ============="
    print
    os.chdir(os.path.join(srcDir, l))

    if l != "java":
        if os.system("gmake " + platform.getMakeEnvs(version, l) + " prefix=" + buildDir + " install") != 0:
            print sys.argv[0] + ": `" + l + "' build failed"
            os.chdir(cwd)
            sys.exit(1)
    else:
        antCmd = platform.getAntEnv() + " ant " + platform.getAntOptions() + " -Dprefix=" + buildDir

        if os.system(antCmd + " -Dbuild.suffix=-java2 -Dice.mapping=java2 install") != 0 or \
           os.system(antCmd + " -Dbuild.suffix=-java5 -Dice.mapping=java5 install") != 0: 
           print sys.argv[0] + ": `" + l + "' build failed"
           os.chdir(cwd)
           sys.exit(1)

    os.chdir(os.path.join(cwd))
    print
    print "============= Finished buiding " + l + " sources ============="
    print

#
# Copy platform third party dependencies.
#
print "Copying third party dependencies..."
sys.stdout.flush()
platform.copyThirdPartyDependencies(buildDir)
platform.completeDistribution(buildDir, version)

#
# Copy platform specific files (README, SOURCES, etc)
#
print "Copying distribution files (README, SOURCES, etc)...",
sys.stdout.flush()
platform.copyDistributionFiles(distDir, buildDir)

if os.path.exists(os.path.join(buildDir, "doc")):
    shutil.rmtree(os.path.join(buildDir, "doc"))
print "ok"

if not os.path.exists("RELEASE_NOTES.txt"):
    print "warning: couldn't find ./RELEASE_NOTES.txt file"
else:
    copy("RELEASE_NOTES.txt", os.path.join(buildDir, "RELEASE_NOTES.txt"))

#
# Everything should be clean now, we can create the binary distribution archive
# 
print "Archiving " + platform.getPackageName(version) + ".tar.gz ...",
sys.stdout.flush()
os.chdir(buildRootDir)
tarfile = os.path.join(cwd, platform.getPackageName(version)) + ".tar.gz"
os.system("tar c" + quiet + "f - Ice-" + version + " | gzip -9 - > " + tarfile)
os.chdir(cwd)
print "ok"

#
# Done.
#
print "Cleaning up...",
sys.stdout.flush()
shutil.rmtree(buildRootDir)
print "ok"

os.chdir(cwd)
