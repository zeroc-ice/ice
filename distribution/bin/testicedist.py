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

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, zipfile, time, datetime, glob

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "lib")))

import BuildUtils, FixUtil

version = "3.6b"

#
# Substitute development PublicKeyToken by release PublicKeyToken in Silverlight projects
#
projectSubstituteExprs = [(re.compile(re.escape("PublicKeyToken=1f998c50fec78381")), "PublicKeyToken=cdd571ade22f2f16")]

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
        self._sourceDir = sourceDir + os.sep

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

            self._current = int(m.group(1).strip())
            if self._total == 0:
                self._total = int(m.group(2).strip())
            self._currentTest = m.group(3).strip()

            if self._currentTest.startswith(self._sourceDir):
                self._currentTest = self._currentTest[len(self._sourceDir):]

            trace("[%d/%d] %s" % (self._current, self._total, self._currentTest), report, False)
            return

        # Match configuration line
        m = self.matchConfiguration.match(line)
        if m:
            c = m.group(1).strip()
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
                    trace("failed! (%d/%d status = %s)" % (self._current, self._total, m.group(2).strip()), report)
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

    for k,v in env.items():
        if k in os.environ and v == os.environ[k]:
            continue
        output.write("%s = %s\n" % (k, v))
    output.write(command)
    output.write('\n')
    output.flush()

    p = subprocess.Popen(command, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT, bufsize = 0, env = env)

    while(True):

        line = p.stdout.readline()
        if not line:
            # The process terminated
            return p.wait() == 0

        if type(line) != str:
            line = line.decode()


        line = line.rstrip()
        filterFunc(line)

        output.write("%s\n" % line)
        output.flush()

class Interpreter:

    def __init__(self, home, requireHome, exe, subdir, versionArg, versionFn = None):

        self.home = home
        self.version = None
        self.exe = exe

        if requireHome and not home:
            self.cmd = None
        elif home:
            self.cmd = os.path.join(home, subdir, exe)
        else:
            self.cmd = exe

        self._versionArg = versionArg
        self._versionFn = versionFn or (lambda out: out)

    def init(self):
        def check_output(cmd):
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            output = ""
            for line in p.stdout.readlines():
                output += line.decode('UTF-8').strip() + "\n"
            p.wait()
            return output.strip()

        if self.cmd:
            try:
                self.version = self._versionFn(check_output([self.cmd, self._versionArg]).splitlines()[0].strip())
            except:
                pass

        return self

    def isMissing(self):
        return not self.version

    def checkMissing(self, returnEvenIfMissing):
        return self if returnEvenIfMissing or self.version else None

    def __str__(self):
        if not self.version:
            return "*** missing %s ***" % self.exe
        else:
            return "%s from %s" % (self.version, self.home if self.home else "PATH")

class JavaInterpreter(Interpreter):
    def __init__(self, home):
        Interpreter.__init__(self, home, True, "java", "bin", "-version",
                             lambda out: out.replace(" version", "").replace("\"", ""))

class PythonInterpreter(Interpreter):
    def __init__(self, home = None, requireHome = False):
        Interpreter.__init__(self, home, requireHome, "python", "", "-V")

class RubyInterpreter(Interpreter):
    def __init__(self, home = None, requireHome = False):
        Interpreter.__init__(self, home, requireHome, "ruby", "", "-v", lambda out: " ".join(out.split()[0:2]))

class NodeJSInterpreter(Interpreter):
    def __init__(self, home = None, requireHome = False, exe = "node"):
        Interpreter.__init__(self, home, requireHome, exe, "", "-v", lambda out: "%s %s" % (exe, out))

