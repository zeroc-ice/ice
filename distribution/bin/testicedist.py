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

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, zipfile, time, datetime, threading, tempfile

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

    if f:
        f.write(msg)
        if nl:
            f.write('\n')
        f.flush()
    
def prependPathToEnvironVar(env, var, path):
    env[var] = os.pathsep.join([path] + env.get(var, "").split(os.pathsep)).strip(os.pathsep)

class TestConfiguration:    
    def __init__(self, name, options, compilers = None, archs = None, configs = None, languages = None):
        self.name = name
        self.options = options
        self.compilers = compilers
        self.archs = archs
        self.configs = configs
        self.languages = languages

    def runWith(self, compiler, arch, conf, lang):
        if self.compilers and compiler not in self.compilers:
            return False
        if self.archs and arch not in self.archs:
            return False
        if self.configs and conf not in self.configs:
            return False
        if self.languages and lang not in self.languages:
            return False
        return True

class TestDemoResults:

    def __init__(self):
        self._sourceDir = None
        self._total = 0
        self._current = 0
        self._currentTest = None
        self._failures = []
        self._finished = False

        self.matchStartTest = re.compile("\*\*\* running tests ([0-9]+)\/([0-9]+) in (.*)")
        self.matchConfiguration = re.compile("\*\*\* configuration: (.*)")
        self.matchFailedTest = re.compile("\(\'test in (.*) failed with exit status\', ([0-9]+)\)")
        self.matchStartDemo = re.compile("\*\*\* running demo ([0-9]+)\/([0-9]+) in (.*)")
        self.matchFailedDemo = re.compile("\(\'demo in (.*) failed with exit status\', ([0-9]+)\)")
        self.matchEndTests = re.compile("The following errors occurred")

    def setSourceDir(self, sourceDir):
        self._sourceDir = sourceDir + '/'

    def filter(self, line):

        if(line.find("error:")) >= 0:
            trace(line, report)
        if(line.find("warning:") >= 0 and line.find("deprecated") == -1):
            trace(line, report)

        # Match start test/demo line
        m = self.matchStartTest.match(line)
        if not m:
            m = self.matchStartDemo.match(line)
        if m:
            if self._current > 0:
                trace("ok", report)
                self._current = 0

            self._current = int(m.group(1))
            if self._total == 0:
                self._total = int(m.group(2))
            self._currentTest = m.group(3)

            if self._currentTest.startswith(self._sourceDir):
                self._currentTest = self._currentTest[len(self._sourceDir):]
                
            trace("[%d/%d] %s" % (self._current, self._total, self._currentTest), report, False)
            return

        # Match configuration line
        m = self.matchConfiguration.match(line)
        if m:
            c = m.group(1)
            if c != "Default":
                trace(" (" + c + ")", report, False)
            trace("... ", report, False)
            return

        # Match failed test/demo line (only if not the end of the demos/tests where
        # a summary of all the failures is printed)
        if not self._finished:
            m = self.matchFailedTest.match(line)
            if not m:
                m = self.matchFailedDemo.match(line)
            if m:
                if self._current > 0:
                    trace("failed! (%d/%d status = %s)" % (self._current, self._total, m.group(2)), report)
                    self._current = 0
                    self._failures.append(self._currentTest)
                return

        # Match end of the tests
        m = self.matchEndTests.match(line)
        if m:
            self._finished = True

    def flush(self):
        if self._current > 0:
            trace("ok", report)
            self._current = 0
        elif self._total == 0:
            trace("nothing to run", report)

    def interrupted(self):
        if self._current > 0:
            self._total = self._current - 1

    def getFailures(self):
        return self._failures

class Summary:

    def __init__(self, desc):
        self._desc = desc
        self._results = []
        self._failures = []
        self._buildTime = None
        self._runTime = None
        self._totalBuildTime = 0
        self._totalRunTime = 0

    def addResults(self, compiler, arch, buildConfiguration, lang, result):
        self._results.append((compiler, arch, buildConfiguration, lang, result))

    def startBuild(self):
        self._buildTime = time.time()

    def addBuildFailure(self, compiler, arch, buildConfiguration, lang):
        self._failures.append((compiler, arch, buildConfiguration, lang))

    def finishBuild(self):
        self._totalBuildTime += time.time() - self._buildTime

    def startRun(self):
        self._runTime = time.time()

    def finishRun(self):
        self._totalRunTime += time.time() - self._runTime

    def printSummary(self):
        if len(self._failures) > 0:
            trace("\nBuild failure(s) for " + self._desc + " with the following configuration(s):", report)
            for (compiler, arch, conf, lang) in self._failures:
                trace("- %s %s (%s/%s/%s)" % (lang, self._desc, compiler, arch, conf), report)
            trace("", report)

        total = 0
        failureCount = 0
        for (_, _, _, _, r) in self._results:
            total += r._total
            failureCount += len(r._failures)

        if total > 0:
            if failureCount == 0:
                trace("Ran %d %s without any failures" % (total, self._desc), report)
            elif failureCount == 1:
                trace("Ran %d %s sucessfully, 1 failure:" % (total - 1, self._desc), report)
            else:
                trace("Ran %d %s sucessfully, %d failures:" % (total - failureCount, self._desc, failureCount), report)
                
            if failureCount > 0:
                for (compiler, arch, conf, lang, r) in self._results:
                    for test in r.getFailures():
                        trace("- %s/%s (%s/%s/%s)" % (lang, test, compiler, arch, conf), report)

        if self._totalBuildTime > 0:
            trace("Build %s duration: %s (HH:mm:ss)" % (self._desc, datetime.timedelta(seconds=self._totalBuildTime)), 
                  report)
        if self._totalRunTime > 0:
            trace("Run %s duration: %s (HH:mm:ss)" % (self._desc, datetime.timedelta(seconds=self._totalRunTime)),
                  report)

