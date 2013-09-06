#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, zipfile, time, threading, tempfile

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "lib")))

import BuildUtils

version = "3.5.1"

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

output = None
report = None

def trace(msg, f, nl = True):
    if nl:
        print(msg)
    else:
        sys.stdout.write(msg)
        sys.stdout.flush()

    f.write(msg)
    if nl:
        f.write('\n')
    f.flush()

class Platform:
    
    def __init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, languages, \
                 compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, skipTests, \
                 skipDemos, parallelJobs, testConfigurations):
        
        self._iceHome = iceHome
        self._archive = archive
        self._demoArchive = demoArchive
        self._demoScriptsArchive = demoScriptsArchive
        self._buildDir = buildDir
        self._sourceDir = None
        self._demoDir = None
        self._verbose = verbose
        self._debug = debug
        self._languages = languages
        self._compilers = compilers
        self._archs = archs
        self._configurations = configurations
        self._rlanguages = rlanguages
        self._rcompilers = rcompilers
        self._rarchs = rarchs
        self._rconfigurations = rconfigurations
        self._skipTests = skipTests
        self._skipDemos = skipDemos
        self._parallelJobs = parallelJobs
        self._testConfigurations = testConfigurations
        
        #
        # Check paths are valid
        #
        if self._iceHome and not os.path.exists(self._iceHome):
            print("Invalid Ice Home setting `%s'" % self._iceHome)
            sys.exit(1)
        
        if self._archive and not os.path.exists(self._archive):
            print("Invalid Ice source archive `%s'" % self._archive)
            sys.exit(1)
            
        if self._demoArchive and not os.path.exists(self._demoArchive):
            print("Invalid Ice demo archive `%s'" % self._demoArchive)
            sys.exit(1)
        
    def setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, \
                              useBinDist):
        if useBinDist:
            env["USE_BIN_DIST"] = "yes"
        
        if not self._debug:
            env["OPTIMIZE"] = "yes"

        if thirpatyHome:
            env["THIRDPARTY_HOME"] = thirpatyHome
                    
        if iceHome:
            env["ICE_HOME"] = iceHome
        
        if buildConfiguration == "cpp11":
            env["CPP11"] = "yes"
            
        if arch == "amd64" and not self.isWindows():
            env["LP64"] = "yes"
            
        if lang == "java":
            
            if javaHome:            
                env["JAVA_HOME"] = javaHome
            
            dbHome = env.get("DB_HOME", "")
            
            for d in [dbHome, iceHome]:
                if not d:
                    continue
                
                separator = ";" if self.isWindows() else ":"
                
                classPath = env.get("CLASSPATH", "")
                dbClassPath = os.path.join(d, "lib", "db.jar")
                    
                if not os.path.exists(dbClassPath):
                    continue
                
                if classPath == "":
                    env["CLASSPATH"] = dbClassPath
                elif classPath.find(dbClassPath) == -1:
                    env["CLASSPATH"] = dbClassPath + separator + classPath
                
            
    def checkJavaSupport(self, arch, buildConfiguration, output):
        return True
            
    def isWindows(self):
        return False
        
    def isSolaris(self):
        return False
        
    def isDarwin(self):
        return False
        
    def isLinux(self):
        return False
        
    def isRhel(self):
        return False
        
    def isUbuntu(self):
        return False
        
    def isSles(self):
        return False
    
    def getVersion(self):
        return "3.5.1"
        
    def getDemoDir(self, lang):
        if lang == "cpp":
            return "demo"
        elif lang == "cs":
            return "democs"
        elif lang == "java":
            return "demoj"
        elif lang == "php":
            return "demophp"
        elif lang == "py":
            return "demopy"
        elif lang == "rb":
            return "demorb"
        elif lang == "vb":
            return "demovb"

    def getThirdpartyHome(self):
        return None
    
    def getJavaHome(self, arch, buildConfiguration):
        return None
    
    def getLanguageMappings(self, compiler, arch, buildConfiguration):
        if self.isSolaris():
            return ["cpp", "java"]
        if buildConfiguration in ["debug", "cpp11", "winrt"]:
            return ["cpp"]
        elif buildConfiguration == "java1.7":
            return ["java"]
        elif compiler == "VC90" or buildConfiguration == "silverlight":
            return ["cs"]
        else:
            languages = ["cpp"]
            if self.isWindows() or (self.isLinux() and not self.isRhel()):
                languages += ["cs"] 
            languages += ["java", "py"]
            if not self.isDarwin():
                languages += ["php", "rb"]
            if self.isWindows():
                languages += ["vb"]
            return languages

    def getBuildConfigurations(self, compiler):
        return ["default"]

    def makeCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        command = "make"
        if self._parallelJobs:
            command += " -j%s" % self._parallelJobs
        return command
        
    def makeCleanCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "make clean"

    def runScriptCommand(self, script, compiler, arch, buildConfiguration, lang):
        return "%s %s" % (sys.executable, script)
        
    def extractSourceArchive(self, compiler, arch, conf):
        if not os.path.exists(os.path.join(self._buildDir, compiler, arch, conf)):
            os.makedirs(os.path.join(self._buildDir, compiler, arch, conf))
        
        os.chdir(os.path.join(self._buildDir, compiler, arch, conf))

        if self._archive.endswith(".tar.gz"):
            self._sourceDir = os.path.join(self._buildDir, compiler, arch, conf, \
                                           re.sub(".tar.gz", "", os.path.basename(self._archive)))
        elif self._archive.endswith(".zip"):
            self._sourceDir = os.path.join(self._buildDir, compiler, arch, conf, \
                                           re.sub(".zip", "", os.path.basename(self._archive)))
        else:
            trace("Invalid Ice source archive `%s'" % self._archive, report)
            sys.exit(1)
              
        if not os.path.exists(self._sourceDir):
            trace("Extracting %s source archive into %s ... " % \
              (os.path.basename(self._archive), os.path.basename(self._sourceDir)), report, False)
            if self._archive.endswith(".tar.gz"):
                runCommand("tar -zxf %s" %(self._archive), self._verbose)
            elif self._archive.endswith(".zip"):
                zipfile.ZipFile(self._archive).extractall()                
            trace("Ok", report)
        else:
            trace("%s already exists using it" % self._sourceDir, report)
            
    def extractDemoArchive(self, compiler, arch, conf):
        if not os.path.exists(os.path.join(self._buildDir, compiler, arch, conf)):
            os.makedirs(os.path.join(self._buildDir, compiler, arch, conf))
        
        os.chdir(os.path.join(self._buildDir, compiler, arch, conf))

        if self._demoArchive.endswith(".tar.gz"):
            self._demoDir = os.path.join(self._buildDir, compiler, arch, conf, \
                                           re.sub(".tar.gz", "", os.path.basename(self._demoArchive)))
        elif self._demoArchive.endswith(".zip"):
            self._demoDir = os.path.join(self._buildDir, compiler, arch, conf, \
                                           re.sub(".zip", "", os.path.basename(self._demoArchive)))
        else:
            trace("Invalid Ice source archive `%s'" % self._demoArchive, report)
            sys.exit(1)
        
        if not os.path.exists(self._demoDir):
            trace("Extracting %s demo archive into %s ... " % \
                  (os.path.basename(self._demoArchive), os.path.basename(self._demoDir)), report, False)
            if self._demoArchive.endswith(".tar.gz"):
                runCommand("tar -zxf %s" %(self._demoArchive), self._verbose)
                runCommand("tar -zxf %s" %(self._demoScriptsArchive), self._verbose)
            elif self._demoArchive.endswith(".zip"):
                zipfile.ZipFile(self._demoArchive).extractall()
                zipfile.ZipFile(self._demoScriptsArchive).extractall()
            trace("Ok", report)
        else:
            trace("%s already exists using it" % self._demoDir, report)
            
    def buildTestSuite(self, compiler, arch, buildConfiguration, lang):
            
        trace("", report)
        trace("--------------------------------------------------------------------------", report)
        trace("Building tests compiler: %s arch: %s build configuration: %s lang: %s ... " % \
              (compiler, arch, buildConfiguration, lang), report, False)

        if lang == "py" or lang == "rb":
            #
            # Nothing to build for python or ruby tests.
            #
            trace("Ok", report)
            return True
            
        buildDir = None
        command = None

        if lang == "java":
            buildDir = os.path.join(self._sourceDir, lang)
        else:
            buildDir = os.path.join(self._sourceDir, lang, "test")
                
        env = os.environ.copy()
        thirpatyHome = self.getThirdpartyHome()                    
        iceHome = self.getIceHome()

        commands = []

        javaHome = None

        if buildConfiguration == "silverlight":
            commands.append(self.makeSilverlightCommand(compiler, arch, buildConfiguration, lang, buildDir))

        if lang == "java":
            javaHome = self.getJavaHome(arch, buildConfiguration)
            if not self.checkJavaSupport(arch, buildConfiguration, output):
                return False
            commands.append("ant test-jar")
        else:
            commands.append(self.makeCommand(compiler, arch, buildConfiguration, lang, buildDir))

        self.setPlatformEnviroment(env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, True)

        os.chdir(buildDir)

        status = True
        for command in commands:
            if self._verbose:
                print(command)
            
            p = subprocess.Popen(command, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE, \
                                 stderr = subprocess.STDOUT, bufsize = 0, env = env)    
            if p:
                while(True):
                    line = p.stdout.readline()
                    
                    if p.poll() is not None and not line:
                        break
                    if not line:
                        time.sleep(0.1)
                    
                    if type(line) != str:
                        line = line.decode()
                    
                    line = line.strip()
                    print(line)
                    output.write(line)
                    output.write("\n")
                    output.flush()
                
                if p.poll() != 0:
                    trace("Failed", report)
                    status = False
                    trace("--------------------------------------------------------------------------", report)
                    break
        if status:
            trace("Ok", report)
        return status

    def runTestSuite(self, compiler, arch, buildConfiguration, lang):
        os.chdir(os.path.join(self._sourceDir, lang))
        
        env = os.environ.copy()            
        iceHome = self.getIceHome()
        
        for testConf in self._testConfigurations:
            args = "--continue %s" % testConf.options

            if testConf.languages and lang not in testConf.languages:
                trace("Skiping language `%s' not in test configuration: `%s', languages: %s" % (lang, testConf.name, testConf.languages), output)
                continue
            
            if arch == "amd64":
                if not self.isWindows() or os.path.exists(os.path.join(iceHome, "bin", "x64")):
                    args += " --x64"
                    
            if buildConfiguration == "cpp11":
                args += " --c++11"
            
            if buildConfiguration == "debug":
                args += " --debug"

            if lang == "cs" and compiler == "VC90":
                args += " --compact"

            if buildConfiguration == "silverlight":
                args += " --silverlight"
                
            javaHome = None
            if lang == "java":
                javaHome = self.getJavaHome(arch, buildConfiguration)
                if not self.checkJavaSupport(arch, buildConfiguration, output):
                    return False
                    
            thirpatyHome = self.getThirdpartyHome()            
            self.setPlatformEnviroment(env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, True)

            command = "%s %s" % (self.runScriptCommand("allTests.py", compiler, arch, buildConfiguration, lang), args)

            if self._verbose:
                print(command)
                
            errors = False
            resume = []
            total = 0
            failures = 0
            
            
            trace("Running tests, test configuration: %s ..." % (testConf.name), report)
              
            p = subprocess.Popen(command, shell = True, stdout = subprocess.PIPE, \
                                stderr = subprocess.STDOUT, bufsize = 0, env = env)

            while True:
                line = p.stdout.readline()
                
                if p.poll() is not None and not line:
                    break
                if not line:
                    time.sleep(0.1)
                    
                if type(line) != str:
                    line = line.decode()

                line = line.strip()
                print(line)
            
                if line:
                    if errors:
                        resume.append(line)
                    elif line.find("*** running tests") == 0:
                        total = total + 1
                    elif line.find("** Error logged and will be displayed again when suite is completed **") == 0:
                        failures = failures + 1
                    elif line.find("The following errors occurred:") == 0:
                        #
                        # All tests has run now comes the error resume
                        #
                        errors = True
                        
                output.write(line)
                output.write("\n")
                output.flush()

            trace("Run %s tests" % total, report)
            trace("Failures: %s" % failures, report)

            if resume:
                trace("The following errors occurred:", report)
                for line in resume:
                    trace(line, report)
                trace("Error details can be found in: `%s'" % os.path.join(self._buildDir, "output.txt"), report)
            trace("--------------------------------------------------------------------------", report)
        
    def buildDemos(self, compiler, arch, buildConfiguration, lang, sourceArchive):
        trace("", report)
        trace("--------------------------------------------------------------------------", report)
        trace("Building demos compiler: %s arch: %s build configuration: %s lang: %s ... " % \
              (compiler, arch, buildConfiguration, lang), report, False)
        
        if lang == "py" or lang == "rb":
            #
            # Nothing to build for python or ruby demos.
            #
            trace("Ok", report)
            return True
        
        command = None
        
        if sourceArchive:
            buildDir = os.path.join(self._sourceDir, lang, "demo")
        else:
            buildDir = os.path.join(self._demoDir, self.getDemoDir(lang))

        env = os.environ.copy()
        
        thirpatyHome = self.getThirdpartyHome()
        iceHome = self.getIceHome()
        javaHome = None    
        if lang == "java":
            javaHome = self.getJavaHome(arch, buildConfiguration)
            if not self.checkJavaSupport(arch, buildConfiguration, output):
                return False
            command = "ant"
        else:
            command = self.makeCommand(compiler, arch, buildConfiguration, lang, buildDir)

        self.setPlatformEnviroment(env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, \
                                   sourceArchive)
        
        os.chdir(buildDir)
        
        if self._verbose:
            print(command)

        p = subprocess.Popen(command, shell = True, stdout = subprocess.PIPE, \
                             stderr = subprocess.STDOUT, bufsize = 0, env = env)

        if p:
            while(True):
                line = p.stdout.readline()
                
                if p.poll() is not None and not line:
                    break
                if not line:
                    time.sleep(0.1)
                
                if type(line) != str:
                    line = line.decode()
                
                line = line.strip()
                print(line)
                output.write(line)
                output.write("\n")
                output.flush()
            
            if p.poll() == 0:
                trace("Ok", report)
                return True
            else:
                trace("Failed", report)
                trace("--------------------------------------------------------------------------", report)
                trace("", report)
                return False

    def runDemos(self, compiler, arch, buildConfiguration, lang, sourceArchive):
        
        if sourceArchive:
            os.chdir(os.path.join(self._sourceDir, lang))
        else:
            os.chdir(os.path.join(self._demoDir, self.getDemoDir(lang)))
        
        trace("Running demos ...", report)
              
        env = os.environ.copy()
        iceHome = self.getIceHome()
        
        args = "--continue"        

        if arch == "amd64":
            if not self.isWindows() or os.path.exists(os.path.join(iceHome, "bin", "x64")):
                args += " --x64"
        if buildConfiguration == "cpp11":
            args += " --c++11"
        if buildConfiguration == "debug":
            args += " --mode=debug"
        else:
            args += " --mode=release"
            
        javaHome = None
        if lang == "java":
            javaHome = self.getJavaHome(arch, buildConfiguration)
            if not self.checkJavaSupport(arch, buildConfiguration, output):
                return False
                
        thirpatyHome = self.getThirdpartyHome()            
        self.setPlatformEnviroment(env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, True)

        command = "%s %s" % (self.runScriptCommand("allDemos.py", compiler, arch, buildConfiguration, lang), args)

        if self._verbose:
            print(command)
            
        errors = False
        resume = []
        total = 0
        failures = 0
        
        p = subprocess.Popen(command, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE, \
                             stderr = subprocess.STDOUT, bufsize = 0, env = env)

        while True:
            line = p.stdout.readline()
            
            if p.poll() is not None and not line:
                break
            if not line:
                time.sleep(0.1)
                
            if type(line) != str:
                line = line.decode()

            line = line.strip()
            print(line)
           
            if line:
                if errors:
                    resume.append(line)
                elif line.find("*** running demo") == 0:
                    total = total + 1
                elif line.find("** Error logged and will be displayed again when suite is completed **") == 0:
                    failures = failures + 1
                elif line.find("The following errors occurred:") == 0:
                    #
                    # All tests has run now comes the error resume
                    #
                    errors = True
                    
            output.write(line)
            output.write("\n")
            output.flush()
            
        return (total, failures, resume)
    
    def run(self):
        if self._archive and not self._skipTests:
            #
            # Build and run tests for all compilers && buildConfigurations
            #        
            for compiler in self.getSupportedCompilers():
                
                if self._compilers and compiler not in self._compilers:
                    trace("Skiping compiler: %s, not in --filter-compilers" % compiler, report)
                    continue
                
                if self._rcompilers and compiler in self._rcompilers:
                    trace("Skiping compiler: %s, in --rfilter-compilers" % compiler, report)
                    continue
                
                for arch in self.getSupportedArchitectures():
                    total = 0
                    failures = 0
                    errors = []
                    
                    if self._archs and arch not in self._archs:
                        trace("Skiping arch: %s, not in --filter-archs" % arch, report)
                        continue
                            
                    if self._rarchs and arch in self._rarchs:
                        trace("Skiping arch: %s, in --rfilter-archs" % arch, report)
                        continue
                    
                    for conf in self.getBuildConfigurations(compiler, arch):

                        if self._configurations and conf not in self._configurations:
                            trace("Skiping configuration: %s, not in --filter-configurations" % conf, report)
                            continue

                        if self._rconfigurations and conf in self._rconfigurations:
                            trace("Skiping configuration: %s, in --rfilter-configurations" % conf, report)
                            continue

                        if compiler in ["VC90"] and arch in ["amd64"]:
                            continue

                        if conf in ["silverlight"] and arch in ["amd64"]:
                            continue

                        self.extractSourceArchive(compiler, arch, conf)
                        for lang in self.getLanguageMappings(compiler, arch, conf):
                            
                            if self._languages and lang not in self._languages:
                                trace("Skiping language: %s, not in --filter-languages" % lang, report)
                                continue
                            
                            if self._rlanguages and lang in self._rlanguages:
                                trace("Skiping language: %s, in --rfilter-languages" % lang, report)
                                continue
                            
                            if lang == "vb":
                                continue # No VB tests.
                            if not self.buildTestSuite(compiler, arch, conf, lang):
                                continue # Build failed.
                            self.runTestSuite(compiler, arch, conf, lang)
        else:
            trace("Skiping tests", output)
            
        if self._demoArchive and not self._skipDemos:
            #
            # Build and run demos for all compilers && buildConfigurations
            #        
            for compiler in self.getSupportedCompilers():

                if compiler in ["VC90"] and arch in ["amd64"]:
                    continue

                if self._compilers and compiler not in self._compilers:
                    trace("Skiping compiler: %s, not in --filter-compilers" % compiler, output)
                    continue
                
                if self._rcompilers and compiler in self._rcompilers:
                    trace("Skiping compiler: %s, in --rfilter-compilers" % compiler, output)
                    continue
                
                for arch in self.getSupportedArchitectures():
                    total = 0
                    failures = 0
                    errors = []
                    
                    if self._archs and arch not in self._archs:
                        trace("Skiping arch: %s, not in --filter-archs" % arch, output)
                        continue
                            
                    if self._rarchs and arch in self._rarchs:
                        trace("Skiping arch: %s, in --rfilter-archs" % arch, output)
                        continue
                    
                    for conf in self.getBuildConfigurations(compiler, arch):
                        if conf == "silverlight":
                            #
                            # Silverlight demos need manual intervention
                            #
                            continue
                        self.extractDemoArchive(compiler, arch, conf)
                        for lang in self.getLanguageMappings(compiler, arch, conf):
                            
                            if lang == "php":
                                continue
                            
                            if self._languages and lang not in self._languages:
                                trace("Skiping language: %s, not in --filter-languages" % lang, output)
                                continue
                            
                            if self._rlanguages and lang in self._rlanguages:
                                trace("Skiping language: %s, in --rfilter-languages" % lang, output)
                                continue
                            
                            if not self.buildDemos(compiler, arch, conf, lang, False):
                                continue # Build failed.
                            total, failures, errors = self.runDemos(compiler, arch, conf, lang, False)
                        
                            trace("Run %s demo" % total, report)
                            trace("Failures: %s" % failures, report)

                            if errors:
                                trace("The following errors occurred:", report)
                                for line in errors:
                                    trace(line, report)
                                trace("", report)
                                trace("Error details can be found in: `%s'\n" % \
                                    os.path.join(self._buildDir, "output.txt"), report)
                            trace("--------------------------------------------------------------------------", report)
        else:
            trace("Skiping demos", output)
            