class PhpInterpreter(Interpreter):
    def __init__(self, home = None, requireHome = False):
        Interpreter.__init__(self, home, requireHome, "php", "", "-v", lambda out: " ".join(out.split()[0:2]))

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
        self._skipMissingInterpreters = False
        self._parallelJobs = 8
        self._testConfigurations = []
        self._demoConfiguration = ""
        self._interpreters = {}
        self._sourceDir = None
        self._demoDir = None

    def initInterpreters(self):
        if self._interpreters:
            return

        languages = set()
        defaultCompiler = self.getDefaultCompiler()
        for arch in self.getArchitectures():
            for conf in self.getConfigurations(defaultCompiler):
                languages.update(self.getLanguageMappings(defaultCompiler, arch, conf, True))

        self._interpreters = {}
        for (lang, value) in self._supportedInterpreters.items():
            if not lang in languages:
                continue # lang filtered out or not supported

            if isinstance(value, Interpreter):
                self._interpreters[lang] = value.init()
            else:
                self._interpreters[lang] = {}
                for (config, value2) in value.items():
                    if config and (not config in self.getConfigurations(defaultCompiler)):
                        continue # config filered out or not supported

                    if isinstance(value2, Interpreter):
                        self._interpreters[lang][config] = value2.init()
                    else:
                        self._interpreters[lang][config] = {}
                        for (arch, interpreter) in value2.items():
                            if arch and (not arch in self.getArchitectures()):
                                continue # arch filered out or not supported
                            self._interpreters[lang][config][arch] = interpreter.init()

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
            print("Ice source archive not found: `%s'" % self._archive)
            print(": you must run testicedist.py from the directory created by makedist.py")
            sys.exit(1)

        if self._demoArchive and not os.path.exists(self._demoArchive):
            print("Ice demo source archive not found: `%s'" % self._demoArchive)
            print(": you must run testicedist.py from the directory created by makedist.py")
            sys.exit(1)

        if self._demoScriptsArchive and not os.path.exists(self._demoScriptsArchive):
            print("Ice demo scripts source archive not found: `%s'" % self._demoScriptsArchive)
            print(": you must run testicedist.py from the directory created by makedist.py")
            sys.exit(1)

        #
        # Ensure all the require interpreters are installed.
        #
        missing = set()
        for arch in self.getArchitectures():
            for conf in self.getConfigurations(self.getDefaultCompiler()):
                for lang in self.getLanguageMappings(self.getDefaultCompiler(), arch, conf, True):
                    interpreter = self.getInterpreter(arch, conf, lang, True)
                    if interpreter and interpreter.isMissing():
                        missing.add("%s interpreter for %s/%s" % (interpreter.exe, conf, arch))

        if len(missing) > 0:
            if self._skipMissingInterpreters:
                print("warning: missing interpreters:")
            else:
                print("error: missing interpreters (use --skip-missing-interpreters to skip missing interpreters):")
            for m in missing:
                print(" " + m)
            if not self._skipMissingInterpreters:
                sys.exit(1)
            print("")

    def printConfigurationSummary(self, f = None):
        trace("Using `%s' path as build directory" % self._buildDir, f)
        trace("Using Ice installation from `%s'" % self._iceHome, f)
        trace("Using `%s' source archive" % self._archive, f)
        trace("Using `%s' demo archive" % self._demoArchive, f)
        trace("", f)
        trace("Compilers: %s" % self.getCompilers(), f)
        trace("Architectures: %s" % self.getArchitectures(), f)
        buildConfigs = set()
        buildLanguages = set()
        for comp in self.getCompilers():
            for arch in self.getArchitectures():
                for c in self.getConfigurations(comp):
                    buildConfigs.add(c)
                    buildLanguages.update(self.getLanguageMappings(comp, arch, c))
        trace("Languages: %s" % list(buildLanguages), f)
        trace("Build configurations: %s" % list(buildConfigs), f)

        trace("\nInterpreters:", f)
        self.initInterpreters()
        for lang in sorted(self._interpreters.keys()):
            if isinstance(self._interpreters[lang], Interpreter):
                trace("- %s: %s " % (lang, self._interpreters[lang]), f)
                continue

            trace("- %s:" % lang, f)
            for conf in sorted(self._interpreters[lang].keys()):
                if isinstance(self._interpreters[lang][conf], Interpreter):
                    trace(" %s = %s" % (conf if conf else "default", self._interpreters[lang][conf]), f)
                    continue

                for arch in sorted(self._interpreters[lang][conf].keys()):
                    if conf:
                        trace(" %s/%s = %s" % (conf, arch, self._interpreters[lang][conf][arch]), f)
                    else:
                        trace(" %s = %s" % (arch, self._interpreters[lang][conf][arch]), f)


        if not self._skipTests:
            trace("\nTest configurations:", f)
            (_, total, runnableConfigs) = self.getBuildAndRunConfigs(self.filterTests)
            count = 1
            for comp in filterRC(runnableConfigs, self.getSupportedCompilers()):
                for arch in filterRC(runnableConfigs, self.getSupportedArchitectures(), comp):
                    for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp), comp, arch):
                        for lang in filterRC(runnableConfigs, self.getSupportedLanguages(), comp, arch, conf):
                            for c in runnableConfigs[comp][arch][conf][lang]:
                                options = "" if c.options == "" else " " + c.name
                                cfg = "" if conf == "default" else "/" + conf
                                trace("- [%d] %s%s tests (%s/%s%s)" % (count, lang, options, comp, arch, cfg), f)
                                count += 1

        if not self._skipDemos:
            trace("\nDemo configurations:", f)
            (_, total, runnableConfigs) = self.getBuildAndRunConfigs(self.filterDemos)
            count = 1
            for comp in filterRC(runnableConfigs, self.getSupportedCompilers()):
                for arch in filterRC(runnableConfigs, self.getSupportedArchitectures(), comp):
                    for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp), comp, arch):
                        for lang in filterRC(runnableConfigs, self.getSupportedLanguages(), comp, arch, conf):
                            cfg = "" if conf == "default" else "/" + conf
                            trace("- [%d] %s %s demos (%s/%s%s)" % (count, lang, conf, comp, arch, cfg), f)
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
        defaultCompiler = self.getDefaultCompiler()
        otherArchs = self.getSupportedArchitectures()
        if defaultArch:
            otherArchs.remove(defaultArch)

        configs = []
        # Add --all tests for default architecture, no --all for other architectures
        if defaultArch:
            configs.append(TestConfiguration("all", "--all" + f, archs = [defaultArch] , configs = ["default"]))
        if(otherArchs):
            configs.append(TestConfiguration("default", "" + f, archs = otherArchs, configs = ["default"]))

        # Add cross tests
        if defaultArch and defaultCompiler:
            langs = set(["cpp", "java", "cs"]) & set(self.getLanguageMappings(defaultCompiler, defaultArch, "default"))
            for l1 in langs:
                for l2 in langs:
                    if l1 == l2:
                        continue

                    configs.append(TestConfiguration("cross-tcp",
                                                     "--cross=%s%s" % (l2,f),
                                                     configs = ["default"],
                                                     compilers = [defaultCompiler],
                                                     archs = [defaultArch],
                                                     languages = [l1]))
                    if not filterArg and not rfilterArg:
                        configs.append(TestConfiguration("cross-ssl",
                                                         "--cross=%s --protocol=ssl --filter=\"Ice/operations\"" % l2 ,
                                                         configs = ["default"],
                                                         compilers = [defaultCompiler],
                                                         archs = [defaultArch],
                                                         languages = [l1]))

        #
        # Add test configuration for all other non-default
        # configurations (except for WinRT where we can't run the
        # tests from the command line yet).
        #
        buildConfigs = set()
        for comp in self.getCompilers():
            for arch in self.getArchitectures():
                for c in self.getConfigurations(comp):
                    if c != "default" and c != "winrt":
                        buildConfigs.add(c)
        for c in buildConfigs:
            configs.append(TestConfiguration(c, f, configs = [c]))

        return configs

    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = os.environ.copy()

        if useBinDist:
            env["USE_BIN_DIST"] = "yes"

        if buildConfiguration != "debug":
            env["OPTIMIZE"] = "yes"

        if buildConfiguration == "winrt":
            env["WINRT"] = "yes"

        if self._iceHome:
            env["ICE_HOME"] = self._iceHome

        if buildConfiguration == "cpp11":
            env["CPP11"] = "yes"

        if buildConfiguration == "no-cpp11":
            env["CPP11"] = "no"

        if self.isLinux():
            env["LP64"] = "yes" if self.is64(arch) else "no"

        if self.requiresInterpreter(lang):
            #
            # If the interpreter isn't installed in the default location,
            # add the directory where the command is located to the
            # PATH. If the interpreter is missing, return None to skip
            # the tests/demos with this interpreter.
            #
            interpreter = self.getInterpreter(arch, buildConfiguration, lang)
            if not interpreter:
                return None
            if interpreter.home and interpreter.cmd:
                prependPathToEnvironVar(env, "PATH", os.path.dirname(interpreter.cmd))

            if lang == "java":
                if interpreter.home:
                    env["JAVA_HOME"] = interpreter.home
                if os.path.exists(os.path.join(self._iceHome, "lib", "db.jar")):
                    prependPathToEnvironVar(env, "CLASSPATH", os.path.join(self._iceHome, "lib", "db.jar"))

        return env

    def is64(self, arch):
        return arch == "x64" or arch == "sparcv9"

    def isWindows(self):
        return False

    def isLinux(self):
        return False

    def getCompilers(self):
        return list(filter(lambda x: include(x, self._compilers, self._rcompilers), self.getSupportedCompilers()))

    def getArchitectures(self):
        return list(filter(lambda x: include(x, self._archs, self._rarchs), self.getSupportedArchitectures()))

    def getLanguages(self):
        return list(filter(lambda x: include(x, self._languages, self._rlanguages), self.getSupportedLanguages()))

    def getConfigurations(self, compiler):
        return list(filter(lambda x: include(x, self._configurations, self._rconfigurations),
                           self.getSupportedConfigurations(compiler)))

    def getDefaultArchitecture(self):
        # Default architecture is by default the first supported architecture
        for a in self.getSupportedArchitectures():
            return a if include(a, self._archs, self._rarchs) else None

    def getDefaultCompiler(self):
        # Default compiler is by default the first supported compiler
        for a in self.getSupportedCompilers():
            return a if include(a, self._compilers, self._rcompilers) else None

    def requiresInterpreter(self, lang):
        self.initInterpreters()
        return lang in self._interpreters

    def getInterpreter(self, arch, conf, lang, returnEvenIfMissing = False):
        self.initInterpreters()
        if lang in self._interpreters:
            if isinstance(self._interpreters[lang], Interpreter):
                return self._interpreters[lang].checkMissing(returnEvenIfMissing)

            conf = conf if conf in self._interpreters[lang] else ""
            if conf in self._interpreters[lang]:
                if isinstance(self._interpreters[lang][conf], Interpreter):
                    return self._interpreters[lang][conf].checkMissing(returnEvenIfMissing)

                arch = arch if arch in self._interpreters[lang][conf] else ""
                if arch in self._interpreters[lang][conf]:
                   return self._interpreters[lang][conf][arch].checkMissing(returnEvenIfMissing)

        return None

    def getLanguageMappings(self, compiler, arch, conf, returnEvenIfMissingInterpreter = False):

        isDefaultCompilerAndArch = arch == self.getDefaultArchitecture() and compiler == self.getDefaultCompiler()

        languages = None
        if conf == "java1.8":
            languages = ["java"] if isDefaultCompilerAndArch else []
        elif conf == "silverlight":
            languages = ["cs"] if isDefaultCompilerAndArch else []
        elif conf == "default":
            if isDefaultCompilerAndArch:
                languages = self.getSupportedLanguages()
            elif self.isWindows() and compiler == self.getDefaultCompiler():
                languages = ["cpp", "py", "rb", "php"] # On Windows, test C++/Python/Ruby/PHP with supported archs
            else:
                languages = ["cpp"] # Only test C++ if non-default compiler or architecture
        else:
            languages = ["cpp"] # Only test C++ for other configurations (winrt, cpp11, etc).

        #
        # On Windows, only test php with VC110/x86, the VC110 runtime
        # might not be installed otherwise.
        #
        if self.isWindows() and "php" in languages and (compiler != "VC110" or arch != "x86"):
            languages.remove("php")

        languages = filter(lambda x: include(x, self._languages, self._rlanguages), languages)

        #
        # If allowed to skip missing interpreters, filter out
        # languages for which the interpreter is missing.
        #
        if not returnEvenIfMissingInterpreter and self._skipMissingInterpreters:
            return [l for l in languages if not self.requiresInterpreter(l) or self.getInterpreter(arch, conf, l)]
        else:
            return languages

    def getSupportedConfigurations(self, compiler):
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

    def getNodeCmd(self):
        return "node"

    def getBuildAndRunConfigs(self, filterFn):
        buildTotal = 0
        runTotal = 0
        runnableConfigs = {}
        for compiler in self.getCompilers():
            runnableConfigs[compiler] = {}
            for arch in self.getArchitectures():
                runnableConfigs[compiler][arch] = {}
                for conf in self.getConfigurations(compiler):
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
                    for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp), comp, arch):
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
                for conf in filterRC(runnableConfigs, self.getSupportedConfigurations(comp), comp, arch):
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

                for root, dirnames, filenames in os.walk(os.path.join(self._sourceDir, "cs", "test")):
                    for f in filenames:
                        if fnmatch.fnmatch(f, "*.csproj"):
                            FixUtil.fileMatchAndReplace(os.path.join(root, f), projectSubstituteExprs, False)

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

        output.write("*** [%s] building %s tests (%s/%s/%s)\n" % (index, lang, compiler, arch, buildConfiguration))
        output.flush()

        if lang == "rb":
            trace("ok", report)
            return True

        buildDir = None
        if lang == "java":
            buildDir = os.path.join(self._sourceDir, lang)
        else:
            buildDir = os.path.join(self._sourceDir, lang, "test")

        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, True)
        if not env:
            trace("skipped (no interpreter)", report)
            return True

        commands = []
        if buildConfiguration == "silverlight":
            commands.append(self.makeSilverlightCommand(compiler, arch, buildConfiguration, lang, buildDir))

        if lang == "java":
            commands.append("%s :test:assemble" % ("gradlew" if self.isWindows() else "./gradlew"))
        else:
            commands.append(self.makeCommand(compiler, arch, buildConfiguration, lang, buildDir))

        os.chdir(buildDir)

        status = True
        for command in commands:
            if self._verbose:
                print(command)

            if not spawnAndWatch(command, env, filterBuildOutput):
                trace("failed!", report)
                status = False
                break

        if status:
            trace("ok", report)
        return status

    def runTests(self, compiler, arch, buildConfiguration, lang, testConf, results, start, index):
        os.chdir(os.path.join(self._sourceDir, lang))
        results.setSourceDir(os.path.join(self._sourceDir, lang))

        args = "--continue %s" % testConf.options

        if start:
            args += " --start=%s" % start

        if self.isWindows() or self.isLinux():
            args += " --x64" if self.is64(arch) else " --x86"

        if self.isLinux() and buildConfiguration == "cpp11":
            args += " --c++11"

        if buildConfiguration == "silverlight":
            args += " --silverlight"

        trace("", report)
        options = ""
        if testConf.options != "":
            options = " with " + testConf.options

        trace("*** [%s] running %s tests%s (%s/%s/%s)" % (index, lang, options, compiler, arch, buildConfiguration),
              report)

        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, True)
        if not env:
            return False

        interpreter = self.getInterpreter(arch, buildConfiguration, lang)
        if interpreter:
            trace("Using %s" % interpreter, report)

        command = "%s %s" % (self.runScriptCommand("allTests.py", compiler, arch, buildConfiguration, lang), args)

        if self._verbose:
            print(command)
        spawnAndWatch(command, env, lambda line: results.filter(line))
        return True

    def filterTests(self, compiler, arch, conf, lang, testConfigs):
        if lang == "vb":
            return False

        for testConf in self._testConfigurations:
            if testConf.runWith(compiler, arch, conf, lang):
                testConfigs.append(testConf)

        return True

    def buildDemos(self, compiler, arch, buildConfiguration, lang, index):

        trace("*** [%s] building %s demos (%s/%s/%s)... " % (index, lang, compiler, arch, buildConfiguration),
              report, False)

        output.write("*** [%s] building %s demos (%s/%s/%s)\n" % (index, lang, compiler, arch, buildConfiguration))
        output.flush()

        if lang == "py" or lang == "rb":
            trace("ok", report)
            return True

        sourceArchive = False # TODO: Support?
        if sourceArchive:
            buildDir = os.path.join(self._sourceDir, lang, "demo")
        else:
            buildDir = os.path.join(self._demoDir, lang)

        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, sourceArchive)
        if not env:
            trace("skipped (no interpreter)", report)
            return True

        if lang == "java":
            commands = ("gradlew build" if self.isWindows() else "./gradlew build")
        elif lang == "js":
            commands = "%s build.js" % self.getNodeCmd()
        else:
            commands = self.makeDemosCommand(compiler, arch, buildConfiguration, lang, buildDir)
        if type(commands) == str:
            commands = [commands]

        os.chdir(buildDir)

        status = True
        for command in commands:
            if self._verbose:
                print(command)

            if not spawnAndWatch(command, env, filterBuildOutput):
                trace("failed!", report)
                status = False
                break

        if status:
            trace("ok", report)
        return status

    def runDemos(self, compiler, arch, buildConfiguration, lang, demoConf, results, start, index):

        sourceArchive = False # TODO: Support?
        sourceDir = None
        if sourceArchive:
            sourceDir = os.path.join(self._sourceDir, lang)
        else:
            sourceDir = os.path.join(self._demoDir, lang)
        os.chdir(sourceDir)
        results.setSourceDir(sourceDir)

        args = ""
        if start:
            args += " --start=%s" % start

        if demoConf:
            args += " --continue %s" % demoConf
        else:
            args += " --continue"

        if self.isWindows() or self.isLinux():
            args += " --x64" if self.is64(arch) else " --x86"

        if self.isLinux() and buildConfiguration == "cpp11":
            args += " --c++11"

        if buildConfiguration == "debug":
            args += " --mode=debug"
        else:
            args += " --mode=release"

        args += " --fast"

        options = ""
        if demoConf != "":
            options = " with " + demoConf

        trace("", report)
        trace("*** [%s] running %s demos%s (%s/%s/%s)" % (index, lang, options, compiler, arch, buildConfiguration),
              report)
        env = self.getPlatformEnvironment(compiler, arch, buildConfiguration, lang, sourceArchive)
        if not env:
            return False

        interpreter = self.getInterpreter(arch, buildConfiguration, lang)
        if interpreter:
            trace("Using %s" % interpreter, report)

        command = "%s %s" % (self.runScriptCommand("allDemos.py", compiler, arch, buildConfiguration, lang), args)

        if self._verbose:
            print(command)
        spawnAndWatch(command, env, lambda line: results.filter(line))
        return True

    def filterDemos(self, compiler, arch, conf, lang, demoConfigs):
        if conf == "silverlight": # Silverlight demos need manual intervention
            return False
        if conf == "winrt": # Winrt demos need manual intervention
            return False
        if lang == "php":
            return False

        demoConfigs.append(self._demoConfiguration)
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

        def getJavaHome(version):
            jvmDirs = glob.glob("/Library/Java/JavaVirtualMachines/jdk%s.*" % version)
            if len(jvmDirs) == 0:
                return None
            for jvm in jvmDirs[::-1]:
                if os.path.exists(os.path.join(jvm, "Contents", "Home", "bin", "java")):
                    return os.path.join(jvm, "Contents", "Home")
            return None

        self._supportedInterpreters = {
            "java": {
                "": JavaInterpreter(getJavaHome("1.7")),
                "java1.8": JavaInterpreter(getJavaHome("1.8")),
            },
            "py": PythonInterpreter(),
            "js": NodeJSInterpreter(),
            "rb": RubyInterpreter(),
            "php": PhpInterpreter()
        }

    def getSupportedCompilers(self):
        return ["clang"]

    def getSupportedConfigurations(self, compiler):
        return ["default", "no-cpp11", "java1.8"]

    def getSupportedLanguages(self):
        return ["cpp", "java", "py", "js"]

    def getSupportedArchitectures(self):
        return ["x64"]


