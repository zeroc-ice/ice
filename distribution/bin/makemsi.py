#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, zipfile, time, stat

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "lib")))


from BuildUtils import *
from DistUtils import *

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)


def _handle_error(fn, path, excinfo):  
    print("error removing %s" % path)
    os.chmod(path, stat.S_IWRITE)
    fn(path)

def setMakefileOption(filename, optionName, value):
    optre = re.compile("^\#?\s*?%s\s*?=.*" % optionName)
    if os.path.exists(filename + ".tmp"):
        os.remove(filename + ".tmp")
    new = open(filename + ".tmp", "w")
    old = open(filename, "r")
    for line in old:
        if optre.search(line):
            new.write("%s = %s\n" % (optionName, value))
        else:
            new.write(line)
    old.close()
    new.close()
    shutil.move(filename + ".tmp", filename)

def executeCommand(command, env):
    print(command)
    p = subprocess.Popen(command, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE, \
                         stderr = subprocess.STDOUT, bufsize = 0, env = env)

    if p:
        while(True):
            c = p.stdout.read(1)
            
            if not c:
                if p.poll() is not None:
                    break
                time.sleep(0.1)
                continue

            if type(c) != str:
                c = c.decode()
            
            sys.stdout.write(c)
        
        if p.poll() != 0:
            #
            # Command failed
            #
            print("Command failed exit status %s" % p.poll())
            sys.exit(1)

def relPath(sourceDir, targetDir, f):
    sourceDir = os.path.normpath(sourceDir)
    targetDir = os.path.normpath(targetDir)
    f = os.path.normpath(f)
    if f.find(sourceDir) == 0:
        f =  os.path.join(targetDir, f[len(sourceDir) + 1:])
    return f
#
# Program usage.
#
def usage():
    print("")
    print(r"Options:")
    print("")
    print(r"  --help                      Show this message.")
    print("")
    print(r"  --verbose                   Be verbose.")
    print("")
    print(r"  --proguard-home=<path>      Proguard location, default location")
    print(r"                              is C:\proguard")
    print("")
    print(r"  --php-home=<path>           PHP source location, default location")
    print(r"                              is C:\php-5.4.20")
    print("")
    print(r"  --php-bin-home=<path>       PHP binaries location, default location")
    print(r"                              is C:\Program Files\PHP")
    print("")
    print(r"  --ruby-home                 Ruby location, default location is")
    print(r"                              C:\Ruby193")
    print("")
    print(r"  --ruby-devkit-home          Ruby DevKit location, default location is")
    print(r"                              C:\RubyDevKit-4.5.2")
    print("")
    print(r"  --skip-build                Skip build and go directly to installer creation,")
    print(r"                              existing build will be used")
    print("")
    print(r"  --skip-installer            Skip the installer creation, just do the build")
    print("")
    print(r"  --filter-languages=<name>   Just build and run the given languages")
    print("")
    print(r"  --filter-compilers=<name>   Just build the given compilers")
    print("")
    print(r"  --filter-archs=<name>       Just build the given architectures")
    print("")
    print(r"  --filter-confs=<name>       Just build the given configurations")
    print("")
    print(r"  --filter-profiles=<name>    Just build the given profiles")
    print("")
    print(r"  --cert-file=<path>          Certificate file used to sign the installer")
    print("")
    print(r"makemsi.py --verbose")
    print("")

version = "3.5.1"
verbose = False

args = None
opts = None

proguardHome = None
phpHome = None
phpBinHome = None
rubyHome = None
rubyDevKitHome = None
skipBuild = False
skipInstaller = False

filterLanguages = []
filterCompilers = []
filterArchs = []
filterConfs = []
filterProfiles = []

rFilterLanguages = []
rFilterCompilers = []
rFilterArchs = []
rFilterConfs = []
rFilterProfiles = []