class Darwin(Platform):
    
    def __init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, languages, \
                 compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, skipTests, \
                 skipDemos, parallelJobs, testConfigurations):
        Platform.__init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, \
                          languages, compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, \
                          skipTests, skipDemos, parallelJobs, testConfigurations)
        
    def getIceHome(self):
        return self._iceHome if self._iceHome else "/Library/Developer/Ice-%" % self.getVersion()

    def getSupportedCompilers(self):
        return ["clang"]

    def getBuildConfigurations(self, compiler, arch):
        return ["default", "cpp11"]

    def getSupportedLanguages(self):
        return ["cpp", "java", "py"]
        
    def getSupportedArchitectures(self):
        return ["x86", "amd64"]
        
    def isDarwin(self):
        return True
        
    def setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist):
        Platform.setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist)
            
class Linux(Platform):
    
    def __init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, languages, \
                 compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, skipTests, \
                 skipDemos, parallelJobs, testConfigurations):
        Platform.__init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, \
                          languages, compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, \
                          skipTests, skipDemos, parallelJobs, testConfigurations)
        #
        # Init Linux distribution attributes from lsb_release
        #
        p = subprocess.Popen("lsb_release -i", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            os.exists(1)
            
        self._distribution = re.sub("Distributor ID:", "", p.stdout.readline().decode('UTF-8')).strip()
        
        p = subprocess.Popen("lsb_release -r", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            os.exists(1)
        self._release = re.sub("Release:", "", p.stdout.readline().decode('UTF-8')).strip()
        
        p = subprocess.Popen("uname -m", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            os.exists(1)
        self._machine = p.stdout.readline().decode('UTF-8').strip()

    def isLinux(self):
        return True
        
    def isUbuntu(self):
        return self._distribution == "Ubuntu"
        
    def isRhel(self):
        return self._distribution.find("RedHat") != -1
        
    def isSles(self):
        return self._distribution == "SUSE LINUX"
        
    def getIceHome(self):
        return self._iceHome if self._iceHome else "/usr"
        
    def getDistribution(self):
        return self._distribution
        
    def getRelease(self):
        return self._release
        
    def getBuildConfigurations(self, compiler, arch):
        return ["default"]
        
    def getSupportedLanguages(self):
        languages = ["cpp"]
        if not self.isRhel():
            languages += ["cs"]
        languages += ["java", "php", "py", "rb"]
        return languages
        
    def getSupportedCompilers(self):
        return ["g++"]
        
    def getSupportedArchitectures(self):
        if self._machine == "x86_64":
            return ["x86", "amd64"]
        else:
            return ["x86"]
            
    def setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist):
        Platform.setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist)
        
        if lang == "java":
            #
            # Set Berkeley DB classpath
            #
            classPath = env.get("CLASSPATH", "")
            
            if classPath.find("db.jar") == -1:
                c = "/usr/share/java/db-5.3.21.jar"
                if classPath == "":
                    env["CLASSPATH"] = c
                elif classPath.find(c) == -1:
                    env["CLASSPATH"] = c + ":" + classPath
                
                #
                # Set LD_LIBRARY_PATH for Berkeley DB
                #
                if self._distribution == "Ubuntu":
                    l = "/usr/lib/i386-linux-gnu/" if arch == "x86" else "/usr/lib/x86_64-linux-gnu/"
                    libraryPath = env.get("LD_LIBRARY_PATH", "")
                    if libraryPath == "":
                        env["LD_LIBRARY_PATH"] = l
                    elif libraryPath.find(l) == -1:
                        env["LD_LIBRARY_PATH"] = l + ":" + libraryPath

class Solaris(Platform):
    
    def __init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, languages, \
                 compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, skipTests, \
                 skipDemos, parallelJobs, testConfigurations):
        Platform.__init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, \
                          languages, compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, \
                          skipTests, skipDemos, parallelJobs, testConfigurations)

    def isSolaris(self):
        return True
        
    def getIceHome(self):
        return self._iceHome if self._iceHome else "/usr"
        
    def getDistribution(self):
        return self._distribution
        
    def getRelease(self):
        return self._release
        
    def getBuildConfigurations(self, compiler, arch):
        return ["default"]
        
    def getSupportedLanguages(self):
        return ["cpp", "java"]
        
    def getSupportedCompilers(self):
        return ["CC"]
        
    def getSupportedArchitectures(self):
        return ["x86", "amd64"]
            
    def setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist):
        Platform.setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist)
        
        if lang == "java":
            #
            # Set Berkeley DB classpath
            #
            classPath = env.get("CLASSPATH", "")
            
            if classPath.find("db.jar") == -1:
                c = "/usr/share/java/db-5.3.21.jar"
                if classPath == "":
                    env["CLASSPATH"] = c
                elif classPath.find(c) == -1:
                    env["CLASSPATH"] = c + ":" + classPath
                
    def makeCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        command = "gmake"
        if self._parallelJobs:
            command += " -j%s" % self._parallelJobs
        return command
        
    def makeCleanCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "gmake clean"

