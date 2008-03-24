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

#
# Defines which languges are supported on each supported platform
#
languages = { \
    'SunOS' : ['cpp', 'java'], \
    'HP-UX' : ['cpp'], \
    'Darwin' : ['cpp', 'java', 'py'], \
    'Linux' : ['cpp', 'java', 'cs', 'py', 'rb', 'php'], \
}

#
# Defines which languages are to also be built in 64bits mode
#
# NOTE: makebindist.py doesn't currently support different third party locations
# for 32 and 64 bits. This is an issue on HP-UX for example where Bzip2 32bits is
# in /usr/local and in /opt for the 64bits version.
#
build_lp64 = { \
    'SunOS' : ['cpp'], \
}

#
# Defines third party dependencies for each supported platform and their default 
# location.
#
bzip2 = { \
    'HP-UX' : '/usr/local', \
}

berkeleydb = { \
    'SunOS' : '/opt/db', \
    'Darwin' : '/opt/db', \
    'HP-UX' : '/opt/db', \
}

berkeleydbjar = { \
    'Linux' : '/usr/share/java/db-4.6.21.jar', \
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
    'Darwin' : '/opt/looks-2.1.4/looks-2.1.4.jar', \
    'Linux' : '/opt/looks-2.1.4/looks-2.1.4.jar', \
}

jgoodies_forms = { \
    'SunOS' : '/share/opt/forms-1.2.0/forms-1.2.0.jar', \
    'Darwin' : '/opt/forms-1.2.0/forms-1.2.0.jar', \
    'Linux' : '/opt/forms-1.2.0/forms-1.2.0.jar', \
}

proguard = { \
    'SunOS' : '/share/opt/proguard4.1/lib/proguard.jar', \
    'Darwin' : '/opt/proguard/lib/proguard.jar', \
    'Linux' : '/opt/proguard/lib/proguard.jar', \
}    

