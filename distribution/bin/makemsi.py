#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, zipfile, time, stat

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "lib")))

#
# Files from debug builds that are not included in the installers.
#
debugFilterFiles = ["dumpdb.exe",
                    "glacier2router.exe",
                    "iceboxadmin.exe",
                    "icegridadmin.exe",
                    "icegridnode.exe",
                    "icegridregistry.exe",
                    "icepatch2calc.exe",
                    "icepatch2client.exe",
                    "icepatch2server.exe",
                    "iceserviceinstall.exe",
                    "icestormadmin.exe",
                    "icestormmigrate.exe",
                    "slice2cpp.exe",
                    "slice2cs.exe",
                    "slice2freeze.exe",
                    "slice2freezej.exe",
                    "slice2html.exe",
                    "slice2java.exe",
                    "slice2js.exe",
                    "slice2php.exe",
                    "slice2py.exe",
                    "slice2rb.exe",
                    "transformdb.exe",
                    "dumpdb.pdb",
                    "glacier2router.pdb",
                    "iceboxadmin.pdb",
                    "icegridadmin.pdb",
                    "icegridnode.pdb",
                    "icegridregistry.pdb",
                    "icepatch2calc.pdb",
                    "icepatch2client.pdb",
                    "icepatch2server.pdb",
                    "iceserviceinstall.pdb",
                    "icestormadmin.pdb",
                    "icestormmigrate.pdb",
                    "slice2cpp.pdb",
                    "slice2cs.pdb",
                    "slice2freeze.pdb",
                    "slice2freezej.pdb",
                    "slice2html.pdb",
                    "slice2java.pdb",
                    "slice2js.pdb",
                    "slice2php.pdb",
                    "slice2py.pdb",
                    "slice2rb.pdb",
                    "transformdb.pdb"]

#
# That files are only included for the main bin directory
# with correspond with (VS2013 x86  dist)
#
mainDistOnly = ["slice2cpp.exe",
                "slice2cs.exe",
                "slice2freeze.exe",
                "slice2freezej.exe",
                "slice2html.exe",
                "slice2java.exe",
                "slice2js.exe",
                "slice2php.exe",
                "slice2py.exe",
                "slice2rb.exe",
                "slice2cpp.pdb",
                "slice2cs.pdb",
                "slice2freeze.pdb",
                "slice2freezej.pdb",
                "slice2html.pdb",
                "slice2java.pdb",
                "slice2js.pdb",
                "slice2php.pdb",
                "slice2py.pdb",
                "slice2rb.pdb"]

def filterDebugFiles(f):
    if f in debugFilterFiles:
        return True
    if os.path.splitext(f)[1] in [".exe", ".dll", ".pdb", ".lib"]:
        return False
    return True


from BuildUtils import *
from DistUtils import *

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

#signCommand = "signtool sign /f \"%s\" /p %s /t http://timestamp.verisign.com/scripts/timstamp.dll %s"

global signTool
global certFile
global certPassword

global jarKeystore
global jarKeystorePassword

def sign(f, name = None):
    command = [signTool,
               "sign",
               "/f" , certFile,
               "/p", certPassword,
               "/t", "http://timestamp.verisign.com/scripts/timstamp.dll"]
    if name != None:
        command += ["/d", name]
    command += [f]

    if subprocess.check_call(command) != 0:
        return False
    return True

def signJar(filepath):
    command = ("\"%(java_home)s\\bin\\jarsigner\" -keystore %(keystore)s -storepass %(password)s %(filepath)s " +
               "zeroc.com -tsa http://timestamp.digicert.com")
    runCommand(command % {"java_home": getJavaHome("x86", "1.7"),
                          "keystore": jarKeystore,
                          "password": jarKeystorePassword,
                          "filepath": filepath}, True)

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

def overwriteFile(filename, data):
    f = open(filename, "w")
    f.write(data)
    f.close()

def executeCommand(command, env, verbose = True):
    if verbose:
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