class Windows(Platform):
    
    def __init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, languages, \
                 compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, skipTests, \
                 skipDemos, parallelJobs, testConfigurations):
        Platform.__init__(self, iceHome, buildDir, archive, demoArchive, demoScriptsArchive, debug, verbose, \
                          languages, compilers, archs, configurations, rlanguages, rcompilers, rarchs, rconfigurations, \
                          skipTests, skipDemos, parallelJobs, testConfigurations)

    def getIceHome(self):
        return self._iceHome if self._iceHome else "C:\Program Files (x86)\ZeroC\Ice-3.5.1"
    
    def makeSilverlightCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && devenv testsl.sln /build" % (self.getVcVarsAll(compiler), arch, buildDir)

    def makeCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && nmake /f Makefile.mak" % (self.getVcVarsAll(compiler), arch, buildDir)

    def makeCleanCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && nmake /f Makefile.mak clean" % (self.getVcVarsAll(compiler), arch, buildDir)
    
    def runScriptCommand(self, script, compiler, arch, buildConfiguration, lang):
        if lang != "py":
            python = sys.executable
        else:
            python = os.path.join(self.getPythonHome(compiler, arch, buildConfiguration, lang), "python")
        return "\"%s\" %s && %s %s" % (self.getVcVarsAll(compiler), arch, python, script)

    def isWindows(self):
        return True
        
    def isWindows8(self):
        version = sys.getwindowsversion()
        return version.major == 6 and version.minor == 2 and version.build == 9200
        
    
    def checkJavaSupport(self, arch, buildConfiguration, output):
        javaHome = self.getJavaHome(arch, buildConfiguration)
        if javaHome is None:
            trace("Could not detect Java for arch: %s buildConfiguration: %s" % (arch, buildConfiguration), output)
            return False
        return True

    def getBuildConfigurations(self, compiler, arch):
        
        buildConfigurations = ["default"]
        if compiler == "VC100":
            buildConfigurations.append("debug")
            buildConfigurations.append("silverlight")
            buildConfigurations.append("java1.7")
        elif compiler == "VC110":
            buildConfigurations.append("debug")
            buildConfigurations.append("silverlight")
            if self.isWindows8():
                buildConfigurations.append("winrt")
        return buildConfigurations

    def getSupportedCompilers(self):
        return ["VC90", "VC100", "VC110"]
        
    def getSupportedLanguages(self):
        return ["cpp", "cs", "java", "php", "py", "rb", "vb"]
        
    def getSupportedArchitectures(self):
        archs = ["x86"]
        
        if os.environ.get("PROCESSOR_ARCHITECTURE", "") == "AMD64" or \
           os.environ.get("PROCESSOR_ARCHITEW6432", "") == "AMD64":
            archs.append("amd64")
        
        return archs

    def getThirdpartyHome(self):
        return BuildUtils.getThirdpartyHome(version)

    def getVcVarsAll(self, compiler):
        return BuildUtils.getVcVarsAll(compiler)

    def getJavaHome(self, arch, buildConfiguration):
        return BuildUtils.getJavaHome(arch, buildConfiguration)

    def getPythonHome(self, compiler, arch, buildConfiguration, lang):
        return BuildUtils.getIceHome(arch)
    
    def setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist):
        Platform.setPlatformEnviroment(self, env, compiler, arch, buildConfiguration, lang, iceHome, thirpatyHome, javaHome, useBinDist)
        
        if not os.path.exists(os.path.join(iceHome, "bin", "bzip2.dll")):
            path = env.get("PATH", "")
            p = None
            
            if compiler == "VC100":
                if arch == "x86":
                    p = os.path.join(thirpatyHome, "bin")
                else:
                    p = os.path.join(thirpatyHome, "bin", "x64")
                    
            if compiler == "VC110":
                if arch == "x86":
                    p = os.path.join(thirpatyHome, "bin", "vc110")
                else:
                    p = os.path.join(thirpatyHome, "bin", "vc110", "x64")
                    
            if path == "":
                env["PATH"] = p
            else:
                env["PATH"] = p + ";" + path

        if lang == "cs" and compiler == "VC90":
            env["COMPACT"] = "yes"

        if lang == "java":
            #
            # Set Berkeley DB classpath
            #
            classPath = env.get("CLASSPATH", "")
            
            c = None
            if os.path.exists(os.path.join(iceHome, "lib", "db.jar")):
                c = os.path.join(iceHome, "lib", "db.jar")
            elif os.path.exists(os.path.join(thirpatyHome, "lib", "db.jar")):
                c = os.path.join(thirpatyHome, "lib", "db.jar")

            if classPath == "":
                env["CLASSPATH"] = c
            elif classPath.find(c) == -1:
                env["CLASSPATH"] = c + ";" + classPath
                
            #
            # Prepend Java bin directory from JAVA_HOME to path
            #
            if javaHome:
                path = env.get("PATH", "")
                p = os.path.join(javaHome, "bin")
                
                if path == "":
                    env["PATH"] = p
                else:
                    env["PATH"] = p + ";" + path
                