def include(value, include, exclude):
    if include and value not in include:
        return False
    if exclude and value in exclude:
        return False
    return True

def filterRC(runnableConfigs, values, compiler = None, arch = None, config = None, language = None):
    d = runnableConfigs
    if compiler:
        if compiler in d:
            d = d[compiler]
        else:
            return []
    if arch:
        if arch in d:
            d = d[arch]
        else:
            return []
    if config:
        if config in d:
            d = d[config]
        else:
            return []
    if language: 
        if language in d[language]:
            d = d[language]
        else:
            return []

    f = []
    for v in values:
        if v in d:
            f.append(v)
    return f

def filterBuildOutput(line):
    if(line.find("error:")) >= 0:
        trace(line, report)
    if(line.find("warning:") >= 0 and line.find("deprecated") == -1):
        trace(line, report)
    return True

def spawnAndWatch(command, env, filterFunc):
        
    p = subprocess.Popen(command, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT, bufsize = 0, env = env)    
    if not p:
        return False

    while(True):

        line = p.stdout.readline()            
        if p.poll() is not None and not line:
            # The process terminated
            break
                    
        if type(line) != str:
            line = line.decode()

        filterFunc(line)

        output.write(line)
        output.flush()
                
    return p.poll() == 0

class Platform:

    def __init__(self, distDir):
        self._iceHome = BuildUtils.getIceHome(version)
        self._archive = os.path.join(distDir, "Ice-%s.tar.gz" % version)
        self._demoArchive = os.path.join(distDir, "Ice-%s-demos.tar.gz" % version)
        self._demoScriptsArchive = os.path.join(distDir, "Ice-%s-demo-scripts.tar.gz" % version)
        self._buildDir = os.path.abspath(os.path.join(distDir, "build"))
        self._verbose = False
        self._languages = []
        self._compilers = []
        self._archs = []
        self._configurations = []
        self._rlanguages = []
        self._rcompilers = []
        self._rarchs = []
        self._rconfigurations = []
        self._skipBuild = False
        self._skipTests = False
        self._skipDemos = False
        self._parallelJobs = 8
        self._testConfigurations = []
        self._demoConfiguration = ""

        self._sourceDir = None
        self._demoDir = None        
    
    def validateConfiguration(self):

        #
        # Check paths are valid
        #
        if not self._iceHome:
            print("Can't find an Ice " + version + " binary distribution, either set ICE_HOME or " + 
                  "use --ice-home to specify the path of the binary distribution")
            sys.exit(1)
        elif self._iceHome and not os.path.exists(self._iceHome):
            print("Invalid Ice Home setting `%s'" % self._iceHome)
            sys.exit(1)

        if self._archive and not os.path.exists(self._archive):
            print(sys.argv[0] + "Ice source archive not found: `%s'" % self._archive)
            print(sys.argv[0] + ": you must run testicedist.py from the directory created by makedist.py")
            sys.exit(1)
            
        if self._demoArchive and not os.path.exists(self._demoArchive):
            print(sys.argv[0] + "Ice demo source archive not found: `%s'" % self._demoArchive)
            print(sys.argv[0] + ": you must run testicedist.py from the directory created by makedist.py")
            sys.exit(1)
        
        if self._demoScriptsArchive and not os.path.exists(self._demoScriptsArchive):
            print(sys.argv[0] + "Ice demo scripts source archive not found: `%s'" % self._demoScriptsArchive)
            print(sys.argv[0] + ": you must run testicedist.py from the directory created by makedist.py")
            sys.exit(1)

    def printConfigurationSummary(self, f = None):
        trace("Using `%s' path as build directory" % self._buildDir, f)
        trace("Using Ice installation from `%s'" % self._iceHome, f)
        trace("Using `%s' source archive" % self._archive, f)
        trace("Using `%s' demo archive" % self._demoArchive, f)
        trace("", f)
        trace("Compilers: %s" % self.getCompilers(), f)
        trace("Architectures: %s" % self.getArchitectures(), f)
        trace("Languages: %s" % self.getLanguages(), f)

        if not self._skipTests:
            trace("\nTest configurations:", f)
            (_, total, runnableConfigs) = self.getBuildAndRunConfigs(self.filterTests)
            count = 1
            for comp in filterRC(runnableConfigs, self.getSupportedCompilers()):
                for arch in filterRC(runnableConfigs, self.getSupportedArchitectures(), comp):
                    for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp, arch), comp, arch):
                        for lang in filterRC(runnableConfigs, self.getSupportedLanguages(), comp, arch, conf):
                            for c in runnableConfigs[comp][arch][conf][lang]:
                                trace("- [%d] %s %s tests (%s/%s/%s)" % (count, lang, c.name, comp, arch, conf), f)
                                count += 1 
    
        if not self._skipDemos:
            trace("\nDemo configurations:", f)
            (_, total, runnableConfigs) = self.getBuildAndRunConfigs(self.filterDemos)
            count = 1
            for comp in filterRC(runnableConfigs, self.getSupportedCompilers()):
                for arch in filterRC(runnableConfigs, self.getSupportedArchitectures(), comp):
                    for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp, arch), comp, arch):
                        for lang in filterRC(runnableConfigs, self.getSupportedLanguages(), comp, arch, conf):
                            trace("- [%d] %s demos (%s/%s/%s)" % (count, lang, comp, arch, conf), f)
                            count += 1

        trace("", f)
            
    def getTestConfigurations(self, filterArg, rfilterArg):
        f = ""
        f += " --filter=\"%s\"" % filterArg if filterArg else ""
        f += " --rfilter=\"%s\"" % rfilterArg if rfilterArg else ""

        #
        # By default, run tests with --all on the default architecture. On 
        # other architectures run tests without --all. Run cross tests on 
        # the default architecture only.
        # 
        defaultArch = self.getDefaultArchitecture()
        otherArchs = self.getSupportedArchitectures()
        otherArchs.remove(defaultArch)

        configs = []
        configs.append(TestConfiguration("all", "--all" + f, archs = [defaultArch] , configs = ["default"]))
        if(otherArchs):
            configs.append(TestConfiguration("default", "" + f, archs = otherArchs, configs = ["default"]))

        langs = set(["cpp", "java", "cs"]) & set(self.getSupportedLanguages())
        for l1 in langs:
            for l2 in langs:
                if l1 != l2:
                    configs.append(TestConfiguration("cross-tcp", 
                                                     "--cross=%s%s" % (l2,f),
                                                     configs = ["default"], 
                                                     archs = [defaultArch],
                                                     languages = [l1]))
                    if not filterArg and not rfilterArg:
                        configs.append(TestConfiguration("cross-ssl", 
                                                         "--cross=%s --protocol=ssl --filter=\"Ice/operations\"" % l2 ,
                                                         configs = ["default"], 
                                                         archs = [defaultArch],
                                                         languages = [l1]))

        return configs

    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = os.environ.copy()

        if useBinDist:
            env["USE_BIN_DIST"] = "yes"
        
        if buildConfiguration != "debug":
            env["OPTIMIZE"] = "yes"

        if self._iceHome:
            env["ICE_HOME"] = self._iceHome
        
        if buildConfiguration == "cpp11":
            env["CPP11"] = "yes"
            
        if self.is64(arch) and not self.isWindows():
            env["LP64"] = "yes"
            
        if lang == "java":

            javaHome = self.getJavaHome(arch, buildConfiguration)
            if javaHome:            
                env["JAVA_HOME"] = javaHome
            
            if os.path.exists(os.path.join(self._iceHome, "lib", "db.jar")):
                prependPathToEnvironVar(env, "CLASSPATH", os.path.join(self._iceHome, "lib", "db.jar"))
        return env
            
    def checkJavaSupport(self, arch, buildConfiguration, output):
        return True

    def is64(self, arch):
        return arch == "x64" or arch == "sparcv9"
            
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

    def getCompilers(self):
        return list(filter(lambda x: include(x, self._compilers, self._rcompilers), self.getSupportedCompilers()))

    def getArchitectures(self):
        return list(filter(lambda x: include(x, self._archs, self._rarchs), self.getSupportedArchitectures()))

    def getLanguages(self):
        return list(filter(lambda x: include(x, self._languages, self._rlanguages), self.getSupportedLanguages()))

    def getConfigurations(self, compiler, arch):
        return list(filter(lambda x: include(x, self._configurations, self._rconfigurations), 
                           self.getSupportedConfigurations(compiler, arch)))
        
    def getDefaultArchitecture(self):
        # Default architecture is first non-filtered architecture by default
        for a in self.getArchitectures():
            if self._archs and not a in self._archs:
                continue
            if self._rarchs and a in self._rarchs:
                continue
            return a

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

    def getJavaHome(self, arch, buildConfiguration):
        return None
    
    def getLanguageMappings(self, compiler, arch, buildConfiguration):
        if buildConfiguration in ["debug", "cpp11", "winrt"]:
            languages = ["cpp"]
        elif buildConfiguration == "java1.6":
            languages = ["java"]
        elif compiler == "VC90" or buildConfiguration == "silverlight":
            languages = ["cs"]
        else:
            languages = self.getSupportedLanguages()

        if arch != self.getDefaultArchitecture() and "java" in languages:
            languages.remove("java")

        return filter(lambda x: include(x, self._languages, self._rlanguages), languages)

    def getSupportedConfigurations(self, compiler, arch):
        return ["default"]

    def makeCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        command = "make"
        if self._parallelJobs:
            command += " -j%s" % self._parallelJobs
        return command
        
    def makeDemosCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return self.makeCommand(compiler, arch, buildConfiguration, lang, buildDir)
        
    def makeCleanCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "make clean"

    def runScriptCommand(self, script, compiler, arch, buildConfiguration, lang):
        return "%s %s" % (sys.executable, script)

    def getBuildAndRunConfigs(self, filterFn):
        buildTotal = 0
        runTotal = 0
        runnableConfigs = {}
        for compiler in self.getCompilers():
            runnableConfigs[compiler] = {}
            for arch in self.getArchitectures():
                runnableConfigs[compiler][arch] = {}
                for conf in self.getConfigurations(compiler, arch):
                    runnableConfigs[compiler][arch][conf] = {}
                    for lang in self.getLanguageMappings(compiler, arch, conf):
                        configs = []
                        if(filterFn(compiler, arch, conf, lang, configs)):
                            runnableConfigs[compiler][arch][conf][lang] = configs
                            buildTotal += 1
                            runTotal += len(configs)
        return (buildTotal, runTotal, runnableConfigs)

    def buildAndRun(self, build, run, filterFn, extract, start, summary):

        (buildTotal, runTotal, runnableConfigs) = self.getBuildAndRunConfigs(filterFn) 

        count = 1
        if not self._skipBuild:
            for comp in filterRC(runnableConfigs, self.getSupportedCompilers()):
                for arch in filterRC(runnableConfigs, self.getSupportedArchitectures(), comp):
                    for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp, arch), comp, arch):
                        extract(comp, arch, conf)
                        for lang in filterRC(runnableConfigs, self.getSupportedLanguages(), comp, arch, conf):
                            summary.startBuild()
                            try:
                                if not build(comp, arch, conf, lang, "%d/%d" % (count, buildTotal)):
                                    summary.addBuildFailure(comp, arch, conf, lang)
                                    runTotal -= len(runnableConfigs[comp][arch][conf][lang])
                                    runnableConfigs[comp][arch][conf][lang] = []
                            except KeyboardInterrupt:
                                raise
                            finally:
                                summary.finishBuild()
                            count += 1

        count = 1
        configIndex = None
        runIndex = None
        if start:
            start = start.split('/')
            configIndex = int(start[0])
            if(len(start) > 1):
                runIndex = start[1]

        for comp in filterRC(runnableConfigs, self.getSupportedCompilers()):
            for arch in filterRC(runnableConfigs, self.getSupportedArchitectures(), comp):
                for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp, arch), comp, arch):
                    extract(comp, arch, conf)
                    for lang in filterRC(runnableConfigs, self.getSupportedLanguages(), comp, arch, conf):
                        for testConfig in runnableConfigs[comp][arch][conf][lang]:
                            if configIndex:
                                if count < configIndex:
                                    count += 1
                                    continue # Skip
                                elif count > configIndex:
                                    runIndex = None
                                            
                            index = "%d/%d" % (count, runTotal)
                            summary.startRun()
                            r = TestDemoResults()
                            summary.addResults(comp, arch, conf, lang, r)
                            try:
                                run(comp, arch, conf, lang, testConfig, r, runIndex, index)
                                r.flush()
                            except KeyboardInterrupt:
                                r.interrupted()
                                raise
                            finally:
                                summary.finishRun()
                            count += 1
                            

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
            if self._archive.endswith(".tar.gz"):
                runCommand("tar -zxf %s" %(self._archive), self._verbose)
            elif self._archive.endswith(".zip"):
                zipfile.ZipFile(self._archive).extractall()                
            
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
            if self._demoArchive.endswith(".tar.gz"):
                runCommand("tar -zxf %s" %(self._demoArchive), self._verbose)
                runCommand("tar -zxf %s" %(self._demoScriptsArchive), self._verbose)
            elif self._demoArchive.endswith(".zip"):
                zipfile.ZipFile(self._demoArchive).extractall()
                zipfile.ZipFile(self._demoScriptsArchive).extractall()
            
    def buildTests(self, compiler, arch, buildConfiguration, lang, index):
            
        trace("*** [%s] building %s tests (%s/%s/%s)... " % (index, lang, compiler, arch, buildConfiguration), report,
              False)

        if lang == "py" or lang == "rb":
            trace("ok", report)
            return True

        buildDir = None
        if lang == "java":
            buildDir = os.path.join(self._sourceDir, lang)
        else:
            buildDir = os.path.join(self._sourceDir, lang, "test")
                
        commands = []
        if buildConfiguration == "silverlight":
            commands.append(self.makeSilverlightCommand(compiler, arch, buildConfiguration, lang, buildDir))

        if lang == "java":
            if not self.checkJavaSupport(arch, buildConfiguration, output):
                return False
            commands.append("ant test-jar")
        else:
            commands.append(self.makeCommand(compiler, arch, buildConfiguration, lang, buildDir))

        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, True)

        os.chdir(buildDir)

        status = True
        for command in commands:
            if self._verbose:
                print(command)
        
        if spawnAndWatch(command, env, filterBuildOutput):
            trace("ok", report)
            return True
        else:
            trace("failed!", report)
            return False

    def runTests(self, compiler, arch, buildConfiguration, lang, testConf, results, start, index):
        os.chdir(os.path.join(self._sourceDir, lang))
        results.setSourceDir(os.path.join(self._sourceDir, lang))

        args = "--continue %s" % testConf.options

        if start:
            args += " --start=%s" % start

        if self.is64(arch):
            args += " --x64"
                    
        if buildConfiguration == "cpp11":
            args += " --c++11"
            
        if lang == "cs" and compiler == "VC90":
            args += " --compact"

        if buildConfiguration == "silverlight":
            args += " --silverlight"
                
        if lang == "java" and not self.checkJavaSupport(arch, buildConfiguration, output):
            return False
                    
        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, True)

        command = "%s %s" % (self.runScriptCommand("allTests.py", compiler, arch, buildConfiguration, lang), args)

        if self._verbose:
            print(command)
                
        trace("", report)
        trace("*** [%s] %s: running %s tests with %s (%s/%s/%s)" % (index, testConf.name, lang, testConf.options, 
                                                                    compiler, arch, buildConfiguration), report)
        spawnAndWatch(command, env, lambda line: results.filter(line))
        return True

    def filterTests(self, compiler, arch, conf, lang, testConfigs):
        if compiler == "VC90" and arch == "x64":
            return False
        if conf == "silverlight" and arch == "x64":
            return False
        if lang == "vb":
            return False

        for testConf in self._testConfigurations:
            if testConf.runWith(compiler, arch, conf, lang):
                testConfigs.append(testConf)
        return True
            
    def buildDemos(self, compiler, arch, buildConfiguration, lang, index):

        trace("*** [%s] building %s demos (%s/%s/%s)... " % (index, lang, compiler, arch, buildConfiguration),
              report, False)
        if lang == "py" or lang == "rb":
            trace("ok", report)
            return True

        if lang == "java" and not self.checkJavaSupport(arch, buildConfiguration, output):
            return False

        sourceArchive = False # TODO: Support?
        if sourceArchive:
            buildDir = os.path.join(self._sourceDir, lang, "demo")
        else:
            buildDir = os.path.join(self._demoDir, self.getDemoDir(lang))

        command = self.makeDemosCommand(compiler, arch, buildConfiguration, lang, buildDir) if lang != "java" else "ant"
        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, sourceArchive)
        
        os.chdir(buildDir)
        
        if self._verbose:
            print(command)

        if spawnAndWatch(command, env, filterBuildOutput):
            trace("ok", report)
            return True
        else:
            trace("failed!", report)
            return False

    def runDemos(self, compiler, arch, buildConfiguration, lang, testConf, results, start, index):

        sourceArchive = False # TODO: Support?
        sourceDir = None
        if sourceArchive:
            sourceDir = os.path.join(self._sourceDir, lang)
        else:
            sourceDir = os.path.join(self._demoDir, self.getDemoDir(lang))
        os.chdir(sourceDir)
        results.setSourceDir(sourceDir)

        args = ""
        if start:
            args += " --start=%s" % start
        
        if testConf:
            args += " --continue %s" % testConf
        else:
            args += " --continue"        

        if self.is64(arch):
            args += " --x64"
        if buildConfiguration == "cpp11":
            args += " --c++11"
        if buildConfiguration == "debug":
            args += " --mode=debug"
        else:
            args += " --mode=release"

        args += " --fast"

        if lang == "java" and not self.checkJavaSupport(arch, buildConfiguration, output):
            return False
                
        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, sourceArchive)

        command = "%s %s" % (self.runScriptCommand("allDemos.py", compiler, arch, buildConfiguration, lang), args)

        if self._verbose:
            print(command)
            
        trace("", report)
        trace("*** [%s] running %s demos (%s/%s/%s)" % (index, lang, compiler, arch, buildConfiguration), report)
        spawnAndWatch(command, env, lambda line: results.filter(line))
        return True

    def filterDemos(self, compiler, arch, conf, lang, testConfigs):
        if compiler in ["VC90"] and arch in ["x64"]:
            return False
        if conf == "silverlight": # Silverlight demos need manual intervention
            return False
        if lang == "php":
            return False

        testConfigs.append(self._demoConfiguration)
        return True

    def run(self, startTests, startDemos):

        testSummary = Summary("tests")
        demoSummary = Summary("demos")

        try:
            if self._archive and not self._skipTests and not startDemos:
                trace("\n******", report)
                trace("****** Building and running tests", report)
                trace("******", report)
                self.buildAndRun(self.buildTests, 
                                 self.runTests, 
                                 self.filterTests, 
                                 self.extractSourceArchive,
                                 startTests, 
                                 testSummary)

            if self._demoArchive and not self._skipDemos:
                trace("\n******", report)
                trace("****** Building and running demos", report)
                trace("******", report)

                self.buildAndRun(self.buildDemos, 
                                 self.runDemos, 
                                 self.filterDemos, 
                                 self.extractDemoArchive, 
                                 startDemos, 
                                 demoSummary)

        except KeyboardInterrupt:
            pass

        trace("\n\nSummary:", report)
        testSummary.printSummary()
        demoSummary.printSummary()