def copyIfModified(source, target, verbose, signFile = True):
    if not os.path.exists(target) or os.path.getmtime(source) > os.path.getmtime(target):
        copy(source, target, verbose = verbose)
        if signFile and (target.endswith(".exe") or target.endswith(".dll") or target.endswith(".so")):
            if not sign(target):
                os.remove(target)
                sys.exit(1)
        return True
    return False
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
    print(r"                              is C:\php-5.6.6")
    print("")
    print(r"  --php-bin-home=<path>       PHP binaries location, default location")
    print(r"                              is C:\Program Files (x86)\PHP")
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
    print(r"  --key-file=<path>           Key file used to sign the .NET Assemblies")
    print("")

iceVersion = "3.6.0"
verbose = False

args = None
opts = None

phpHome = None
phpBinHome = None
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

signTool = None
certFile = None
certPassword = None
keyFile = None
jarKeystore = None
jarKeystorePassword = None

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "verbose", "php-home=", "php-bin-home=",
                                                  "skip-build", "skip-installer", "filter-languages=",
                                                  "filter-compilers=", "filter-archs=","filter-confs=",
                                                  "filter-profiles=", "filter-languages=", "rfilter-compilers=",
                                                  "rfilter-archs=", "rfilter-confs=", "rfilter-profiles=", "sign-tool=",
                                                  "cert-file=", "cert-password=", "key-file=", "jar-keystore=",
                                                  "jar-keystore-password="])
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
    elif o == "--php-home":
        phpHome = a
    elif o == "--php-bin-home":
        phpBinHome = a
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
    elif o == "--sign-tool":
        signTool = a
    elif o == "--cert-file":
        certFile = a
    elif o == "--cert-password":
        certPassword = a
    elif o == "--key-file":
        keyFile = a
    elif o == "--jar-keystore":
        jarKeystore = a
    elif o == "--jar-keystore-password":
        jarKeystorePassword = a

basePath = os.path.abspath(os.path.dirname(__file__))
iceBuildHome = os.path.abspath(os.path.join(basePath, "..", ".."))
sourceArchive = os.path.join(iceBuildHome, "Ice-%s.zip" % iceVersion)
demoArchive = os.path.join(iceBuildHome, "Ice-%s-demos.zip" % iceVersion)

distFiles = os.path.join(iceBuildHome, "distfiles-%s" % iceVersion)

iceInstallerFile = os.path.join(distFiles, "src", "windows" , "Ice.aip")

thirdPartyHome = getThirdpartyHome(iceVersion)
if thirdPartyHome is None:
    print("Cannot detect Ice %s ThirdParty installation" % iceVersion)
    sys.exit(1)

if not signTool:
    signToolDefaultPath = "c:\\Program Files (x86)\\Microsoft SDKs\Windows\\v7.1A\Bin\\signtool.exe"
    if os.path.exists(signToolDefaultPath):
        signTool = signToolDefaultPath
else:
    if not os.path.isabs(signTool):
        signTool = os.path.abspath(os.path.join(os.getcwd(), signTool))

if signTool is None:
    print("You need to specify the signtool path using --sign-tool option")
    sys.exit(1)

if not os.path.exists(signTool):
    print("signtool `%s' not found")
    sys.exit(1)

if not certFile:
    if os.path.exists("c:\\release\\authenticode\\zeroc2014.pfx"):
        certFile = "c:\\release\\authenticode\\zeroc2014.pfx"
    elif os.path.exists(os.path.join(os.getcwd(), "..", "..", "release", "authenticode", "zeroc2014.pfx")):
        certFile = os.path.join(os.getcwd(), "..", "..", "release", "authenticode", "zeroc2014.pfx")
else:
    if not os.path.isabs(certFile):
        certFile = os.path.abspath(os.path.join(os.getcwd(), certFile))

if certFile is None:
    print("You need to specify the sign certificate using --cert-file option")
    sys.exit(1)

if not os.path.exists(certFile):
    print("Certificate `%s' not found")
    sys.exit(1)

if certPassword is None:
    print("You need to set the sign certificate password using --cert-password option")
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