#
# Program usage.
#
def usage():
    print("")
    print("Options:")
    print("  --help                             Show this message.")
    print("  --verbose                          Be verbose.")
    print("  --build-dir=<path>                 Directory to build the sources")
    print("  --source-archive=<path>            Ice source distribution archive")
    print("  --demo-archive=<path>              Demo source distribution archive")
    print("  --ice-home=<path>                  Use the binary distribution from the given path.")
    print("  --parallel-jobs=<NUMBER>           Makefile -j option to builds")
    print("  --filter-languages=<name>          Just build and run the given languages")
    print("  --filter-compilers=<name>          Just build and run the given compilers")
    print("  --filter-archs=<name>              Just build and run the given architectures")
    print("  --filter-configurations=<name>     Just build and run the given configurations")
    print("  --rfilter-languages=<name>         Just build and run the languages not in the list")
    print("  --rfilter-compilers=<name>         Just build and run the compilers not in the list")
    print("  --rfilter-archs=<name>             Just build and run the languages not in the list")
    print("  --rfilter-configurations=<name>    Just build and run the configurations not in the list")
    print("  --skip-tests                       Don't build or run tests")
    print("  --skip-demos                       Don't build or run demos")
    print("  --print-languages                  Print the platform supported languages")
    print("  --print-compilers                  Print the platform supported compilers")
    print("  --print-archs                      Print the platform supported archs")
    print("  --tests-driver=<path>              Run tests with configurations from test driver file")
    print("                                     if --test-driver isn't used we run allTests.py with --all option")
    print("                                     the test driver file contains a configuration per line using :")
    print("                                     as delimiter, each line has the form of \"name:options:languages\"")
    print("                                     if languages part isn't present the configuration apply to all")
    print("                                     languages otherwise apply just to the given languages.")
    print("")
    