class Darwin(Platform):
    
    def __init__(self, distDir):
        Platform.__init__(self, distDir)

    def getSupportedCompilers(self):
        return ["clang"]

    def getSupportedConfigurations(self, compiler, arch):
        return ["default", "cpp11"]

    def getSupportedLanguages(self):
        return ["cpp", "java", "py"]
        
    def getSupportedArchitectures(self):
        return ["x64", "x86"]
        
    def isDarwin(self):
        return True
        
    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = Platform.getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist)
        if lang == "java":
            #
            # Set DYLD_LIBRARY_PATH for Berkeley DB
            #
            prependPathToEnvironVar(env, "DYLD_LIBRARY_PATH", os.path.join(self._iceHome, "lib"))
        return env

    def getTestConfigurations(self, filterArg, rfilterArg):
        f = ""
        f += " --filter=\"%s\"" % filterArg if filterArg else ""
        f += " --rfilter=\"%s\"" % rfilterArg if rfilterArg else ""
        configs = Platform.getTestConfigurations(self, filterArg, rfilterArg)
        configs.append(TestConfiguration("cpp11", "" + f, configs = ["cpp11"]))
        return configs

class Linux(Platform):
    
    def __init__(self, distDir):
        Platform.__init__(self, distDir)
        
        #
        # Init Linux distribution attributes from lsb_release
        #
        p = subprocess.Popen("lsb_release -i", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            sys.exit(1)
            
        self._distribution = re.sub("Distributor ID:", "", p.stdout.readline().decode('UTF-8')).strip()
        
        p = subprocess.Popen("lsb_release -r", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            sys.exit(1)
        self._release = re.sub("Release:", "", p.stdout.readline().decode('UTF-8')).strip()
        
        p = subprocess.Popen("uname -m", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            sys.exit(1)
        self._machine = p.stdout.readline().decode('UTF-8').strip()

    def isLinux(self):
        return True
        
    def isUbuntu(self):
        return self._distribution == "Ubuntu"
        
    def isRhel(self):
        return self._distribution.find("RedHat") != -1 or self._distribution.find("Amazon") != -1
        
    def isSles(self):
        return self._distribution == "SUSE LINUX"
        
    def getDistribution(self):
        return self._distribution
        
    def getRelease(self):
        return self._release
        
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
            return ["x64"]
        else:
            return ["x86"]
            
    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = Platform.getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist)
        if lang == "java":
            #
            # Set Berkeley DB classpath
            #
            prependPathToEnvironVar(env, "CLASSPATH", "/usr/share/java/db-5.3.21.jar")
                
            #
            # Set LD_LIBRARY_PATH for Berkeley DB
            #
            if self.isUbuntu():
                prependPathToEnvironVar(env, "LD_LIBRARY_PATH", \
                                    "/usr/lib/i386-linux-gnu/" if arch == "x86" else "/usr/lib/x86_64-linux-gnu/")
        return env

class Solaris(Platform):

    def __init__(self, distDir):
        Platform.__init__(self, distDir)
        self._parallelJobs = 40

    def isSolaris(self):
        return True
        
    def getDistribution(self):
        return self._distribution
        
    def getRelease(self):
        return self._release
        
    def getSupportedLanguages(self):
        return ["cpp", "java"]
        
    def getSupportedCompilers(self):
        return ["CC"]
        
    def getSupportedArchitectures(self):
        return ["sparcv9", "sparc"]
            
    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = Platform.getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist)
        if lang == "java":
            #
            # Set Berkeley DB classpath
            #
            prependPathToEnvironVar(env, "CLASSPATH", "/usr/share/java/db-5.3.21.jar")
        return env
                
    def makeCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        command = "gmake"
        if self._parallelJobs:
            command += " -j%s" % self._parallelJobs
        return command
        
    def makeCleanCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "gmake clean"

class Windows(Platform):

    def __init__(self, distDir):
        Platform.__init__(self, distDir)
        self._archive = os.path.join(distDir, "Ice-%s.zip" % version)
        self._demoArchive = os.path.join(distDir, "Ice-%s-demos.zip" % version)
        self._demoScriptsArchive = os.path.join(distDir, "Ice-%s-demo-scripts.zip" % version)
    
    def getTestConfigurations(self, filter, rfilter):
        f = ""
        f += " --filter=\"%s\"" % filterArg if filterArg else ""
        f += " --rfilter=\"%s\"" % rfilterArg if rfilterArg else ""
        configs = Platform.getTestConfigurations(self, filterArg, rfilterArg)
        #
        # Run non default configurations without --all
        #
        for c in ["debug", "java1.6", "silverlight"]:
            configs.append(TestConfiguration(c, "" + f, configs = [c]))
        return configs
                
    def canonicalArch(self, arch):
        if arch == "x64":
            arch = "amd64"
        return arch
        
    def makeSilverlightCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && devenv testsl.sln /build" % (BuildUtils.getVcVarsAll(compiler), 
                                                                    self.canonicalArch(arch), buildDir)
        
    def makeDemosCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        bConf = "Debug" if buildConfiguration == "debug" else "Release"
        bArch = ".NET" if lang in ["cs", "vb"] else "Win32" if arch == "x86" else "Win64"
            
        return '"%s" %s  && cd %s && devenv demo.sln /build %s /projectconfig "%s|%s"' % \
                (BuildUtils.getVcVarsAll(compiler), self.canonicalArch(arch), buildDir, bConf, bConf, bArch)

    def makeCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && nmake /f Makefile.mak" % (BuildUtils.getVcVarsAll(compiler), 
                                                                 self.canonicalArch(arch), buildDir)

    def makeCleanCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && nmake /f Makefile.mak clean" % (BuildUtils.getVcVarsAll(compiler), 
                                                                       self.canonicalArch(arch), buildDir)
    
    def runScriptCommand(self, script, compiler, arch, buildConfiguration, lang):
        if lang != "py":
            python = sys.executable
        else:
            pythonHome = BuildUtils.getPythonHome(lang)
            python = os.path.join(pythonHome, "python") if pythonHome else "python"
        return "\"%s\" %s && %s %s" % (BuildUtils.getVcVarsAll(compiler), self.canonicalArch(arch), python, script)

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

    def getSupportedConfigurations(self, compiler, arch):        
        buildConfigurations = ["default"]
        if compiler == "VC100":
            buildConfigurations.append("debug")
            buildConfigurations.append("silverlight")
            buildConfigurations.append("java1.6")
        elif compiler == "VC110":
            buildConfigurations.append("debug")
            buildConfigurations.append("java1.6")
            buildConfigurations.append("silverlight")
            if self.isWindows8():
                buildConfigurations.append("winrt")
        return buildConfigurations

    #
    # Return just the compilers that are installed in the system.
    #
    def getSupportedCompilers(self):
        compilers = []
        if BuildUtils.getVcVarsAll("VC90"):
            compilers.append("VC90")
        if BuildUtils.getVcVarsAll("VC100"):
            compilers.append("VC100")
        if BuildUtils.getVcVarsAll("VC110"):
            compilers.append("VC110")
        return compilers
        
    def getSupportedLanguages(self):
        return ["cpp", "cs", "java", "php", "py", "rb", "vb"]
        
    def getSupportedArchitectures(self):
        archs = ["x86"]
        if os.environ.get("PROCESSOR_ARCHITECTURE", "") == "AMD64" or \
           os.environ.get("PROCESSOR_ARCHITEW6432", "") == "AMD64":
            archs.append("x64")
        return archs

    def getJavaHome(self, arch, buildConfiguration):
        return BuildUtils.getJavaHome(arch, buildConfiguration)

    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = Platform.getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist)

        if lang == "cs" and compiler == "VC90":
            env["COMPACT"] = "yes"
        return env
                