class Linux(Platform):

    def __init__(self, distDir):
        Platform.__init__(self, distDir)

        self._distribution = None
        self._distributionVersion = None

        #
        # Try to figure out linux distribution by checking /etc/redhat-release or
        # /etc/issue. If not found, we try lsb_release.
        #
        for path in ["/etc/redhat-release", "/etc/issue"]:
            if not os.path.isfile(path):
                continue

            f = open(path, "r")
            issue = f.read()
            f.close()

            if issue.find("Red Hat") != -1:
                self._distribution = "RedHat"
                if issue.find("Santiago") != -1:
                    self._distributionVersion = 6
                elif issue.find("Maipo") != -1:
                    self._distributionVersion = 7
                break
            elif issue.find("Amazon Linux") != -1:
                self._distribution = "Amazon"
                break
            elif issue.find("CentOS") != -1:
                self._distribution = "CentOS"
                if issue.find("release 6") != -1:
                    self._distributionVersion = 6
                elif issue.find("release 7") != -1:
                    self._distributionVersion = 7
                break
            elif issue.find("SUSE Linux") != -1:
                self._distribution = "SUSE LINUX"
                if issue.find("Server 11") != -1:
                    self._distributionVersion = 11
                elif issue.find("Server 12") != -1:
                    self._distributionVersion = 12
            elif issue.find("Ubuntu") != -1:
                self._distribution = "Ubuntu"
                break

        if not self._distribution:
            p = subprocess.Popen("lsb_release -i", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if(p.wait() != 0):
                print("lsb_release failed:\n" + p.stdout.read().strip())
                sys.exit(1)
            self._distribution = re.sub("Distributor ID:", "", p.stdout.readline().decode('UTF-8')).strip()

            p = subprocess.Popen("lsb_release -s -r", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if(p.wait() != 0):
                print("lsb_release failed:\n" + p.stdout.read().strip())
                sys.exit(1)
            self._distributionVersion = p.stdout.readline().decode('UTF-8').strip()

        if self._distribution not in ["RedHat", "Amazon", "CentOS", "Ubuntu", "SUSE LINUX"]:
            print("Unknown distribution from lsb_release -i")
            print(self._distribution)
            sys.exit(1)

        p = subprocess.Popen("uname -m", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if(p.wait() != 0):
            print("uname failed:\n" + p.stdout.read().strip())
            sys.exit(1)
        self._machine = p.stdout.readline().decode('UTF-8').strip()

        def getJavaHome(version):
            jvmDir = None
            arch = self.getDefaultArchitecture()
            if self.isUbuntu():
                minorVersion = version.split('.')[1]
                jvmDir = "/usr/lib/jvm/java-%s-openjdk-%s" % (minorVersion, "amd64" if arch == "x64" else "i386")
                if not os.path.exists(jvmDir):
                    jvmDir = "/usr/lib/jvm/java-%s-oracle" % (minorVersion)
            else:
                jvmDir = "/usr/%s/jvm/java-%s.0" % ("lib64" if self.isSles() and arch == "x64" else "lib", version)
            return None if not os.path.exists(jvmDir) else jvmDir

        self._supportedInterpreters = {
            "java": {
                "": JavaInterpreter(getJavaHome("1.7")),
                "java1.8": JavaInterpreter(getJavaHome("1.8")),
            },
            "py": PythonInterpreter(),
            "rb": RubyInterpreter(),
            "php": PhpInterpreter(),
            "js": NodeJSInterpreter(exe = "nodejs" if self.isUbuntu() else "node")
        }

    def isLinux(self):
        return True

    def isDistribution(self, names, version):
        if self._distribution in names:
            if version:
                return version == self._distributionVersion
            else:
                return True

    def isUbuntu(self, version = None):
        return self.isDistribution(["Ubuntu"], version)

    def isRhel(self, version = None):
        return self.isDistribution(["RedHat", "CentOS"], version)

    def isSles(self, version = None):
        return self.isDistribution(["SUSE LINUX"], version)

    def getSupportedLanguages(self):
        languages = ["cpp", "java", "php", "py", "rb"]
        #
        # NodeJS modules are only installed for Ubuntu as other distributions
        # doesn't provide nodejs packages.
        #
        if self.isUbuntu():
            languages.append("js")
        return languages

    def getSupportedCompilers(self):
        return ["g++"]

    def getSupportedArchitectures(self):
        if self._machine == "x86_64":
            if self.isRhel(6) or self.isSles(11) or self.isUbuntu():
                return ["x64"] # Mono-arch Linux distribution
            else:
                return ["x64", "x86"] # Bi-arch Linux distribution
        else:
            return ["x86"]

    def getSupportedConfigurations(self, compiler):
        configurations = ["default"]
        # Only test C++11 if default compiler is recent
        if not self.isRhel(6) and not self.isSles(11):
            configurations.append("cpp11")
        # Only test Java 1.8 if the distribution provides it
        if not self.isUbuntu() and not self.isSles():
            configurations.append("java1.8")
        return configurations

class Windows(Platform):

    def __init__(self, distDir):
        Platform.__init__(self, distDir)
        self._archive = os.path.join(distDir, "Ice-%s.zip" % version)
        self._demoArchive = os.path.join(distDir, "Ice-%s-demos.zip" % version)
        self._demoScriptsArchive = os.path.join(distDir, "Ice-%s-demo-scripts.zip" % version)

        defaultArch = self.getDefaultArchitecture()
        self._supportedInterpreters = {
            "java": {
                "": JavaInterpreter(BuildUtils.getJavaHome(defaultArch, "1.7")),
                "java1.8": JavaInterpreter(BuildUtils.getJavaHome(defaultArch, "1.8")),
            },
            "py": {
                "": {
                    "x86": PythonInterpreter(BuildUtils.getPythonHome("x86"), True), # Require Python installation
                    "x64": PythonInterpreter(BuildUtils.getPythonHome("x64"), True)  # Require Python installation
                }
            },
            "rb": {
                "": {
                    "x86": RubyInterpreter(BuildUtils.getPythonHome("x86"), True), # Require Ruby installation
                    "x64": RubyInterpreter(BuildUtils.getPythonHome("x64"), True)  # Require Ruby installation
                }
            },
            "php": PhpInterpreter(), # Use PHP from the PATH
            "js": NodeJSInterpreter(BuildUtils.getNodeHome(defaultArch), False) # Use NodeJS install or from the PATH
        }

    def canonicalArch(self, arch):
        if arch == "x64":
            arch = "amd64"
        return arch

    def makeSilverlightCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        return "\"%s\" %s  && cd %s && devenv testsl.sln /build" % (BuildUtils.getVcVarsAll(compiler),
                                                                    self.canonicalArch(arch), buildDir)

    def makeDemosCommand(self, compiler, arch, buildConfiguration, lang, buildDir):
        bConf = "Debug" if buildConfiguration == "debug" else "Release"
        bArch = "Any CPU" if lang in ["cs", "vb"] else "Win32" if arch == "x86" else "x64"
        commands = []
        #
        # For VC120 demos we need first to upgrade the project files, the projects in the archive are for VC110,
        # that is only required for C++ projects.
        #
        if(compiler == "VC120" and lang == "cpp" and buildConfiguration != "winrt"
           and not os.path.exists(os.path.join(buildDir, "UpgradeLog.htm"))):
            commands.append('"%s" %s  && cd %s && devenv demo.sln /upgrade' % \
                            (BuildUtils.getVcVarsAll(compiler), self.canonicalArch(arch), buildDir))

        commands.append('"%s" %s  && cd %s && devenv %s /build "%s|%s"' % \
                        (BuildUtils.getVcVarsAll(compiler), self.canonicalArch(arch), buildDir, "demo.sln",
                        bConf, bArch))

        return commands

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
            pythonHome = BuildUtils.getPythonHome(arch)
            python = os.path.join(pythonHome, "python") if pythonHome else "python"
        return "\"%s\" %s && %s %s" % (BuildUtils.getVcVarsAll(compiler), self.canonicalArch(arch), python, script)

    def isWindows(self):
        return True

    def isWindows8(self):
        return sys.getwindowsversion()[0] == 6 and sys.getwindowsversion()[1] >= 2

    def getSupportedConfigurations(self, compiler):
        buildConfigurations = ["default", "debug", "java1.8"]
        if compiler == "VC120":
            buildConfigurations.append("silverlight")
            if self.isWindows8():
                buildConfigurations.append("winrt")
        return buildConfigurations

    def getSupportedCompilers(self):
        #
        # Return just the compilers that are installed in the system.
        #
        compilers = []
        if BuildUtils.getVcVarsAll("VC120"):
            compilers.append("VC120")
        if BuildUtils.getVcVarsAll("VC110"):
            compilers.append("VC110")
        return compilers

    def getSupportedLanguages(self):
        return ["cpp", "cs", "java", "js", "php", "py", "rb", "vb"]

    def getSupportedArchitectures(self):
        archs = []
        if os.environ.get("PROCESSOR_ARCHITECTURE", "") == "AMD64" or \
           os.environ.get("PROCESSOR_ARCHITEW6432", "") == "AMD64":
            archs.append("x64")
        archs.append("x86")
        return archs

    def getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist):
        env = Platform.getPlatformEnvironment(self, compiler, arch, buildConfiguration, lang, useBinDist)
        if (lang == "cs" or lang == "vb") and not self.is64(arch):
            p = os.path.join(BuildUtils.getIceHome(version), "bin")
            if compiler == "VC110":
                p = os.path.join(p, "vc110")
            p = os.path.join(p, "x64")
            prependPathToEnvironVar(env, "PATH",p);
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
    print("  --skip-build                       Don't build")
    print("  --skip-tests                       Don't build or run tests")
    print("  --skip-demos                       Don't build or run demos")
    print("  --skip-missing-interpreters        Skip missing interpreters")
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
    platformObj = Windows(distDir)
elif platformName == "sunos5":
   platformObj = Solaris(distDir)
elif platformName == "darwin":
    platformObj = Darwin(distDir)
elif platformName == "linux":
    platformObj = Linux(distDir)
else:
    print("Unknown platform: %s" % sys.platform)
    sys.exit(1)

args = None
opts = None
try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "verbose", "skip-tests", "skip-demos", "ice-home=", \
                                                  "parallel-jobs=", "filter-languages=", "filter-compilers=", \
                                                  "filter-archs=", "filter-configurations=", "rfilter-languages=", \
                                                  "rfilter-compilers=", "rfilter-archs=", "rfilter-configurations=", \
                                                  "print-configurations", "test-driver=",
                                                  "filter=","rfilter=", "skip-build", "skip-missing-interpreters",
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
        platformObj._iceHome = os.path.abspath(os.path.expanduser(a))
    elif o == "--verbose":
        platformObj._verbose = True
    elif o == "--filter":
        filterArg = a
    elif o == "--rfilter":
        rfilterArg = a
    elif o == "--filter-languages":
        platformObj._languages += a.split(',')
    elif o == "--filter-compilers":
        platformObj._compilers += a.split(',')
    elif o == "--filter-archs":
        platformObj._archs += [arch.replace("x86_64", "x64") for arch in a.split(',')]
    elif o == "--filter-configurations":
        platformObj._configurations += a.split(',')
    elif o == "--rfilter-languages":
        platformObj._rlanguages += a.split(',')
    elif o == "--rfilter-compilers":
        platformObj._rcompilers += a.split(',')
    elif o == "--rfilter-archs":
        platformObj._rarchs += [arch.replace("x86_64", "x64") for arch in a.split(',')]
    elif o == "--rfilter-configurations":
        platformObj._rconfigurations += a.split(',')
    elif o == "--skip-build":
        platformObj._skipBuild = True
    elif o == "--skip-tests":
        platformObj._skipTests = True
    elif o == "--skip-demos":
        platformObj._skipDemos = True
    elif o == "--skip-missing-interpreters":
        platformObj._skipMissingInterpreters = True
    elif o == "--parallel-jobs":
        platformObj._parallelJobs = a
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
    testConfigurations = platformObj.getTestConfigurations(filterArg, rfilterArg)
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

platformObj._testConfigurations = testConfigurations
if filterArg:
    platformObj._demoConfiguration = "--filter=\"%s\"" % filterArg
elif rfilterArg:
    platformObj._demoConfiguration = "--rfilter=\"%s\"" % rfilterArg

if printConfigurations:
    platformObj.printConfigurationSummary()
    sys.exit(0)

platformObj.validateConfiguration()

buildDir = os.path.join(distDir, "build")
if not os.path.exists(os.path.join(buildDir)):
    os.mkdir(os.path.join(buildDir))
output = open(os.path.join(buildDir, "output.txt"), "w")
report = open(os.path.join(buildDir, "report.txt"), "w")

platformObj.printConfigurationSummary(report)
platformObj.run(startTests, startDemos)

output.close()
report.close()