verbose = False
sourceArchive = None
demoArchive = None
demoScriptsArchive = None
buildDir = None
parallelJobs = None
iceHome = None
debug = False

filterLanguages = []
filterCompilers = []
filterArchs = []
filterConfigurations = []

skipTests = False
skipDemos = False

rFilterLanguages = []
rFilterCompilers = []
rFilterArchs = []
rFilterConfigurations = []

args = None
opts = None

printLanguages = False
printCompilers = False
printArchs = False

testDriver = None

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "verbose", "skip-tests", "skip-demos", "ice-home=", \
                                                  "parallel-jobs=", "filter-languages=", "filter-compilers=", \
                                                  "filter-archs=", "filter-configurations=", "rfilter-languages=", \
                                                  "rfilter-compilers=", "rfilter-archs=", "rfilter-configurations", \
                                                  "print-languages", "print-compilers", "print-archs", "test-driver="])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

if args:
    usage()
    sys.exit(1)

for o, a in opts:
    if o == "--ice-home":
        iceHome = a
    elif o == "--verbose":
        verbose = True
    elif o == "--filter-languages":
        filterLanguages.append(a)
    elif o == "--filter-compilers":
        filterCompilers.append(a)
    elif o == "--filter-archs":
        filterArchs.append(a)
    elif o == "--filter-configurations":
        filterConfigurations.append(a)
    elif o == "--rfilter-languages":
        rFilterLanguages.append(a)
    elif o == "--rfilter-compilers":
        rFilterCompilers.append(a)
    elif o == "--rfilter-archs":
        rFilterArchs.append(a)
    elif o == "--rfilter-configurations":
        rFilterConfigurations.append(a)
    elif o == "--skip-tests":
        skipTests = True
    elif o == "--skip-demos":
        skipDemos = True
    elif o == "--parallel-jobs":
        parallelJobs = a
    elif o == "--print-languages":
        printLanguages = True
    elif o == "--print-compilers":
        printCompilers = True
    elif o == "--print-archs":
        printArchs = True
    elif o == "--test-driver":
        testDriver = a