#
# Program usage.
#
def usage():
    print("")
    print("Options:")
    print("  --help                             Show this message.")
    print("  --verbose                          Be verbose.")
    print("  --build-dir=<path>                 Directory to build the sources")
    print("  --ice-home=<path>                  Use the binary distribution from the given path")
    print("  --parallel-jobs=<NUMBER>           Makefile -j option to builds")
    print("  --filter=<regex>                   Run all the demos/tests that match the given regex")
    print("  --rfilter=<regex>                  Run all the demos/tests that do not match the given regex")
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
    print("  --print-configurations             Print build and run configurations")
    print("  --tests-driver=<path>              Run tests with configurations from test driver file")
    print("                                     if --test-driver isn't used we run allTests.py with --all option")
    print("                                     the test driver file contains a configuration per line using :")
    print("                                     as delimiter, each line has the form of \"name:options:languages\"")
    print("                                     if languages part isn't present the configuration apply to all")
    print("                                     languages otherwise apply just to the given languages.")
    print("  --start-with-test=bi/ti            Resume running the tests at the given build/test index.")
    print("  --start-with-demo=bi/di            Resume running the demos at the given build/demo index.")
    print("")
    

distDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

platformName = "linux" if sys.platform.startswith("linux") else sys.platform
if platformName == "win32":
    platform = Windows(distDir)