certFile = None
keyFile = None

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "verbose", "proguard-home=", "php-home=", "php-bin-home=", \
                                                  "ruby-home=", "ruby-devkit-home=", "skip-build", "skip-installer", \
                                                  "filter-languages=", "filter-compilers=", "filter-archs=", \
                                                  "filter-confs=", "filter-profiles=", "filter-languages=", \
                                                  "filter-compilers=", "filter-archs=", "filter-confs=", \
                                                  "filter-profiles=", "cert-file=", "key-file="])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

if args:
    usage()
    sys.exit(1)

for o, a in opts:
    if o == "--help":
        usage()
        sys.exit(0)
    elif o == "--verbose":
        verbose = True
    elif o == "--proguard-home":
        proguardHome = a
    elif o == "--php-home":
        phpHome = a
    elif o == "--php-bin-home":
        phpBinHome = a
    elif o == "--ruby-home":
        rubyHome = a
    elif o == "--ruby-devkit-home":
        rubyDevKitHome = a
    elif o == "--skip-build":
        skipBuild = True
    elif o == "--skip-installer":
        skipInstaller = True
    elif o == "--filter-languages":
        filterLanguages.append(a)
    elif o == "--filter-compilers":
        filterCompilers.append(a)
    elif o == "--filter-archs":
        filterArchs.append(a)
    elif o == "--filter-confs":
        filterConfs.append(a)
    elif o == "--filter-profiles":
        filterProfiles.append(a)
    elif o == "--rfilter-languages":
        rFilterLanguages.append(a)
    elif o == "--rfilter-compilers":
        rFilterCompilers.append(a)
    elif o == "--rfilter-archs":
        rFilterArchs.append(a)
    elif o == "--rfilter-confs":
        rFilterConfs.append(a)
    elif o == "--rfilter-profiles":
        rFilterProfiles.append(a)
    elif o == "--cert-file":
        certFile = a
    elif o == "--key-file":
        keyFile = a

basePath = os.path.abspath(os.path.dirname(__file__))
iceBuildHome = os.path.abspath(os.path.join(basePath, "..", ".."))
sourceArchive = os.path.join(iceBuildHome, "Ice-%s.zip" % version)
demoArchive = os.path.join(iceBuildHome, "Ice-%s-demos.zip" % version)

distFiles = os.path.join(iceBuildHome, "distfiles-%s" % version)

iceInstallerFile = os.path.join(distFiles, "src", "windows" , "Ice.aip")
pdbsInstallerFile = os.path.join(distFiles, "src", "windows" , "PDBs.aip")

thirdPartyHome = getThirdpartyHome(version)
if thirdPartyHome is None:
    print("Cannot detect Ice %s ThirdParty installation" % version)
    sys.exit(1)

if not certFile:
    if os.path.exists("c:\\release\\authenticode\\zeroc2013.pfx"):
        certFile = "c:\\release\\authenticode\\zeroc2013.pfx"
    elif os.path.exists(os.path.join(os.getcwd(), "..", "..", "release", "authenticode", "zeroc2013.pfx")):
        certFile = os.path.join(os.getcwd(), "..", "..", "release", "authenticode", "zeroc2013.pfx")
else:
    if not os.path.isabs(certFile):
        certFile = os.path.abspath(os.path.join(os.getcwd(), certFile))
        
if certFile is None:
    print("You need to specify the sign certificate using --cert-file option")
    sys.exit(1)

if not os.path.exists(certFile):
    print("Certificate `%s' not found")
    sys.exit(1)
    
    
if not keyFile:
    if os.path.exists("c:\\release\\strongname\\IceReleaseKey.snk"):
        keyFile = "c:\\release\\strongname\\IceReleaseKey.snk"
    elif os.path.exists(os.path.join(os.getcwd(), "..", "..", "release", "strongname", "IceReleaseKey.snk")):
        keyFile = os.path.join(os.getcwd(), "..", "..", "release", "strongname", "IceReleaseKey.snk")
else:
    if not os.path.isabs(keyFile):
        keyFile = os.path.abspath(os.path.join(os.getcwd(), keyFile))
        
