#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, glob, fnmatch, string, re, fileinput, time, subprocess
from stat import *

def runCommand(cmd, verbose = False):
    if verbose:
        print(cmd)
    if os.system(cmd) != 0:
        print("Error executing: " + cmd)
        sys.exit(1)
#
# Defines which languges are supported on each supported platform
#
languages = { \
    'SunOS' : ['cpp', 'cpp-64', 'java'], \
    'Darwin' : ['cpp', 'java', 'py', 'js'], \
    'Linux' : ['cpp', 'cpp-11', 'java', 'cs', 'py', 'rb', 'php', 'js'], \
}

#
# Defines third party dependencies for each supported platform and their default
# location.
#
bzip2 = { \
}

berkeleydb = { \
    'Darwin' : '/Library/Developer/Ice-@ver@-ThirdParty', \
    'SunOS'  : '/opt/db', \
}

berkeleydbjar = { \
    'Linux' : '/usr/share/java/db-5.3.21.jar', \
}

expat = { \
}

iconv = {\
}

mcpp = {
    'SunOS' : '/opt/mcpp', \
    'Darwin' : '/Library/Developer/Ice-@ver@-ThirdParty'
}

openssl = { \
}

proguard = { \
    'SunOS' : '/opt/proguard/lib/proguard.jar', \
    'Darwin' : '/opt/proguard/lib/proguard.jar', \
    'Linux' : '/opt/proguard/lib/proguard.jar', \
}

javaApplicationBundler = { \
    'Darwin' : '/opt/appbundler-1.0.jar', \
}

#
# Some utility methods
#

#
# Remove file or directory, warn if it doesn't exist.
#
def remove(path, recurse = True):

    if not os.path.exists(path):
        print("warning: " + path + " doesn't exist")
        return

    if os.path.isdir(path):
        if recurse:
            shutil.rmtree(path)
        else:
            try:
                os.rmdir(path)
            except:
                pass
    else:
        os.remove(path)

#
# Check the git version
#
def checkGitVersion():
    # Ensure we're using git >= 1.8.3
    p = os.popen("git --version")
    gitVersionMatch = re.search(".* ([0-9]+)\.([0-9b]+)\.([0-9]+).*", p.read())
    p.close()

    version = (int(gitVersionMatch.group(1)) * 10000) + \
              (int(gitVersionMatch.group(2)) * 100) + \
              int(gitVersionMatch.group(3))

    if (version < 10803):
        print(sys.argv[0] + ": invalid git version, git >= 1.8.3 is required")
        sys.exit(1)

def getCommitForTag(repoDir, tag):

    cwd = os.getcwd()
    os.chdir(repoDir)

    try:
        p = os.popen("git show --show-signature %s" % tag)
        commit = p.read()
        p.close()
        if type(commit) != str:
            commit = commit.decode()
        commit = commit.split("\n")[0]
        if commit.find("commit") == -1:
            print("Error getting commit %s for tag" % tag)
            sys.exit(1)
        commit = re.sub("commit", "", commit).strip()
        os.chdir(cwd)
        return commit
    except subprocess.CalledProcessError as e:
        print(e)
        sys.exit(1)
#
# Copy src to dest
#
def copy(src, dest, warnDestExists = True, verbose = False):

    if verbose:
        print("copy: %s to: %s" %(src, dest))

    if not os.path.exists(src):
        print("warning: can't copy `" + src + "': file doesn't exist")
        return

    if not os.path.isdir(src) and os.path.isdir(dest):
        dest = os.path.join(dest, os.path.basename(src))

    if os.path.exists(dest):
        if warnDestExists:
            print("warning: overwritting " + dest)
        remove(dest)

    if os.path.dirname(dest) and not os.path.exists(os.path.dirname(dest)):
        os.makedirs(os.path.dirname(dest))

    if os.path.isdir(src):
        shutil.copytree(src, dest, True)
    elif os.path.islink(src):
        os.symlink(os.readlink(src), dest)
    else:
        shutil.copy2(src, dest)

    fixPermission(dest)

#
# Move src to dest
#
def move(src, dest, warnDestExists = True):

    if not os.path.exists(src):
        print("warning: can't move `" + src + "': file doesn't exist")
        return

    if os.path.exists(dest):
        if warnDestExists:
            print("warning: overwritting " + dest)
        shutil.rmtree(dest)

    if os.path.dirname(dest) and not os.path.exists(os.path.dirname(dest)):
        os.makedirs(os.path.dirname(dest))

    shutil.move(src, dest)
    fixPermission(dest)