elif platformName == "sunos5":
   platform = Solaris(distDir)
elif platformName == "darwin":
    platform = Darwin(distDir)
elif platformName == "linux":
    platform = Linux(distDir)
else:
    print("Unknown platform: %s" % sys.platform)
    sys.exit(1)

args = None
opts = None
try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "verbose", "skip-tests", "skip-demos", "ice-home=", \
                                                  "parallel-jobs=", "filter-languages=", "filter-compilers=", \
                                                  "filter-archs=", "filter-configurations=", "rfilter-languages=", \
                                                  "rfilter-compilers=", "rfilter-archs=", "rfilter-configurations", \
                                                  "print-configurations", "test-driver=",
                                                  "filter=","rfilter=", "skip-build",
                                                  "start-with-demo=", "start-with-test="])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

if args:
    usage()
    sys.exit(1)

printConfigurations = False
testDriver = None
filterArg = None
rfilterArg = None
startTests = None
startDemos= None
for o, a in opts:
    if o == "--help":
        usage()
        sys.exit(0)
    if o == "--ice-home":
        platform._iceHome = os.path.abspath(os.path.expanduser(a))
    elif o == "--verbose":
        platform._verbose = True
    elif o == "--filter":
        filterArg = a
    elif o == "--rfilter":
        rfilterArg = a
    elif o == "--filter-languages":
        platform._languages.append(a)
    elif o == "--filter-compilers":
        platform._compilers.append(a)
    elif o == "--filter-archs":
        if a == "x86_64":
            a = "x64"
        platform._archs.append(a)
    elif o == "--filter-configurations":
        platform._configurations.append(a)
    elif o == "--rfilter-languages":
        platform._rlanguages.append(a)
    elif o == "--rfilter-compilers":
        platform._rcompilers.append(a)
    elif o == "--rfilter-archs":
        if a == "x86_64":
            a = "x64"
        platform._rarchs.append(a)
    elif o == "--rfilter-configurations":
        platform._rconfigurations.append(a)
    elif o == "--skip-build":
        platform._skipBuild = True
    elif o == "--skip-tests":
        platform._skipTests = True
    elif o == "--skip-demos":
        platform._skipDemos = True
    elif o == "--parallel-jobs":
        platform._parallelJobs = a
    elif o == "--print-configurations":
        printConfigurations = True
    elif o == "--test-driver":
        testDriver = a
    elif o == "--start-with-test":
        startTests = a
    elif o == "--start-with-demo":
        startDemos = a