if keyFile is None:
    print("You need to specify the key file to sign assemblies using --key-file option")
    sys.exit(1)

if not os.path.exists(keyFile):
    print("Key file `%s' not found")
    sys.exit(1)

if proguardHome:
    if not os.path.isabs(proguardHome):
        proguardHome = os.path.abspath(os.path.join(os.getcwd(), proguardHome))

    if not os.path.exists(proguardHome):
        #
        # Invalid proguard-home setting
        #
        print("--proguard-home points to nonexistent directory")
        sys.exit(1)

if phpHome:
    if not os.path.isabs(phpHome):
        phpHome = os.path.abspath(os.path.join(os.getcwd(), phpHome))

    if not os.path.exists(phpHome):
        #
        # Invalid proguard-home setting
        #
        print("--php-home points to nonexistent directory")
        sys.exit(1)

if phpBinHome:
    if not os.path.isabs(phpBinHome):
        phpBinHome = os.path.abspath(os.path.join(os.getcwd(), phpBinHome))

    if not os.path.exists(phpBinHome):
        #
        # Invalid proguard-home setting
        #
        print("--php-bin-home points to nonexistent directory")
        sys.exit(1)

if not os.path.exists(sourceArchive):
    print("Couldn't find %s in %s" % (os.path.basename(sourceArchive), os.path.dirname(sourceArchive)))
    sys.exit(1)

if not os.path.exists(demoArchive):
    print("Couldn't find %s in %s" % (os.path.basename(demoArchive), os.path.dirname(demoArchive)))
    sys.exit(1)

    
#
# Windows build configurations by Compiler Arch 
#
builds = {
    "MINGW": {
        "x86": {
            "release": ["cpp", "rb"]}},
    "VC90": {
        "x86": {
            "release": ["cpp", "cs", "php", "vsaddin"]}},
    "VC100": {
        "x86": {
            "release": ["cpp", "cs", "java", "py", "vsaddin"], 
            "debug": ["cpp"]},
        "amd64": {
            "release": ["cpp", "py"], 
            "debug": ["cpp"]}},
    "VC110": {
        "x86": {
            "release": ["cpp", "vsaddin"], 
            "debug": ["cpp"]},
        "amd64": {
            "release": ["cpp"], 
            "debug": ["cpp"]},
        "arm": {
            "release": ["cpp"], 
            "debug": ["cpp"]}},
    "VC120": {
        "x86": {
            "release": ["cpp", "vsaddin"], 
            "debug": ["cpp"]},
        "amd64": {
            "release": ["cpp"], 
            "debug": ["cpp"]},
        "arm": {
            "release": ["cpp"], 
            "debug": ["cpp"]}}}
            