#
# Defaults
#
if buildDir == None:
    buildDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "build"))
else:
    buildDir = os.path.expanduser(buildDir)
    if not os.path.isabs(buildDir):
        buildDir = os.path.abspath(os.path.join(os.getcwd(), buildDir))

#
# Open output files
#
if not os.path.exists(os.path.join(buildDir)):
    os.mkdir(os.path.join(buildDir))
output = open(os.path.join(buildDir, "output.txt"), "w")
report = open(os.path.join(buildDir, "report.txt"), "w")

trace("Using `%s' path as build directory" % buildDir, report)

#
# We use 40 as default parallel jobs for SunOS and 8 everywhere else
#
if parallelJobs == None:
    parallelJobs = 40 if sys.platform == "sunos5" else 8
    
#
# If --ice-home parameter isn't set we get the defaults from BuildUtils
#
if iceHome == None:
    iceHome = BuildUtils.getIceHome(version)
else:
    iceHome = os.path.expanduser(iceHome)
    if not os.path.isabs(iceHome):
        iceHome = os.path.abspath(os.path.join(os.getcwd(), iceHome))
        
if iceHome == None:
    trace("Unable to detect a valid Ice installation, you can specify a custom installation path with --ice-home", \
          report)
    usage()
    sys.exit(1)