if not jarKeystore:
    if os.path.exists("c:\\release\\jarsigner\\keystore.jks"):
        jarKeystore = "c:\\release\\jarsigner\\keystore.jks"
    elif os.path.exists(os.path.join(os.getcwd(), "..", "..", "release", "jarsigner", "keystore.jks")):
        jarKeystore = os.path.join(os.getcwd(), "..", "..", "release", "jarsigner", "keystore.jks")
else:
    if not os.path.isabs(jarKeystore):
        jarKeystore = os.path.abspath(os.path.join(os.getcwd(), jarKeystore))

if jarKeystore is None:
    print("You need to specify the JAR keystore using --jar-keystore option")
    sys.exit(1)

if not os.path.exists(jarKeystore):
    print("Keystore `%s' not found")
    sys.exit(1)

if jarKeystorePassword is None:
    print("You need to set the JAR keystore password using --jar-keystore-password option")
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
global builds
global buildCompilers

buildCompilers = ["VC110", "VC120"]
builds = {
    "VC110": {
        "x86": {
            "release": ["cpp", "php", "vsaddin"],
            "debug": ["cpp"]},
        "amd64": {
            "release": ["cpp"],
            "debug": ["cpp"]}},
    "VC120": {
        "x86": {
            "release": ["cpp", "java", "cs", "vsaddin"],
            "debug": ["cpp"]},
        "amd64": {
            "release": ["cpp"],
            "debug": ["cpp"]},
        "arm": {
            "release": ["cpp"],
            "debug": ["cpp"],}}}

if not skipBuild:

    for compiler in buildCompilers:

        if filterCompilers and compiler not in filterCompilers:
            continue

        if rFilterCompilers and compiler in rFilterCompilers:
            continue

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

                sourceDir = os.path.join(buildDir, "Ice-%s-src" % iceVersion)
                installDir = os.path.join(buildDir, "Ice-%s" % iceVersion)
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

                    if lang == "java":
                        javaHome = getJavaHome(arch, "1.7")

                        if javaHome is None:
                            #
                            # Java 1.7 installation not detected
                            #
                            print("Java 1.7 for arch %s not found" % arch)
                            sys.exit(1)
                        env["JAVA_HOME"] = javaHome

                    if lang == "php":
                        if phpHome is None:
                            if not os.path.exists(r"C:\php-5.6.6"):
                                print("PHP source distribution not found")
                                sys.exit(1)
                            phpHome = r"C:\php-5.6.6"

                        if phpBinHome is None:
                            if not os.path.exists(r"C:\Program Files (x86)\PHP"):
                                print("PHP bin distribution not found")
                                sys.exit(1)
                            phpBinHome = r"C:\Program Files (x86)\PHP"

                        env["PHP_HOME"] = phpHome
                        env["PHP_BIN_HOME"] = phpBinHome

                    if lang == "vsaddin":
                        env["DISABLE_SYSTEM_INSTALL"] = "yes"
                        if compiler == "VC110":
                            env["VS"] = "VS2012"
                        elif compiler == "VC120":
                            env["VS"] = "VS2013"

                    #
                    # Uset the release key to sign .NET assemblies.
                    #
                    if lang == "cs":
                        env["KEYFILE"] = keyFile

                    os.chdir(os.path.join(sourceDir, lang))

                    command = "\"%s\" %s  && nmake /f Makefile.mak install prefix=\"%s\"" % (vcvars, arch, installDir)

                    if lang not in ["java"]:
                        rules = "Make.rules.mak"
                        if lang == "cs":
                            rules += ".cs"
                        elif lang == "php":
                            rules += ".php"

                        setMakefileOption(os.path.join(sourceDir, lang, "config", rules), "prefix", installDir)

                    if lang == "cpp" and compiler in ["VC120"]:
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

                    elif lang == "cs":
                        for profile in [".NET", "SILVERLIGHT"]:

                            if filterProfiles and profile not in filterProfiles:
                                continue

                            if rFilterProfiles and profile in rFilterProfiles:
                                continue

                            if profile == ".NET" and compiler == "VC120":
                                executeCommand(command, env)
                            elif profile == "SILVERLIGHT" and compiler == "VC120":
                                newEnv = env.copy()
                                newEnv["SILVERLIGHT"] = "yes"
                                executeCommand(command, newEnv)
                    else:
                        executeCommand(command, env)

