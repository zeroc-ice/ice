#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, glob, fnmatch, string, re, fileinput, time
from stat import *

#
# Defines which languges are supported on each supported platform
#
languages = { \
    'SunOS' : ['cpp', 'java'], \
    'Darwin' : ['cpp', 'java', 'py'], \
    'Linux' : ['cpp', 'java', 'cs', 'py', 'rb', 'php'], \
}

#
# Defines third party dependencies for each supported platform and their default 
# location.
#
bzip2 = { \
}

berkeleydb = { \
    'SunOS' : '/opt/db', \
    'Darwin' : '/opt/db', \
}

berkeleydbjar = { \
    'Linux' : '/usr/share/java/db-4.8.24.jar', \
}

expat = { \
    'SunOS' : '/usr/sfw', \
    'Darwin' : '/opt/expat', \
}

openssl = { \
    'SunOS' : '/usr/sfw', \
}

mcpp = { 
    'SunOS' : '/opt/mcpp', \
    'Darwin' : '/opt/mcpp' 
}

qt = { \
    'SunOS' : '/opt/qt', \
    'Darwin' : '/Library/Frameworks', \
}

jgoodies_looks = { \
    'SunOS' : '/usr/share/java/looks-2.3.0.jar', \
    'Darwin' : '/opt/looks-2.3.0/looks-2.3.0.jar', \
    'Linux' : '/opt/looks-2.3.0/looks-2.3.0.jar', \
}

jgoodies_forms = { \
    'SunOS' : '/usr/share/java/forms-1.2.1.jar', \
    'Darwin' : '/opt/forms-1.2.1/forms-1.2.1.jar', \
    'Linux' : '/opt/forms-1.2.1/forms-1.2.1.jar', \
}

proguard = { \
    'SunOS' : '/usr/share/java/proguard.jar', \
    'Darwin' : '/opt/proguard/lib/proguard.jar', \
    'Linux' : '/opt/proguard/lib/proguard.jar', \
}

#
# Some utility methods
# 

#
# Remove file or directory, warn if it doesn't exist.
#
def remove(path, recurse = True):

    if not os.path.exists(path):
        print "warning: " + path + " doesn't exist"
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
# Copy src to dest
# 
def copy(src, dest, warnDestExists = True):

    if not os.path.exists(src):
        print "warning: can't copy `" + src + "': file doesn't exist"
        return

    if not os.path.isdir(src) and os.path.isdir(dest):
        dest = os.path.join(dest, os.path.basename(src))

    if os.path.exists(dest):
        if warnDestExists:
            print "warning: overwritting " + dest
        remove(dest)

    if os.path.dirname(dest) and not os.path.exists(os.path.dirname(dest)):
        os.makedirs(os.path.dirname(dest))

    if os.path.isdir(src):
        shutil.copytree(src, dest, True)
    elif os.path.islink(src):
        os.symlink(os.readlink(src), dest)
    else:
        shutil.copy(src, dest)

    fixPermission(dest)

#
# Move src to dest
#
def move(src, dest, warnDestExists = True):

    if not os.path.exists(src):
        print "warning: can't move `" + src + "': file doesn't exist"
        return

    if os.path.exists(dest):
        if warnDestExists:
            print "warning: overwritting " + dest
        shutil.rmtree(dest)

    if os.path.dirname(dest) and not os.path.exists(os.path.dirname(dest)):
        os.makedirs(os.path.dirname(dest))

    shutil.move(src, dest)
    fixPermission(dest)

#
# Copy files from srcpath and matching the given patterns to destpath
#
def copyMatchingFiles(srcpath, destpath, patterns, warnDestExists = True):
    for p in patterns:
        for f in glob.glob(os.path.join(srcpath, p)):
            copy(f, os.path.join(destpath, os.path.basename(f)), warnDestExists)

#
# Get the language mapping directory for a given suffix.
#
def getMappingDir(suffix, mapping):
    if mapping == "cpp":
        return suffix
    elif mapping == "java":
        return suffix + "j"
    else:
        return suffix + mapping

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
def fixVersion(file, version, mmversion = None, libversion = None):

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
    newFile.write(line)
    newFile.close()
    oldFile.close()

    # Preserve the executable permission
    st = os.stat(origfile)
    if st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH): 
        os.chmod(file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) # rwxr-xr-x
    os.remove(origfile)

#
# Generate bison files and comment out the Makefile rule
#
def generateBisonFile(x, verbose = False):

    #
    # Change to the directory containing the file.
    #
    (dir,file) = os.path.split(x)
    cwd = os.getcwd()
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
    fixMakefile("Makefile", base, ext)
    fixMakefile("Makefile.mak", base, ext)

    #
    # Edit the project file(s) to comment out the grammar rules.
    #
    for p in glob.glob("*.dsp"):
        fixProject(p, file)
    os.chdir(cwd)