if not skipBuild:
    
    for compiler in ["MINGW", "VC90", "VC100", "VC110", "VC120"]:

        if filterCompilers and compiler not in filterCompilers:
            continue

        if rFilterCompilers and compiler in rFilterCompilers:
            continue
        
        if compiler not in ["MINGW"]:
            vcvars = getVcVarsAll(compiler)

            if vcvars is None:
                print("Compiler %s not found" % compiler)
                sys.exit(1)
    
        for arch in ["x86", "amd64", "arm"]:
            
            if not arch in builds[compiler]:
                continue
            
            if filterArchs and arch not in filterArchs:
                continue

            if rFilterArchs and arch in rFilterArchs:
                continue

            for conf in ["release", "debug"]:
                
                if not conf in builds[compiler][arch]:
                    continue
        
                if filterConfs and conf not in filterConfs:
                    continue

                if rFilterConfs and conf in rFilterConfs:
                    continue

                buildDir = os.path.join(iceBuildHome, "build-%s-%s-%s" % (arch, compiler, conf))

                if not os.path.exists(buildDir):
                    os.makedirs(buildDir)

                os.chdir(buildDir)

                sourceDir = os.path.join(buildDir, "Ice-%s-src" % version)
                installDir = os.path.join(buildDir, "Ice-%s" % version)
                if not os.path.exists(sourceDir):
                    sys.stdout.write("extracting %s to %s... " % (os.path.basename(sourceArchive), sourceDir))
                    sys.stdout.flush()
                    zipfile.ZipFile(sourceArchive).extractall()
                    if os.path.exists(sourceDir):
                        shutil.rmtree(sourceDir, onerror = _handle_error)
                    shutil.move(installDir, sourceDir)
                    print("ok")

                print ("Build: (%s/%s/%s)" % (compiler,arch,conf))
                for lang in builds[compiler][arch][conf]:

                    if filterLanguages and lang not in filterLanguages:
                        continue

                    if rFilterLanguages and lang in rFilterLanguages:
                        continue

                    env = os.environ.copy()

                    env["THIRDPARTY_HOME"] = thirdPartyHome
                    env["RELEASEPDBS"] = "yes"
                    if conf == "release":
                        env["OPTIMIZE"] = "yes"

                    if lang == "py":
                        pythonHome = getPythonHome(arch)
                        if pythonHome is None:
                            #
                            # Python installation not detected
                            #
                            print("Python 3.3 for arch %s not found" % arch)
                            sys.exit(1)
                        env["PYTHON_HOME"] = pythonHome

                    if lang == "java":
                        javaHome = getJavaHome(arch, "1.7")

                        if javaHome is None:
                            #
                            # Java 1.7 installation not detected
                            #
                            print("Java 1.7 for arch %s not found" % arch)
                            sys.exit(1)
                        env["JAVA_HOME"] = javaHome

                        if proguardHome is None:
                            #
                            # Proguard installation not detected
                            #
                            if not os.path.exists(r"C:\proguard"):
                                print("Proguard not found")
                                sys.exit(1)
                            proguardHome = r"C:\proguard"
                        #
                        # We override CLASSPATH, we just need proguard in classpath to build Ice.
                        #
                        env["CLASSPATH"] = os.path.join(proguardHome, "lib", "proguard.jar")

                    if lang == "php":
                        if phpHome is None:
                            if not os.path.exists(r"C:\php-5.4.20"):
                                print("PHP source distribution not found")
                                sys.exit(1)
                            phpHome = r"C:\php-5.4.20"

                        if phpBinHome is None:
                            if not os.path.exists(r"C:\Program Files (x86)\PHP"):
                                print("PHP bin distribution not found")
                                sys.exit(1)
                            phpBinHome = r"C:\Program Files (x86)\PHP"

                        env["PHP_HOME"] = phpHome
                        env["PHP_BIN_HOME"] = phpBinHome

                    if compiler == "MINGW":
                        if rubyDevKitHome is None:
                            if not os.path.exists(r"C:\RubyDevKit-4.5.2"):
                                print("Ruby DevKit not found")
                                sys.exit(1)
                            rubyDevKitHome = r"C:\RubyDevKit-4.5.2"

                    if lang == "rb":
                        if rubyHome is None:
                            if not os.path.exists(r"C:\Ruby193"):
                                print("Ruby not found")
                                sys.exit(1)
                            rubyHome = r"C:\Ruby193"

                    if lang == "vsaddin":
                        if compiler == "VC90":
                            env["VS"] = "VS2008"
                        elif compiler == "VC100":
                            env["VS"] = "VS2010"
                        elif compiler == "VC110":
                            env["VS"] = "VS2012"
                        elif compiler == "VC120":
                            env["VS"] = "VS2013"
                            
                    #
                    # Uset the release key to sign .NET assemblies.
                    #
                    if lang == "cs":
                        env["KEYFILE"] = keyFile

                    os.chdir(os.path.join(sourceDir, lang))

                    command = None
                    if compiler != "MINGW":
                        command = "\"%s\" %s  && nmake /f Makefile.mak install prefix=\"%s\"" % \
                                  (vcvars, arch, installDir)
                    
                    if lang not in ["java", "rb"]:
                        rules = "Make.rules.mak"
                        if lang == "cs":
                            rules += ".cs"
                        elif lang == "php":
                            rules += ".php"

                        setMakefileOption(os.path.join(sourceDir, lang, "config", rules), "prefix", installDir)

                    if lang == "cpp" and compiler in ["VC110", "VC120"]:
                        for profile in ["DESKTOP", "WINRT"]:

                            if filterProfiles and profile not in filterProfiles:
                                continue

                            if rFilterProfiles and profile in rFilterProfiles:
                                continue

                            if profile == "DESKTOP":
                                if arch == "arm":
                                    command = "\"%s\" %s  && nmake /f Makefile.mak install" % (vcvars, "x86")
                                    executeCommand(command, env)
                                else:
                                    command = "\"%s\" %s  && nmake /f Makefile.mak install" % (vcvars, arch)
                                    executeCommand(command, env)
                            elif profile == "WINRT":
                                if arch == "arm":
                                    command = "\"%s\" %s  && nmake /f Makefile.mak install" % (vcvars, "x86_arm")
                                else:
                                    command = "\"%s\" %s  && nmake /f Makefile.mak install" % (vcvars, arch)
                                newEnv = env.copy()
                                newEnv["WINRT"] = "yes"
                                executeCommand(command, newEnv)

                    elif compiler == "MINGW":
                        prefix = installDir
                        if prefix[1] == ":":
                            prefix = "/%s/%s" % (prefix[0], prefix[2:])
                        prefix = re.sub(re.escape("\\"), "/", prefix) 
                        if lang == "cpp":
                            command = "%s\\devkitvars.bat && make install prefix=\"%s\"" % (rubyDevKitHome, prefix)
                            executeCommand(command, env)
                        elif lang == "rb":
                            command = "%s\\bin\\setrbvars.bat && %s\\devkitvars.bat && make install prefix=\"%s\"" % \
                                      (rubyHome, rubyDevKitHome, prefix)
                            executeCommand(command, env)

                    elif lang == "cs":
                        for profile in [".NET", "SILVERLIGHT", "COMPACT"]:

                            if filterProfiles and profile not in filterProfiles:
                                continue

                            if rFilterProfiles and profile in rFilterProfiles:
                                continue

                            if profile == ".NET" and compiler == "VC100":
                                executeCommand(command, env)
                            elif profile == "SILVERLIGHT" and compiler == "VC100":
                                newEnv = env.copy()
                                newEnv["SILVERLIGHT"] = "yes"
                                executeCommand(command, newEnv)
                            elif profile == "COMPACT" and compiler == "VC90":
                                newEnv = env.copy()
                                newEnv["COMPACT"] = "yes"
                                command = "\"%s\" %s  && nmake /f Makefile.mak install prefix=\"%s\"" % \
                                          (getVcVarsAll("VC90"), arch, installDir)
                                executeCommand(command, newEnv)
                    else:
                        executeCommand(command, env)