#
# Filter files, list of files that must not be included.
#
filterFiles = ["slice36d.dll", "slice36d.pdb", "sliced.lib", "slice2js.exe", "slice2py.exe", "slice2rb.exe"]

if not os.path.exists(os.path.join(iceBuildHome, "installer")):
    os.makedirs(os.path.join(iceBuildHome, "installer"))

os.chdir(os.path.join(iceBuildHome, "installer"))

installerDir = os.path.join(iceBuildHome, "installer", "Ice-%s" % iceVersion)
installerSrcDir = os.path.join(iceBuildHome, "installer", "Ice-%s-src" % iceVersion)
installerDemoDir = os.path.join(iceBuildHome, "installer", "Ice-%s-demos" % iceVersion)
sdkInstallerDir = os.path.join(iceBuildHome, "installer", "sdk", "Ice-%s" % iceVersion)

if not os.path.exists(installerSrcDir):
    sys.stdout.write("extracting %s to %s... " % (os.path.basename(sourceArchive), installerSrcDir))
    sys.stdout.flush()
    zipfile.ZipFile(sourceArchive).extractall()
    shutil.move(installerDir, installerSrcDir)
    print("ok")

if not os.path.exists(installerDemoDir):
    sys.stdout.write("extracting %s to %s... " % (os.path.basename(demoArchive), installerDemoDir))
    sys.stdout.flush()
    zipfile.ZipFile(demoArchive).extractall()
    print("ok")

if not os.path.exists(os.path.join(iceBuildHome, "installer", "sdk")):
    os.makedirs(os.path.join(iceBuildHome, "installer", "sdk"))

for d in [installerDir, sdkInstallerDir]:
    if not os.path.exists(d):
        os.makedirs(d)

for arch in ["x86", "amd64", "arm"]:
    for compiler in ["VC120"]:
        for conf in ["release", "debug"]:

            buildDir = os.path.join(iceBuildHome, "build-%s-%s-%s" % (arch, compiler, conf))
            sourceDir = os.path.join(buildDir, "Ice-%s-src" % iceVersion)
            installDir = os.path.join(buildDir, "Ice-%s" % iceVersion)

            for root, dirnames, filenames in os.walk(os.path.join(installDir, "SDKs")):
                for f in filenames:
                    if f in filterFiles:
                        continue

                    #
                    # Only copy libraries and PDBs from non main build, the rest of the files are the same
                    # for all builds so we just use the ones from the main build.
                    #
                    if os.path.splitext(f)[1] in [".lib", ".pdb"] or (arch == "x86" and conf == "release"):
                        targetFile = relPath(installDir, sdkInstallerDir, os.path.join(root, f))
                        copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)