#
# Copy files from srcpath and matching the given patterns to destpath
#
def copyMatchingFiles(srcpath, destpath, patterns, warnDestExists = True, verbose = False):
    for p in patterns:
        for f in glob.glob(os.path.join(srcpath, p)):
            copy(f, os.path.join(destpath, os.path.basename(f)), warnDestExists)

#
# Comment out rules in a Makefile.
#
def fixMakefileForFile(path):
    (dir,file) = os.path.split(path)
    (base,ext) = os.path.splitext(file)
    # File the makefile file from the same directory as the file.
    for f in glob.glob(os.path.join(dir, "Makefile*")):
        fixMakefile(f, base, ext)

#
# Comment out rules in a Makefile.
#
def fixMakefile(file, base, ext):

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
        if not x.startswith("\t") and x.find(base + ext) != -1:
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

        if doComment or (doCheck and x.find(base) != -1):
            x = "#" + x
        newLines.append(x)

    newMakefile.writelines(newLines)
    newMakefile.close()
    oldMakefile.close()
    os.remove(origfile)

#
# Comment out rules in VC project.
#
def fixProject(file, target):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldProject = open(origfile, "r")
    newProject = open(file, "w")
    origLines = oldProject.readlines()

    #
    # Find a Source File declaration containing SOURCE=<target>
    # and comment out the entire declaration.
    #
    expr = re.compile("SOURCE=.*" + target.replace(".", "\\.") + ".*")
    inSource = 0
    doComment = 0
    newLines = []
    source = []
    for x in origLines:
        if x.startswith("# Begin Source File"):
            inSource = 1

        if inSource:
            if not doComment and expr.match(x) != None:
                doComment = 1
            source.append(x)
        else:
            newLines.append(x)

        if x.startswith("# End Source File"):
            inSource = 0
            for s in source:
                if doComment:
                    newLines.append('#xxx#' + s)
                else:
                    newLines.append(s)
            doComment = 0
            source = []

    newProject.writelines(newLines)
    newProject.close()
    oldProject.close()
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
# Fix version in given file.
#
def fixVersion(file, version, mmversion = None, libversion = None, debversion = None, debmmversion = None, majorVersion = None, minorVersion = None):

    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    line = oldFile.read();
    line = re.sub("@ver@", version, line)
    if mmversion:
        line = re.sub("@mmver@", mmversion, line)
    if libversion:
        line = re.sub("@libver@", libversion, line)
    if debversion:
        line = re.sub("@debver@", debversion, line)
    if debversion:
        line = re.sub("@debmmver@", debmmversion, line)
    if majorVersion:
        line = re.sub("@majorver@", majorVersion, line)
    if minorVersion:
        line = re.sub("@minorver@", minorVersion, line)
    newFile.write(line)
    newFile.close()
    oldFile.close()

    # Preserve the executable permission
    st = os.stat(origfile)
    if st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH):
        os.chmod(file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
    os.remove(origfile)

#
# Comment out tests in allTest.py.
#
def fixAllTest(file, components):
    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    origLines = oldFile.readlines()

    ignore = False
    newLines = []
    for x in origLines:
        #
        # If the rule contains the target string, then
        # comment out this rule.
        #
        for c in components:
            if x.find(c +"/") != -1:
                ignore = True
                break

        if not ignore:
            newLines.append(x)
        ignore = False

    newFile.writelines(newLines)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

def regexpEscape(expr):
    escaped = ""
    for c in expr:
        # TODO: escape more characters?
        if c in ".\\/":
            escaped += "\\" + c
        else:
            escaped += c
    return escaped

def substitute(file, regexps):
    for line in fileinput.input(file, True):
        for (expr, text) in regexps:
            if not expr is re:
                expr = re.compile(expr)
            line = expr.sub(text, line)
        sys.stdout.write(line)
        sys.stdout.flush()

def fixFilePermission(file, verbose = False):

    patterns = [ \
        "*.h", \
        "*.cpp", \
        "*.ice", \
        "README*", \
        "*.xml", \
        "*.mc", \
        "Makefile", \
        "Makefile.mak", \
        "*.dsp", \
        ]

    st = os.stat(file)

    if st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH):
        for p in patterns:
            if fnmatch.fnmatch(file, p):
                if verbose:
                    print("removing exec permissions on: " + file)
                break
        else:
            os.chmod(file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
            return

    os.chmod(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) # rw-r--r--

def fixPermission(dest):

    if os.path.isdir(dest):
        os.chmod(dest, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
        for f in os.listdir(dest):
            fixPermission(os.path.join(dest, f))
    else:
        fixFilePermission(dest)


def tarArchive(dir, verbose = False, archiveDir = None):

    dist = os.path.basename(dir)
    sys.stdout.write("   creating " + dist + ".tar.gz... ")
    sys.stdout.flush()

    cwd = os.getcwd()
    os.chdir(os.path.dirname(dir))

    if verbose:
        quiet = "v"
    else:
        quiet = ""

    if archiveDir:
        os.mkdir("tmp")
        os.rename(dist, os.path.join("tmp", archiveDir))
        os.chdir("tmp")
        runCommand("tar c" + quiet + "f - " + archiveDir + " | gzip -9 - > " + os.path.join("..", dist) + ".tar.gz")
        os.chdir("..")
        os.rename(os.path.join("tmp", archiveDir), dir)
        os.rmdir("tmp")
    else:
        runCommand("tar c" + quiet + "f - " + dist + " | gzip -9 - > " + dist + ".tar.gz")

    os.chdir(cwd)
    print("ok")

def untarArchive(archive, verbose = False, archiveDir = None):

    if not os.path.exists(archive):
        print("couldn't find " + archive)
        return False

    if verbose:
        quiet = "v"
    else:
        quiet = ""

    if archiveDir:
        os.mkdir("tmp")
        os.chdir("tmp")
        runCommand("gunzip -c " + os.path.join("..", archive) + " | tar x" + quiet + "f -")
        os.rename(os.listdir(".")[0], os.path.join("..", archiveDir))
        os.chdir("..")
        os.rmdir("tmp")
    else:
        runCommand("gunzip -c " + archive + " | tar x" + quiet + "f -")

    return True

def zipArchive(dir, verbose = False, archiveDir = None):

    dist = os.path.basename(dir)
    sys.stdout.write("   creating " + dist + ".zip... ")
    sys.stdout.flush()

    cwd = os.getcwd()
    os.chdir(os.path.dirname(dir))

    if archiveDir:
        os.mkdir("tmp")
        os.rename(dist, os.path.join("tmp", archiveDir))
        os.chdir("tmp")
        if verbose:
            runCommand("zip -9r " + os.path.join("..", dist + ".zip ") + archiveDir)
        else:
            runCommand("zip -9rq " + os.path.join("..", dist +".zip ") + archiveDir)
        os.chdir("..")
        os.rename(os.path.join("tmp", archiveDir), dir)
        os.rmdir("tmp")
    else:
        if verbose:
            runCommand("zip -9r " + dist + ".zip " + dist)
        else:
            runCommand("zip -9rq " + dist +".zip " + dist)

    os.chdir(cwd)
    print("ok")

def unzipArchive(archive, verbose = False, archiveDir = None):

    if not os.path.exists(archive):
        print("couldn't find " + archive)
        return False

    if verbose:
        quiet = "v"
    else:
        quiet = ""

    if archiveDir:
        os.mkdir("tmp")
        os.chdir("tmp")
        if verbose:
            runCommand("unzip " + os.path.join("..", archive))
        else:
            runCommand("unzip -q " + os.path.join("..", archive))
        os.rename(os.listdir(".")[0], os.path.join("..", archiveDir))
        os.chdir("..")
        os.rmdir("tmp")
    else:
        runCommand("gunzip -c " + archive + " | tar x" + quiet + "f -")
        if verbose:
            runCommand("unzip " + archive)
        else:
            runCommand("unzip -q " + archive)

    return True

def compareDirs(orig, new):

    added = [ ]
    updated = [ ]
    removed = [ ]
    cwd = os.getcwd()

    for root, dirnames, filenames in os.walk(orig):
        for f in filenames:
            filepath = os.path.join(root[len(orig) + 1:], f)
            if not os.path.exists(os.path.join(new, filepath)):
                removed.append(os.path.join(new, filepath))
            else:
                pipe = os.popen("diff -q " + os.path.join(orig, filepath) + " " + os.path.join(new, filepath))
                if len(pipe.readlines()) > 0:
                    updated.append(os.path.join(new, filepath))
                pipe.close()

    for root, dirnames, filenames in os.walk(new):
        for f in filenames:
            filepath = os.path.join(root[len(new) + 1:], f)
            if not os.path.exists(os.path.join(orig, filepath)):
                added.append(os.path.join(new, filepath))

    return (added, updated, removed)

def writeSrcDistReport(product, version, tag, repoDir, compareToDir, distributions):

    cwd = os.getcwd()
    os.chdir(cwd)

    sys.stdout.write("Writing report in README...")
    sys.stdout.flush()
    readme = open("README", "w")
    print >>readme, "This directory contains the source distributions of " + product + " " + version + ".\n"
    print >>readme, "Version: " + version
    print >>readme, "Creation time: " + time.strftime("%a %b %d %Y, %I:%M:%S %p (%Z)")
    print >>readme, "Git commit: " + getCommitForTag(repoDir, tag)
    (sysname, nodename, release, ver, machine) = os.uname();
    print >>readme, "Host: " + nodename
    print >>readme, "Platform: " + sysname + " " + release
    if os.path.exists("/etc/redhat-release"):
        f = open("/etc/redhat-release")
        print >>readme, "Linux distribution: " + f.readline().strip()
        f.close()
    else:
        print >>readme, "Not created on a Linux distribution"
    print >>readme, "User: " + os.environ["USER"]
    print >>readme, ""

    def compare(distfile, distdir):
        distfile = os.path.basename(distfile)
        (dist, ext) = os.path.splitext(distfile)
        modifications = ([], [], [])

        if not os.path.exists(os.path.join(compareToDir, distfile)):
            return

        sys.stdout.write("   comparing " + distfile + "... ")
        sys.stdout.flush()

        success = False
        if distfile.endswith(".tar.gz"):
            success = untarArchive(os.path.join(compareToDir, distfile), False, dist + "-orig")
        elif distfile.endswith(".zip"):
            success = unzipArchive(os.path.join(compareToDir, distfile), False, dist + "-orig")

        if success:
            n = compareDirs(dist + "-orig", distdir)
            modifications = [ modifications[i] + n[i]  for i in range(len(modifications))]
            if n != ([], [], []):
                runCommand("diff -r -N " + dist + "-orig " + distdir + " > patch-" + distfile)
            remove(dist + "-orig")

        print("ok")

        if modifications != ([], [], []):
            (added, updated, removed) = modifications
            print >>readme
            print >>readme
            print >>readme, "*** Differences for ", distfile
            print >>readme
            for (desc, list) in [("Added", added), ("Removed", removed), ("Updated", updated)]:
                if len(list) > 0:
                    list.sort()
                    print >>readme
                    print >>readme, desc, "files:"
                    print >>readme, string.join(["=" for c in range(len(desc + " files:"))], '')
                    for f in list:
                        print >>readme, f

    if compareToDir:
        print
        print >>readme, "Comparison with", compareToDir
        for (distfile, distdir) in distributions:
            compare(distfile, distdir)

    else:
        print("ok")

    readme.close()

    os.chdir(cwd)

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

        self.defaultLocation = platform.getLocation(locations)
        self.location = os.environ.get(self.buildEnv, self.defaultLocation)


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
            print(self.name + ": <system>")
            return True
        else:
            if not os.path.exists(self.location):
                if os.environ.has_key(self.buildEnv):
                    print(self.name + ": not found at " + self.buildEnv + " location (" + self.location + ")")
                else:
                    print(self.name + ": not found at default location (" + self.location + ")")
                return False
            else:
                if os.environ.has_key(self.buildEnv):
                    print(self.name + ": " + self.location + " (from " + self.buildEnv + ")")
                else:
                    print(self.name + ": " + self.location + " (default location)")
                return True

    def getMakeEnv(self, language):
        if language in self.languages and not os.environ.has_key(self.buildEnv) and self.location:
            return self.buildEnv + "=" + self.location

    def getJar(self):
        if "java" in self.languages and self.location and self.location.endswith(".jar"):
            return self.location

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return []

    def getFiles(self, platform):
        files = self.getFilesFromSubDirs(platform, "bin", "lib", False)
        files += self.getFilesFromSubDirs(platform, "bin/c++11", "lib/c++11", False)
        if platform.lp64Libdir:
            files += self.getFilesFromSubDirs(platform, \
                                              os.path.join("bin", platform.lp64Bindir), \
                                              os.path.join("lib", platform.lp64Libdir), True)
        return files

    def includeInDistribution(self):
        # Only copy third party files installed in /opt or /Library/Developer
        return self.defaultLocation and (self.defaultLocation.startswith("/opt") or
                                         self.defaultLocation.startswith("/Library/Developer"))

    def copyToDistribution(self, platform, buildDir):
        if not self.location:
            return

        #
        # Get files/directories to copy. The path returned by getFiles() are relative
        # to the third party library location and might contain wildcards characters.
        #
        files = [f for path in self.getFiles(platform) for f in glob.glob(os.path.join(self.location, path))]
        if len(files) > 0:
            sys.stdout.write("  Copying " + self.name + "...")
            sys.stdout.flush()
            for src in files:
                copy(src, os.path.join(buildDir, src[len(self.location) + 1::]))
            print("ok")

#
# Platform helper classes
#
class Platform:
    def __init__(self, uname, pkgPlatform, pkgArch, languages, lp64Libdir, lp64Bindir, shlibExtension):
        self.uname = uname
        self.pkgPlatform = pkgPlatform
        self.pkgArch = pkgArch
        self.languages = languages
        self.lp64Libdir = lp64Libdir
        self.lp64Bindir = lp64Bindir
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
            sys.stdout.write("  ")
            sys.stdout.flush()
            found &= t.checkAndPrint()
        return found

    def getMakeEnvs(self, version, language):

        # Get third party environement variables.
        envs = [t.getMakeEnv(language) for t in self.thirdParties if t.getMakeEnv(language)]

        # Build with optimization by default.
        if not os.environ.has_key("OPTIMIZE"):
            envs.append("OPTIMIZE=yes")

        # Language specific environment variables to pass to make.
        if language == "cpp" or language == "cpp-64":
            envs.append("create_runpath_symlink=no")
        elif language == "cs":
            envs.append("NOGAC=1")

        # LP64
        if language == "cpp-64":
            envs.append("LP64=yes")
        else:
            envs.append("LP64=no")

        return string.join(envs, " ")

    def getJavaEnv(self):
        return "CLASSPATH=" + string.join([t.getJar() for t in self.thirdParties if t.getJar()], os.pathsep)
    
    def getJavaBuildOptions(self):
        return ""

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

        if self.pkgArch and os.path.exists(os.path.join(docDir, "README." + self.uname + "-" + self.pkgArch)):
            copy(os.path.join(docDir, "README." + self.uname + "-" + self.pkgArch), os.path.join(buildDir, "README"))
        elif os.path.exists(os.path.join(docDir, "README." + self.uname)):
            copy(os.path.join(docDir, "README." + self.uname), os.path.join(buildDir, "README"))
        else:
            print("warning: couldn't find README file for this binary distribution")

    def copyThirdPartyDependencies(self, buildDir):
        for t in filter(ThirdParty.includeInDistribution, self.thirdParties): t.copyToDistribution(self, buildDir)

    def completeDistribution(self, buildDir, version):
        pass

    def getPackageName(self, prefix, version):
        if self.pkgArch:
            return ("%s-" + version + "-bin-" + self.pkgPlatform + "-" + self.pkgArch) % prefix
        else:
            return ("%s-" + version + "-bin-" + self.pkgPlatform) % prefix

    def getMakeOptions(self):
        return ""

    def createArchive(self, cwd, buildRootDir, distDist, version, quiet):
        sys.stdout.write("Archiving " + self.getPackageName("Ice", version) + ".tar.gz ...")
        sys.stdout.flush()
        os.chdir(buildRootDir)
        tarfile = os.path.join(cwd, self.getPackageName("Ice", version)) + ".tar.gz"
        runCommand("tar c" + quiet + "f - Ice-" + version + " | gzip -9 - > " + tarfile)
        os.chdir(cwd)
        print("ok")

class Darwin(Platform):
    def __init__(self, uname, arch, languages):
        Platform.__init__(self, uname, "osx", None, languages, "", "", "dylib")

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

    def getMakeEnvs(self, version, language):
        envs = Platform.getMakeEnvs(self, version, language)
        # Build fat binaries by default.
        mmversion = re.search("([0-9]+\.[0-9b]+)[\.0-9]*", version).group(1)

        if not os.environ.has_key("CXXARCHFLAGS"):
            envs += " CXXARCHFLAGS=\"-arch i386 -arch x86_64\"";
            envs += " embedded_runpath_prefix=\"/Library/Developer/Ice-" + mmversion + "\""

        return envs

    def getMakeOptions(self):
        return "-j8"

    def completeDistribution(self, buildDir, version):

        print("Fixing python location")
        move(buildDir + '/python', buildDir + '/../python')
        print("ok")

        print("Fixing IceGrid Admin.app location")
        move(buildDir + '/bin/IceGrid Admin.app', buildDir + '/../IceGrid Admin.app')
        print("ok")

    def createArchive(self, cwd, buildRootDir, distDir, version, quiet):

        sys.stdout.write("Creating installer...")
        sys.stdout.flush()

        installerDir = os.path.join(buildRootDir, "installer")
        if os.path.exists(installerDir):
            shutil.rmtree(installerDir)
        os.mkdir(installerDir)

        packagesDir = os.path.join(buildRootDir, "packages")
        if os.path.exists(packagesDir):
            shutil.rmtree(packagesDir)
        os.mkdir(packagesDir)

        iceRootDir = "%s/Ice-@ver@" % buildRootDir
        for name in ["freeze", "glacier2", "ice", "icebox", "icediscovery", "icegrid", "icepatch2", "icestorm", "ice-gradle-plugin", "ant-ice"]:
            runCommand("cd %s/lib && ln -s %s-%s.jar %s.jar" % (iceRootDir, name, "@ver@", name))
            
        for name in ["freeze", "glacier2", "ice", "icebox", "icediscovery", "icegrid", "icepatch2", "icestorm"]:
            runCommand("cd %s/lib && ln -s %s-%s-source.jar %s-source.jar" % (iceRootDir, name, "@ver@", name))

        package = "com.zeroc.ice"
        packageRoot = os.path.join(buildRootDir, "Ice-@ver@")
        packageInstallLocation = "/Library/Developer/Ice-@ver@"

        runCommand("pkgbuild --root %s --identifier=%s --install-location=%s --version @ver@ %s/%s.pkg" % 
                  (packageRoot, package, packageInstallLocation, packagesDir, package))

        package = "com.zeroc.icepython"
        packageRoot = os.path.join(buildRootDir, "python")
        packageInstallLocation = "/Library/Python/2.7/site-packages"

        runCommand("pkgbuild --root %s --identifier=%s --install-location=%s --version @ver@ %s/%s.pkg" % 
                  (packageRoot, package, packageInstallLocation, packagesDir, package))

        package = "com.zeroc.icegridadmin"
        packageRoot = os.path.join(buildRootDir, "IceGrid Admin.app")
        packageInstallLocation = "/Applications/IceGrid Admin.app"

        runCommand("pkgbuild --root \"%s\" --identifier=%s --install-location=\"%s\" --version @ver@ %s/%s.pkg" % 
                  (packageRoot, package, packageInstallLocation, packagesDir, package))


        distribution = os.path.join(distDir, "src", "mac", "Ice", "distribution.xml")
        resources = os.path.join(distDir, "src", "mac", "Ice", "resources")
        scripts = os.path.join(distDir, "src", "mac", "Ice", "scripts")
        

        runCommand("productbuild --distribution=%s --resources=%s --scripts=%s --package-path=%s %s/Ice-@ver@.pkg" % 
                  (distribution, resources, scripts, packagesDir, installerDir))
        

        copy(os.path.join(distDir, "src", "mac", "Ice", "README.txt"), installerDir)
        copy(os.path.join(distDir, "src", "mac", "Ice", "uninstall.sh"), installerDir)
        print("ok")

        volname = "Ice-" + version
        sys.stdout.write( "Building disk image... " + volname + " ")
        sys.stdout.flush()

        if os.path.exists("scratch.dmg.sparseimage"):
            os.remove("scratch.dmg.sparseimage")
        runCommand("hdiutil create scratch.dmg -volname \"%s\" -type SPARSE -fs HFS+" % volname)
        runCommand("hdid scratch.dmg.sparseimage")
        runCommand("ditto -rsrc %s \"/Volumes/%s\"" % (installerDir, volname))
        runCommand("hdiutil detach \"/Volumes/%s\"" % volname)
        if os.path.exists(os.path.join(buildRootDir, "..", volname) + ".dmg"):
            os.remove(os.path.join(buildRootDir, "..", volname) + ".dmg")
        runCommand("hdiutil convert  scratch.dmg.sparseimage -format UDZO -o %s.dmg -imagekey zlib-devel=9" %
                  os.path.join(buildRootDir, "..", volname))
        os.remove("scratch.dmg.sparseimage")

        shutil.rmtree(installerDir)
        shutil.rmtree(packagesDir)
        print("ok")

class Linux(Platform):
    def __init__(self, uname, arch, languages):
        Platform.__init__(self, uname, "linux", arch, languages, "", "", "so")

class SunOS(Platform):
    def __init__(self, uname, arch, languages):
        if arch == "i86pc":
            Platform.__init__(self, uname, "solaris", "x86", languages, "64", "amd64", "so")
        else:
            Platform.__init__(self, uname, "solaris", "sparc", languages, "64", "sparcv9", "so")

    def getMakeOptions(self):
        return "-j 40"

#
# Third-party helper classes
#
class BerkeleyDB(ThirdParty):
    def __init__(self, platform):
        global berkeleydb, berkeleydbjar
        ThirdParty.__init__(self, platform, "BerkeleyDB", berkeleydb, ["cpp", "cpp-64"], None, "DB_HOME")

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        files = [ os.path.join(bindir, "db*") ]
        if not x64:
            files += [ os.path.join(libdir, "db.jar") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"), "jnilib")
        return files


class Bzip2(ThirdParty):
    def __init__(self, platform):
        global bzip2
        ThirdParty.__init__(self, platform, "Bzip2", bzip2, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))

class Expat(ThirdParty):
    def __init__(self, platform):
        global expat
        ThirdParty.__init__(self, platform, "Expat", expat, ["cpp", "cpp-64"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libexpat*"))

class OpenSSL(ThirdParty):
    def __init__(self, platform):
        global openssl
        ThirdParty.__init__(self, platform, "OpenSSL", openssl, ["cpp", "cpp-64"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        files = [ os.path.join(bindir, "openssl") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libssl*"))
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libcrypto*"))
        return files

class Mcpp(ThirdParty):
    def __init__(self, platform):
        global mcpp
        ThirdParty.__init__(self, platform, "Mcpp", mcpp, ["cpp", "cpp-64"])

class Iconv(ThirdParty):
    def __init__(self, platform):
        global iconv
        ThirdParty.__init__(self, platform, "Iconv", iconv, ["cpp", "cpp-64"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        files = platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libiconv*"))
        # We also need some symbolic links
        files += [os.path.join(self.location, os.path.join(libdir, "libiconv." + platform.shlibExtension + ".2")),
                  os.path.join(self.location, os.path.join(libdir, "libiconv." + platform.shlibExtension))]
        return files

class Proguard(ThirdParty):
    def __init__(self, platform):
        global proguard
        ThirdParty.__init__(self, platform, "Proguard", proguard, ["java"])

class JavaApplicationBundler(ThirdParty):
    def __init__(self, platform):
        global javaApplicationBundler
        ThirdParty.__init__(self, platform, "JavaApplicationBundler", javaApplicationBundler, ["java"])


platform = None
def getPlatform(thirdParties):

    global platform
    if not platform:
        (sysname, nodename, release, ver, machine) = os.uname();
        if not languages.has_key(sysname):
            print(sys.argv[0] + ": error: `" + sysname + "' is not a supported system")

        if sysname == "Linux":
            p = subprocess.Popen("lsb_release -i", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
            if(p.wait() != 0):
                print("lsb_release failed:\n" + p.stdout.read().strip())
                sys.exit(1)
            distribution = re.sub("Distributor ID:", "", p.stdout.readline().decode('UTF-8')).strip()
            if distribution.find("RedHat") != -1:
                languages[sysname].remove("cs")

        platform = eval(sysname.replace("-", ""))(sysname, machine, languages[sysname])
        for t in thirdParties:
            eval(t)(platform)
    return platform