#
# Filter files, list of files that must not be included.
#
filterFiles = ["slice35d.dll", "slice35d.pdb", "sliced.lib"]

if not os.path.exists(os.path.join(iceBuildHome, "installer")):
    os.makedirs(os.path.join(iceBuildHome, "installer"))

os.chdir(os.path.join(iceBuildHome, "installer"))

installerDir = os.path.join(iceBuildHome, "installer", "Ice-%s" % version)
installerdSrcDir = os.path.join(iceBuildHome, "installer", "Ice-%s-src" % version)
installerDemoDir = os.path.join(iceBuildHome, "installer", "Ice-%s-demos" % version)

if not os.path.exists(installerdSrcDir):
    sys.stdout.write("extracting %s to %s... " % (os.path.basename(sourceArchive), installerdSrcDir))
    sys.stdout.flush()
    zipfile.ZipFile(sourceArchive).extractall()
    shutil.move(installerDir, installerdSrcDir)
    print("ok")


if not os.path.exists(installerDemoDir):
    sys.stdout.write("extracting %s to %s... " % (os.path.basename(demoArchive), installerDemoDir))
    sys.stdout.flush()
    zipfile.ZipFile(demoArchive).extractall()
    print("ok")


if os.path.exists(installerDir):
    shutil.rmtree(installerDir, onerror = _handle_error)