for arch in ["x86", "amd64"]:
    for compiler in ["VC110", "VC120"]:
        for conf in ["release", "debug"]:

            buildDir = os.path.join(iceBuildHome, "build-%s-%s-%s" % (arch, compiler, conf))
            sourceDir = os.path.join(buildDir, "Ice-%s-src" % iceVersion)
            installDir = os.path.join(buildDir, "Ice-%s" % iceVersion)

            if compiler == "VC120" and arch == "x86" and conf == "release":
                for d in ["Assemblies", "bin", "config", "include", "lib", "slice", "vsaddin"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            #
                            # IceGridGUI.jar in binary distribution should go in the bin directory.
                            #
                            if f == "icegridgui.jar":
                                targetFile = targetFile.replace(os.path.join(installerDir, "lib"), os.path.join(installerDir, "bin"))

                            if copyIfModified(os.path.join(root, f), targetFile, verbose = verbose):
                                if f == "icegridgui.jar":
                                    #
                                    # Sign icegridgui.jar
                                    #
                                    signJar(targetFile)

                for f in ["CHANGES.txt", "LICENSE.txt", "ICE_LICENSE.txt"]:
                    copyIfModified(os.path.join(sourceDir, f), os.path.join(installerDir, f), verbose = verbose)

                #
                # Copy add-in icon from source dist
                #
                copyIfModified(os.path.join(sourceDir, "vsaddin", "icon", "newslice.ico"),
                                os.path.join(installerDir, "icon", "newslice.ico"), verbose = verbose)

            if compiler == "VC120" and arch == "x86" and conf == "debug":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles or filterDebugFiles(f) or f in mainDistOnly:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC120" and arch == "amd64" and conf == "release":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles or f in mainDistOnly:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)

            if compiler == "VC120" and arch == "amd64" and conf == "debug":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles or filterDebugFiles(f) or f in mainDistOnly:
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)

            #
            # VC110 x86 binaries and libaries
            #
            if compiler == "VC110" and arch == "x86":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                        for f in filenames:
                            if f in filterFiles or f in mainDistOnly:
                                continue
                            if conf == "debug" and filterDebugFiles(f):
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            targetFile = os.path.join(os.path.dirname(targetFile), "vc110",
                                                        os.path.basename(targetFile))
                            copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)
                #
                # VC110 php & vsaddin
                #
                if conf == "release":
                    for d in ["php", "vsaddin"]:
                        for root, dirnames, filenames in os.walk(os.path.join(installDir, d)):
                            for f in filenames:
                                if f in filterFiles or f in mainDistOnly:
                                    continue
                                targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                                copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)
            #
            # VC110 amd64 binaries and libaries
            #
            if compiler == "VC110" and arch == "amd64":
                for d in ["bin", "lib"]:
                    for root, dirnames, filenames in os.walk(os.path.join(installDir, d, "x64")):
                        for f in filenames:
                            if f in filterFiles or f in mainDistOnly:
                                continue
                            if conf == "debug" and filterDebugFiles(f):
                                continue
                            targetFile = relPath(installDir, installerDir, os.path.join(root, f))
                            targetFile = os.path.join(os.path.dirname(os.path.dirname(targetFile)), "vc110", "x64", \
                                                        os.path.basename(targetFile))
                            copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)

#
# docs dir
#
docsDir = os.path.join(distFiles, "src", "windows", "docs", "main")
for f in ["README.txt", "THIRD_PARTY_LICENSE.txt"]:
    copyIfModified(os.path.join(docsDir, f), os.path.join(installerDir, f), verbose = verbose)

#
# Copy thirdpary files
#
for root, dirnames, filenames in os.walk(thirdPartyHome):
    for f in filenames:
        if f in filterFiles:
            continue
        targetFile = relPath(thirdPartyHome, installerDir, os.path.join(root, f))

        #
        # exclude mingw and vc100 files
        #
        if (f.endswith("_vc100.dll") or
            f.endswith("_vc100.pdb") or
            f.endswith("_mingw.dll") or
            targetFile.find("vc100/") or
            targetFile.find("mingw/")):
            continue
        if os.path.splitext(f)[1] in [".exe", ".dll", ".jar", ".pdb"]:
            copyIfModified(os.path.join(root, f), targetFile, verbose = verbose)

copyIfModified(os.path.join(thirdPartyHome, "config", "openssl.cnf"),
                os.path.join(iceBuildHome, "installer", "openssl.cnf"), verbose = verbose)

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

    sign(os.path.join(os.path.dirname(iceInstallerFile), ("Ice-%s.exe" % iceVersion)), "Ice %s" % iceVersion)
    sign(os.path.join(os.path.dirname(iceInstallerFile), ("Ice-%s.msi" % iceVersion)), "Ice %s" % iceVersion)
    sign(os.path.join(os.path.dirname(iceInstallerFile), ("Ice-%s-WebInstaller.exe" % iceVersion)), "Ice %s Web Installer" % iceVersion)

    remove(tmpCertFile)