trace("Using Ice installation from `%s'" % iceHome, report)

if sys.platform == "win32":
    sourceArchive = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "Ice-%s.zip" % version))
else:
    sourceArchive = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "Ice-%s.tar.gz" % version))

if not os.path.exists(sourceArchive):
    trace("Ice source archive not found in `%s'" % sourceArchive, report)
    trace(sys.argv[0] + ": you must run testicedist.py from the dist-" + version + " directory created by makedist.py", report)
    usage()
    sys.exit(1)
        
trace("Using `%s' source archive" % sourceArchive, report)


if sys.platform == "win32":
    demoArchive = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "Ice-%s-demos.zip" % version))
else:
    demoArchive = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "Ice-%s-demos.tar.gz" % version))

if not os.path.exists(demoArchive):
    trace("Ice demo archive not found in `%s'" % demoArchive, report)
    trace(sys.argv[0] + ": you must run testicedist.py from the dist-" + version + " directory created by makedist.py", report)
    usage()
    sys.exit(1)
    
trace("Using `%s' demo archive" % demoArchive, report)

demoScriptsArchive = os.path.join(os.path.dirname(demoArchive), re.sub("-demos.", "-demo-scripts.", \
                                                                        os.path.basename(demoArchive)))
    
platformName = "linux" if sys.platform.startswith("linux") else sys.platform