os.makedirs(installerDir)

for arch in ["x86", "amd64", "arm"]:
    for compiler in ["VC100", "MINGW", "VC90", "VC110", "VC120"]:
        for conf in ["release", "debug"]:

            buildDir = os.path.join(iceBuildHome, "build-%s-%s-%s" % (arch, compiler, conf))
            sourceDir = os.path.join(buildDir, "Ice-%s-src" % version)
            installDir = os.path.join(buildDir, "Ice-%s" % version)

            if compiler == "VC100" and arch == "x86" and conf == "release":
                for d in ["Assemblies", "bin", "config", "include", "lib", "python", "slice", "vsaddin"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            #
                            # IceGridGUI.jar in binary distribution should go in the bin directory.
                            #
                            if f == "IceGridGUI.jar":
                                targetFile = targetFile.replace(os.path.join(installerDir, "lib"), os.path.join(installerDir, "bin"))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile)

                for f in ["CHANGES.txt", "LICENSE.txt", "ICE_LICENSE.txt", "RELEASE_NOTES.txt"]:
                    copy(os.path.join(sourceDir, f), os.path.join(installerDir, f), verbose = verbose)

                #
                # Copy add-in icon from source dist
                #
                copy(os.path.join(sourceDir, "vsaddin", "icon", "newslice.ico"), \
                        os.path.join(installerDir, "icon", "newslice.ico"), verbose = verbose)

            if compiler == "VC100" and arch == "x86" and conf == "debug":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC100" and arch == "amd64" and conf == "release":
                for d in ["bin", "lib", "python"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC100" and arch == "amd64" and conf == "debug":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "MINGW" and arch == "x86" and conf == "release":
                for d in ["ruby", "bin"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC90" and arch == "x86" and conf == "release":
                for d in ["Assemblies", "php", "bin", "vsaddin"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)


            if compiler in ["VC110", "VC120"] and arch == "x86" and conf == "release":
                for d in ["vsaddin"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            #
            # VC110 binaries and libaries
            #
            if compiler == "VC110" and arch == "x86":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            targetFile = os.path.join(os.path.dirname(targetFile), "vc110", \
                                                        os.path.basename(targetFile))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC110" and arch == "amd64":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            targetFile = os.path.join(os.path.dirname(os.path.dirname(targetFile)), "vc110", "x64", \
                                                        os.path.basename(targetFile))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)


                        #
            # VC120 binaries and libaries
            #
            if compiler == "VC120" and arch == "x86":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            targetFile = os.path.join(os.path.dirname(targetFile), "vc120", \
                                                        os.path.basename(targetFile))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC120" and arch == "amd64":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            targetFile = os.path.join(os.path.dirname(os.path.dirname(targetFile)), "vc120", "x64", \
                                                        os.path.basename(targetFile))
                            if not os.path.exists(targetFile):
                                copy(os.path.join(root, f), targetFile, verbose = verbose)


            #
            # WinRT SDKs
            #
            if compiler == "VC110":
                for root, dirnames, filenames in os.walk(os.path.join(installDir, "SDKs", "Ice")):
                    for f in filenames:
                        if f in filterFiles:
                            continue
                        targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                        if not os.path.exists(targetFile):
                            copy(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC120":
                for root, dirnames, filenames in os.walk(os.path.join(installDir, "SDKs", "8.1")):
                    for f in filenames:
                        if f in filterFiles:
                            continue
                        targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                        if not os.path.exists(targetFile):
                            copy(os.path.join(root, f), targetFile, verbose = verbose)

#
# docs dir
#
docsDir = os.path.join(distFiles, "src", "windows", "docs", "main")
for f in ["README.txt", "SOURCES.txt", "THIRD_PARTY_LICENSE.txt"]:
    copy(os.path.join(docsDir, f), os.path.join(installerDir, f), verbose = verbose)

#
# Copy thirdpary files
#
for root, dirnames, filenames in os.walk(thirdPartyHome):
    for f in filenames:
        if f in filterFiles:
            continue
        targetFile = relPath(thirdPartyHome, installerDir, os.path.join(root, f))
        if not os.path.exists(targetFile) and os.path.splitext(f)[1] in [".exe", ".dll", ".jar", ".pdb"]:
            copy(os.path.join(root, f), targetFile, verbose = verbose)

copy(os.path.join(thirdPartyHome, "config", "openssl.cnf"), os.path.join(iceBuildHome, "installer"))

#
# Move PDBs to PDBs installer dir
#
pdbinstallerDir = os.path.join(iceBuildHome, "installer/Ice-%s-PDBs" % version)

if os.path.exists(pdbinstallerDir):
    shutil.rmtree(pdbinstallerDir, onerror = _handle_error)

for root, dirnames, filenames in os.walk(installerDir):
    #
    # Keep WinRT SDK PDBs in the main installer
    #
    if root.startswith(os.path.join(installerDir, "SDKs")):
        continue
    for f in filenames:
        if f in filterFiles:
            continue
        targetFile = relPath(installerDir, pdbinstallerDir, os.path.join(root, f))
        if not os.path.exists(targetFile) and os.path.splitext(f)[1] in [".pdb"]:
            move(os.path.join(root, f), targetFile)

if not skipInstaller:
    #
    # Build installers with Advanced installer.
    #

    #
    # XML with path variables definitions
    #
    pathVariables = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<PathVariables Application="Advanced Installer" Version="10.3">
  <Var Name="ICE_BUILD_HOME" Path="%ICE_BUILD_HOME%" Type="2" ContentType="2"/>
</PathVariables>"""

    advancedInstallerHome = getAdvancedInstallerHome()
    if advancedInstallerHome is None:
        print("Advanced Installer installation not found")
        sys.exit(1)

    advancedInstaller = os.path.join(advancedInstallerHome, "bin", "x86", "AdvancedInstaller.com")

    if not os.path.exists(advancedInstaller):
        print("Advanced Installer executable not found in %s" % advancedInstaller)
        sys.exit(1)

    env = os.environ.copy()    
    env["ICE_BUILD_HOME"] = iceBuildHome

    paths = os.path.join(iceBuildHome, "installer", "paths.xml")
    f = open(os.path.join(iceBuildHome, "installer", "paths.xml"), "w")
    f.write(pathVariables)
    f.close()
    
    tmpCertFile = os.path.join(os.path.dirname(iceInstallerFile), os.path.basename(certFile))
    copy(certFile, tmpCertFile)

    #
    # Load path vars
    #
    command = "\"%s\" /loadpathvars %s" % (advancedInstaller, paths)
    executeCommand(command, env)

    #
    # Build the Ice main installer.
    #    
    command = "\"%s\" /rebuild %s" % (advancedInstaller, iceInstallerFile)
    executeCommand(command, env)

    shutil.move(os.path.join(os.path.dirname(iceInstallerFile), "Ice.msi"), \
                             os.path.join(iceBuildHome, "Ice-%s.msi" % version))

    #
    # Build the Ice PDBs installer.
    #
    command = "\"%s\" /rebuild %s" % (advancedInstaller, pdbsInstallerFile)
    executeCommand(command, env)

    shutil.move(os.path.join(os.path.dirname(iceInstallerFile), "PDBs.msi"), \
                             os.path.join(iceBuildHome, "Ice-PDBs-%s.msi" % version))

    remove(tmpCertFile)