#
# Generate flex files and comment out the Makefile rule
#
def generateFlexFile(x, verbose = False):

    #
    # Change to the directory containing the file.
    #
    (dir,file) = os.path.split(x)
    cwd = os.getcwd()
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
    fixMakefile("Makefile", base, ext)
    fixMakefile("Makefile.mak", base, ext)

    #
    # Edit the project file(s) to comment out the flex rules.
    #
    for p in glob.glob("*.dsp"):
        fixProject(p, file)

    os.chdir(cwd)

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
        print line,

def fixFilePermission(file, verbose = False):

    patterns = [ \
        "*.h", \
        "*.cpp", \
        "*.ice", \
        "README*", \
        "INSTALL*", \
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
                    print "removing exec permissions on: " + file
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
    print "   creating " + dist + ".tar.gz ...",
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
        os.system("tar c" + quiet + "f - " + archiveDir + " | gzip -9 - > " + os.path.join("..", dist) + ".tar.gz")
        os.chdir("..")
        os.rename(os.path.join("tmp", archiveDir), dir)
        os.rmdir("tmp")
    else:
        os.system("tar c" + quiet + "f - " + dist + " | gzip -9 - > " + dist + ".tar.gz")

    os.chdir(cwd)
    print "ok"

def untarArchive(archive, verbose = False, archiveDir = None):

    if not os.path.exists(archive):
        print "couldn't find " + archive
        return False

    if verbose:
        quiet = "v"
    else:
        quiet = ""

    if archiveDir:
        os.mkdir("tmp")
        os.chdir("tmp")
        os.system("gunzip -c " + os.path.join("..", archive) + " | tar x" + quiet + "f -")
        os.rename(os.listdir(".")[0], os.path.join("..", archiveDir))
        os.chdir("..")
        os.rmdir("tmp")
    else:
        os.system("gunzip -c " + archive + " | tar x" + quiet + "f -")

    return True

def zipArchive(dir, verbose = False):

    dist = os.path.basename(dir)
    print "   creating " + dist + ".zip ...",
    sys.stdout.flush()

    cwd = os.getcwd()
    os.chdir(os.path.dirname(dir))

    if verbose:
        os.system("zip -9r " + dist + ".zip " + dist)
    else:
        os.system("zip -9rq " + dist +".zip " + dist)

    os.chdir(cwd)
    print "ok"

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

def writeSrcDistReport(product, version, compareToDir, distributions):

    cwd = os.getcwd()
    os.chdir(cwd)

    print "Writing report in README...",
    readme = open("README", "w")
    print >>readme, "This directory contains the source distributions of " + product + " " + version + ".\n"
    print >>readme, "Creation time: " + time.strftime("%a %b %d %Y, %I:%M:%S %p (%Z)")
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

    if compareToDir:
        print
        print >>readme, "Comparison with", compareToDir
        modifications = ([], [], [])
        for dist in distributions:
            dist = os.path.basename(dist)
            print "   comparing " + dist + " ...",
            sys.stdout.flush()
            if untarArchive(os.path.join(compareToDir, dist) + ".tar.gz", False, dist + "-orig"):
                n = compareDirs(dist + "-orig", dist)
                modifications = [ modifications[i] + n[i]  for i in range(len(modifications))]
                if n != ([], [], []):
                    os.system("diff -r -N " + dist + "-orig " + dist + " > patch-" + dist)
                remove(dist + "-orig")
            print "ok"

        (added, updated, removed) = modifications
        for (desc, list) in [("Added", added), ("Removed", removed), ("Updated", updated)]:
            if len(list) > 0:
                list.sort()
                print >>readme
                print >>readme, desc, "files:"
                print >>readme, string.join(["=" for c in range(len(desc + " files:"))], '')
                for f in list:
                    print >>readme, f

    else:
        print "ok"
        
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
    def __init__(self, uname, pkgPlatform, pkgArch, languages, lp64subdir, shlibExtension):
        self.uname = uname
        self.pkgPlatform = pkgPlatform
        self.pkgArch = pkgArch
        self.languages = languages
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

        if self.pkgArch and os.path.exists(os.path.join(docDir, "README." + self.uname + "-" + self.pkgArch)):
            copy(os.path.join(docDir, "README." + self.uname + "-" + self.pkgArch), os.path.join(buildDir, "README"))
        elif os.path.exists(os.path.join(docDir, "README." + self.uname)):
            copy(os.path.join(docDir, "README." + self.uname), os.path.join(buildDir, "README"))
        else:
            print "warning: couldn't find README file for this binary distribution"
            
    def copyThirdPartyDependencies(self, buildDir):
        for t in filter(ThirdParty.includeInDistribution, self.thirdParties): t.copyToDistribution(self, buildDir)

    def completeDistribution(self, buildDir, version):
        pass

    def getPackageName(self, prefix, version):
        if self.pkgArch:
            return ("%s-" + version + "-bin-" + self.pkgPlatform + "-" + self.pkgArch) % prefix
        else:
            return ("%s-" + version + "-bin-" + self.pkgPlatform) % prefix

    def getJGoodiesForms(self):
	for t in self.thirdParties:
            if t.__str__() == "JGoodiesForms":
        	return t.getJar()
	print "Unable to find JGoodiesForms"
	sys.exit(1)

    def getJGoodiesLooks(self):
	for t in self.thirdParties:
            if t.__str__() == "JGoodiesLooks":
        	return t.getJar()
	print "Unable to find JGoodiesLooks"
	sys.exit(1)

class Darwin(Platform):
    def __init__(self, uname, arch, languages):
        Platform.__init__(self, uname, "macosx", None, languages, "", "dylib")

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
        if not os.environ.has_key("CXXARCHFLAGS"):
            envs += " CXXARCHFLAGS=\"-arch i386 -arch x86_64\"";
        return envs

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
            libName = os.path.basename(oldName)
            newName = '/opt/Ice-' + mmversion + '/lib/' + libName
            os.system('install_name_tool -id ' + newName + ' ' + buildDir + '/lib/' + libName)
            for f in binFiles:
                os.system('install_name_tool -change ' + oldName + ' ' + newName + ' ' + f)

        print "ok"

class Linux(Platform):
    def __init__(self, uname, arch, languages):
        Platform.__init__(self, uname, "linux", arch, languages, "", "so")

class SunOS(Platform):
    def __init__(self, uname, arch, languages):
        if arch == "i86pc":
            Platform.__init__(self, uname, "solaris", "x86", languages, "amd64", "so")
        else:
            Platform.__init__(self, uname, "solaris", "sparc", languages, "sparcv9", "so")

#
# Third-party helper classes 
#
class BerkeleyDB(ThirdParty):
    def __init__(self, platform):
        global berkeleydb, berkeleydbjar
        ThirdParty.__init__(self, platform, "BerkeleyDB", berkeleydb, ["cpp", "java"], None, "DB_HOME")
        if not self.location: # BerkeleyDB is installed with the system (Linux)
            self.languages = ["java"]
            self.location = berkeleydbjar.get(str(platform), None)

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
    def __init__(self, platform):
        global bzip2
        ThirdParty.__init__(self, platform, "Bzip2", bzip2, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "*"))