#
# Some utility methods
# 
def fixPermission(dest):

    if os.path.isdir(dest):
        os.chmod(dest, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
        for f in os.listdir(dest):
            fixPermission(os.path.join(dest, f))
    else:
        if os.stat(dest).st_mode & (S_IXUSR | S_IXGRP | S_IXOTH):
            os.chmod(dest, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
        else:
            os.chmod(dest, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) # rw-r--r--

def copy(src, dest):

    if not os.path.exists(src):
        print "warning: can't copy `" + src + "': file doesn't exist"
        return

    # Create the directories if necessary.
    if not os.path.exists(os.path.dirname(dest)):
        os.makedirs(os.path.dirname(dest))
        
    # Copy the directory, link or file.
    if os.path.isdir(src):
        shutil.copytree(src, dest)
    elif os.path.islink(src):
        if os.path.exists(dest):
            os.remove(dest)
        os.symlink(os.readlink(src), dest)
    else:
        if os.path.exists(dest):
            os.remove(dest)
        shutil.copy(src, dest)

    fixPermission(dest)

#
# Third-party helper base class
#
class ThirdParty :
    def __init__(self, platform, name, locations, languages, buildOption = None, buildEnv = None):
        self.name = name
        self.languages = languages
        self.buildOption = buildOption
        if buildEnv:
            self.buildEnv = buildEnv
        else:
            self.buildEnv = self.name.upper() + "_HOME"

        #
        # Get the location of the third party dependency. We first check if the environment
        # variable (e.g.: DB_HOME) is set, if not we use the platform specific location.
        #
        self.location = os.environ.get(self.buildEnv, platform.getLocation(locations))

        if self.location and os.path.islink(self.location):
            self.location = os.path.normpath(os.path.join(os.path.dirname(self.location), os.readlink(self.location)))

        #
        # Add the third party dependency to the platform object.
        #
        platform.addThirdParty(self)
            
    def __str__(self):
        return self.name

    def checkAndPrint(self):

        if self.location == None:
            print self.name + ": <system>"
            return True
        else:
            if not os.path.exists(self.location):
                if os.environ.has_key(self.buildEnv):
                    print self.name + ": not found at " + self.buildEnv + " location (" + self.location + ")"
                else:
                    print self.name + ": not found at default location (" + self.location + ")"
                return False
            else:
                if os.environ.has_key(self.buildEnv):
                    print self.name + ": " + self.location + " (from " + self.buildEnv + ")"
                else:
                    print self.name + ": " + self.location + " (default location)"
                return True
            
    def getMakeEnv(self, language):
        if language in self.languages and not os.environ.has_key(self.buildEnv) and self.location:
            return self.buildEnv + "=" + self.location

    def getAntOption(self):
        if "java" in self.languages and self.buildOption and self.location:
            return "-D" + self.buildOption + "=" + self.location

    def getJar(self):
        if "java" in self.languages and self.location and self.location.endswith(".jar"):
            return self.location

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return []

    def getFiles(self, platform):
        files = self.getFilesFromSubDirs(platform, "bin", "lib", False)
        if platform.lp64subdir:
            files += self.getFilesFromSubDirs(platform, \
                                              os.path.join("bin", platform.lp64subdir), \
                                              os.path.join("lib", platform.lp64subdir), True)
        return files

    def includeInDistribution(self):
        # Only copy third party files installed in /opt
        return self.location and self.location.startswith("/opt")

    def copyToDistribution(self, platform, buildDir):
        if not self.location:
            return

        #
        # Get files/directories to copy. The path returned by getFiles() are relative 
        # to the third party library location and might contain wildcards characters.
        #
        files = [f for path in self.getFiles(platform) for f in glob.glob(os.path.join(self.location, path))]
        if len(files) > 0:
            print "  Copying " + self.name + "...",
            sys.stdout.flush()
            for src in files:
                copy(src, os.path.join(buildDir, src[len(self.location) + 1::]))
            print "ok"

#
# Platform helper classes
#
class Platform:
    def __init__(self, uname, pkgname, arch, languages, build_lp64, lp64subdir, shlibExtension):
        self.uname = uname
        self.pkgname = pkgname
        self.arch = arch
        self.languages = languages
        if not build_lp64:
            self.build_lp64 = { }
            self.lp64subdir = None
        else:
            self.build_lp64 = build_lp64
            self.lp64subdir = lp64subdir
        self.shlibExtension = shlibExtension
        self.thirdParties = []

    def __str__(self):
        return self.__class__.__name__

    def getLocation(self, locations):
        return locations.get(self.uname, None)

    def addThirdParty(self, thirdParty):
        self.thirdParties.append(thirdParty)

    def checkAndPrintThirdParties(self):
        found = True
        for t in self.thirdParties:
            print "  ",
            found &= t.checkAndPrint()
        return found

    def getPackageName(self, version):
        if self.arch:
            return "Ice-" + version + "-bin-" + self.pkgname + "-" + self.arch
        else:
            return "Ice-" + version + "-bin-" + self.pkgname


    def getMakeEnvs(self, version, language):

        # Get third party environement variables.
        envs = [t.getMakeEnv(language) for t in self.thirdParties if t.getMakeEnv(language)]

        # Build with optimization by default.
        if not os.environ.has_key("OPTIMIZE"):
            envs.append("OPTIMIZE=yes")

        # Language specific environment variables to pass to make.
        if language == "cpp":
            envs.append("create_runpath_symlink=no")
        elif language == "cs":
            envs.append("NOGAC=1")

        return string.join(envs, " ")

    def getAntEnv(self):
        return "CLASSPATH=" + string.join([t.getJar() for t in self.thirdParties if t.getJar()], os.pathsep)

    def getAntOptions(self):
        return string.join([t.getAntOption() for t in self.thirdParties if t.getAntOption()], " ")

    def getSharedLibraryFiles(self, root, path, extension = None): 
        if not extension:
            extension = self.shlibExtension
        libs = []
        for f in glob.glob(os.path.join(root, path)):
            (dirname, basename) = os.path.split(f)
            if fnmatch.fnmatch(basename, "*." + extension + "*") and not os.path.islink(f):
                libs.append(os.path.join(dirname[len(root) + 1::], basename))
        return libs

    def copyDistributionFiles(self, distDir, buildDir):
        docDir = os.path.join(distDir, "src", "unix")
        for f in [ 'SOURCES', 'THIRD_PARTY_LICENSE' ]:
            copy(os.path.join(docDir, f + "." + self.uname), os.path.join(buildDir, f))

        if self.arch and os.path.exists(os.path.join(docDir, "README." + self.uname + "-" + self.arch)):
            copy(os.path.join(docDir, "README." + self.uname + "-" + self.arch), os.path.join(buildDir, "README"))
        elif os.path.exists(os.path.join(docDir, "README." + self.uname)):
            copy(os.path.join(docDir, "README." + self.uname), os.path.join(buildDir, "README"))
        else:
            print "warning: couldn't find README file for this binary distribution"
            
    def copyThirdPartyDependencies(self, buildDir):
        for t in filter(ThirdParty.includeInDistribution, self.thirdParties): t.copyToDistribution(self, buildDir)

    def completeDistribution(self, buildDir, version):
        pass

class Darwin(Platform):
    def __init__(self, uname, arch, languages, build_lp64):
        Platform.__init__(self, uname, "macosx", None, languages, build_lp64, "", "dylib")

    def getSharedLibraryFiles(self, root, path, extension = None) : 
        libraries = Platform.getSharedLibraryFiles(self, root, path, extension)
        links = []
        for l in libraries:
            out = os.popen("otool -D " + os.path.join(root, l))
            lines = out.readlines()
            out.close()
            if(len(lines) <= 1):
                continue
            link = lines[1].strip()
            if link != os.path.join(root, l) and link.startswith(root):
                links.append(link[len(root) + 1::])
        return libraries + links

    def completeDistribution(self, buildDir, version):

        print "Fixing install names...",
        sys.stdout.flush()

        isLib = lambda f: (fnmatch.fnmatch(f, "*dylib") or fnmatch.fnmatch(f, "*jnilib")) and not os.path.islink(f)
        isExe = lambda f : os.system('file -b ' + f + ' | grep -q "Mach-O"') == 0

        #
        # Find the install names of the third party libraries included with the distribution.
        #
        oldInstallNames = []
        for t in self.thirdParties:
            if t.includeInDistribution():
                for l in filter(isLib, [os.path.join(buildDir, l) for l in t.getFiles(self)]):
                    p = os.popen('otool -D ' + l + ' | tail -1')
                    oldInstallNames.append(p.readline().strip())
                    p.close()
                    
        #
        # Find the binary files included with this distribution.
        #
        binFiles = [ f for f in glob.glob(os.path.join(buildDir, "bin", "*")) if isExe(f)]
        binFiles += [ f for f in glob.glob(os.path.join(buildDir, "lib", "*")) if isLib(f)]

        #
        # Fix the install names in each binary.
        #
        mmversion = re.search("([0-9]+\.[0-9b]+)[\.0-9]*", version).group(1)
        for oldName in oldInstallNames:
            libName = re.sub("\/opt\/.*\/(.*)", "\\1", oldName)
            newName = '/opt/Ice-' + mmversion + '/' + libName
            os.system('install_name_tool -id ' + newName + ' ' + buildDir + '/lib/' + libName)
            for f in binFiles:
                os.system('install_name_tool -change ' + oldName + ' ' + newName + ' ' + f)

        print "ok"

class HPUX(Platform):
    def __init__(self, uname, arch, languages, build_lp64):
        Platform.__init__(self, uname, "hpux", None, languages, build_lp64, "", "sl")

class Linux(Platform):
    def __init__(self, uname, arch, languages, build_lp64):
        Platform.__init__(self, uname, "linux", arch, languages, build_lp64, "", "so")

class SunOS(Platform):
    def __init__(self, uname, arch, languages, build_lp64):
        if arch == "i86pc":
            Platform.__init__(self, uname, "solaris", "x86", languages, build_lp64, "amd64", "so")
        else:
            Platform.__init__(self, uname, "solaris", "sparc", languages, build_lp64, "sparcv9", "so")

#
# Third-party helper classes 
#
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

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        files = [ os.path.join(bindir, "db_*") ]
        if not x64:
            files += [ os.path.join(libdir, "db.jar") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"), "jnilib")
        return files

class Bzip2(ThirdParty):
    def __init__(self, platform, locations):
        ThirdParty.__init__(self, platform, "Bzip2", locations, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))

class Expat(ThirdParty):
    def __init__(self, platform, locations):
        ThirdParty.__init__(self, platform, "Expat", locations, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))

class OpenSSL(ThirdParty):
    def __init__(self, platform, locations):
        ThirdParty.__init__(self, platform, "OpenSSL", locations, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        files = [ os.path.join(bindir, "openssl") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))
        return files

platform = None
def getPlatform():

    global platform
    if not platform:
        (sysname, nodename, release, ver, machine) = os.uname();
        if not languages.has_key(sysname):
            print sys.argv[0] + ": error: `" + sysname + "' is not a supported system"
        platform = eval(sysname.replace("-", ""))(sysname, machine, languages[sysname], build_lp64.get(sysname, None))

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

    return platform