#
# If there's a test driver file, parse it and setup test configurations
#
if testDriver == None:
    if os.path.exists(os.path.join(distDir, "tests-driver-%s.cnf" % platformName)):
        testDriver = os.path.abspath(os.path.join(distDir, "tests-driver-%s.cnf" % platformName))
elif not os.path.exists(testDriver):
    print(sys.argv[0] + ": test driver file not found in `%s'" % testDriver)
    usage()
    sys.exit(1)
    
testConfigurations = []
if not testDriver:
    testConfigurations = platform.getTestConfigurations(filterArg, rfilterArg)
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
            print(sys.argv[0] + ": invalid buildConfiguration line `%s' missing ':' delimiter" % line)
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
        # Remove any empty entries left by split
        #
        tmp = []
        for lang in languages:
            if lang:
                tmp.append(lang)
        languages = tmp        
        testConfigurations.append(TestConfiguration(name, options, languages = languages))

platform._testConfigurations = testConfigurations
if filterArg:
    platform._demoConfiguration = "--filter=\"%s\"" % filterArg
elif rfilterArg:
    platform._demoConfiguration = "--rfilter=\"%s\"" % rfilterArg

if printConfigurations:
    platform.printConfigurationSummary()
    sys.exit(0)

platform.validateConfiguration()

buildDir = os.path.join(distDir, "build")
if not os.path.exists(os.path.join(buildDir)):
    os.mkdir(os.path.join(buildDir))
output = open(os.path.join(buildDir, "output.txt"), "w")
report = open(os.path.join(buildDir, "report.txt"), "w")

platform.printConfigurationSummary(report)
platform.run(startTests, startDemos)

output.close()
report.close()