if testDriver == None:
    if os.path.exists(os.path.join(os.path.dirname(__file__), "..", "..", "tests-driver-%s.cnf" % platformName)):
        testDriver = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "tests-driver-%s.cnf" % platformName))
elif not os.path.exists(testDriver):
    trace("Test driver file not found in `%s'" % testDriver, report)
    usage()
    sys.exit(1)

    
class TestConfiguration:
    
    def __init__(self, name, options, languages):
        self.name = name
        self.options = options
        self.languages = languages
        
testConfigurations = []

if not testDriver:
    testConfigurations.append(TestConfiguration("all", "--all", []))
else:
    #
    # Parse test driver file
    #
    f = open(testDriver, "r")
    for line in f:
        line = line.strip()
        
        i = line.find("#")
        if i == 0:
            #
            # Skiping comment.
            #
            continue
        
        #
        # Partial comment
        #
        if i != -1:
            line = line[:i]
        line = line.strip()
        
        if not line:
            continue
        
        i = line.find(":")
        if i == -1:
            trace("Invalid buildConfiguration line `%s' missing ':' delimiter" % line, report)
            usage()
            sys.exit(1)
            
        j = line.find(":", i + 1)
            
        name = line[:i].strip()
        options = ""
        languages = []
        
        if j != -1:
            options = line[i + 1:j - 1].strip()
            languages = line[j + 1:].strip().split()
        else:
            options = line[i + 1:].strip()
        
        #
        # Remove empty any entries left by split
        #
        tmp = []
        for lang in languages:
            if lang:
                tmp.append(lang)
        languages = tmp
        
        testConfigurations.append(TestConfiguration(name, options, languages))
            
platform = None

if platformName == "win32":
    platform = Windows(iceHome, buildDir, sourceArchive, demoArchive, demoScriptsArchive, debug, verbose, \
                       filterLanguages, filterCompilers, filterArchs, filterConfigurations, rFilterLanguages, \
                       rFilterCompilers, rFilterArchs, rFilterConfigurations, skipTests, skipDemos, parallelJobs, \
                       testConfigurations)
elif platformName == "sunos5":
   platform = Solaris(iceHome, buildDir, sourceArchive, demoArchive, demoScriptsArchive, debug, verbose, \
                      filterLanguages, filterCompilers, filterArchs, filterConfigurations, rFilterLanguages, \
                      rFilterCompilers, rFilterArchs, rFilterConfigurations, skipTests, skipDemos, parallelJobs, \
                      testConfigurations)
elif platformName == "darwin":
    platform = Darwin(iceHome, buildDir, sourceArchive, demoArchive, demoScriptsArchive, debug, verbose, \
                      filterLanguages, filterCompilers, filterArchs, filterConfigurations, rFilterLanguages, \
                      rFilterCompilers, rFilterArchs, rFilterConfigurations, skipTests, skipDemos, parallelJobs, \
                      testConfigurations)
elif platformName == "linux":
    platform = Linux(iceHome, buildDir, sourceArchive, demoArchive, demoScriptsArchive, debug, verbose, \
                     filterLanguages, filterCompilers, filterArchs, filterConfigurations, rFilterLanguages, \
                     rFilterCompilers, rFilterArchs, rFilterConfigurations, skipTests, skipDemos, parallelJobs, \
                     testConfigurations)
else:
    print("Unknown platform: %s" % sys.platform)
    sys.exit(1)

if printLanguages or printCompilers or printArchs:
    if printLanguages:
        print("Supported languages: %s" % platform.getSupportedLanguages())
    if printCompilers:
        print("Supported compilers: %s" % platform.getSupportedCompilers())
    if printArchs:
        print("Supported architectures: %s" % platform.getSupportedArchitectures())
else:
    platform.run()

output.close()
report.close()