class Expat(ThirdParty):
    def __init__(self, platform):
        global expat
        ThirdParty.__init__(self, platform, "Expat", expat, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        return platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libexpat*"))

class OpenSSL(ThirdParty):
    def __init__(self, platform):
        global openssl
        ThirdParty.__init__(self, platform, "OpenSSL", openssl, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
        files = [ os.path.join(bindir, "openssl") ]
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libssl*"))
        files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "libcrypto*"))
        return files

class Mcpp(ThirdParty):
    def __init__(self, platform):
        global mcpp
        ThirdParty.__init__(self, platform, "Mcpp", mcpp, ["cpp"])

class Qt(ThirdParty):
    def __init__(self, platform):
        global qt
        ThirdParty.__init__(self, platform, "Qt", qt, ["cpp"])

    def getFilesFromSubDirs(self, platform, bindir, libdir, x64):
	files = platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "QtCore*"))
	files += platform.getSharedLibraryFiles(self.location, os.path.join(libdir, "QtSql*"))
        return files

class JGoodiesLooks(ThirdParty):
    def __init__(self, platform):
        global jgoodies_looks
        ThirdParty.__init__(self, platform, "JGoodiesLooks", jgoodies_looks, ["java"])

class JGoodiesForms(ThirdParty):
    def __init__(self, platform):
        global jgoodies_forms
        ThirdParty.__init__(self, platform, "JGoodiesForms", jgoodies_forms, ["java"])

class Proguard(ThirdParty):
    def __init__(self, platform):
        global proguard
        ThirdParty.__init__(self, platform, "Proguard", proguard, ["java"])


platform = None
def getPlatform(thirdParties):

    global platform
    if not platform:
        (sysname, nodename, release, ver, machine) = os.uname();
        if not languages.has_key(sysname):
            print sys.argv[0] + ": error: `" + sysname + "' is not a supported system"
        platform = eval(sysname.replace("-", ""))(sysname, machine, languages[sysname])
        for t in thirdParties:
            eval(t)(platform)
    return platform
