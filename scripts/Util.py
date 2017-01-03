# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, runpy, getopt, traceback, types, threading, time, datetime, re, itertools, random, subprocess, shutil, copy

isPython2 = sys.version_info[0] == 2
if isPython2:
    import Queue as queue
    from StringIO import StringIO
else:
    import queue
    from io import StringIO

from collections import OrderedDict
import Expect

toplevel = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

def run(cmd, cwd=None, err=False):
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)
    out = p.stdout.read().decode('UTF-8').strip()
    if(not err and p.wait() != 0) or (err and p.wait() == 0) :
        raise RuntimeError(cmd + " failed:\n" + out)
    return out

def val(v, escapeQuotes=False, quoteValue=True):
    if type(v) == bool:
        return "1" if v else "0"
    elif type(v) == str:
        if not quoteValue or v.find(" ") < 0:
            return v
        elif escapeQuotes:
            return "\\\"{0}\\\"".format(v.replace("\\\"", "\\\\\\\""))
        else:
            return "\"{0}\"".format(v)
    else:
        return str(v)

def getIceSoVersion():
    config = open(os.path.join(toplevel, "cpp", "include", "IceUtil", "Config.h"), "r")
    intVersion = int(re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1))
    majorVersion = int(intVersion / 10000)
    minorVersion = int(intVersion / 100) - 100 * majorVersion
    patchVersion = intVersion % 100
    if patchVersion < 50:
        return '%d' % (majorVersion * 10 + minorVersion)
    elif patchVersion < 60:
        return '%da%d' % (majorVersion * 10 + minorVersion, patchVersion - 50)
    else:
        return '%db%d' % (majorVersion * 10 + minorVersion, patchVersion - 60)

class Platform:

    def __init__(self):
        self.parseBuildVariables({
            "supported-platforms" : ("supportedPlatforms", lambda s : s.split(" ")),
            "supported-configs" : ("supportedConfigs", lambda s : s.split(" "))
        })

    def parseBuildVariables(self, variables):
        # Run make to get the values of the given variables
        output = run('make print V="{0}"'.format(" ".join(variables.keys())), cwd = toplevel)
        for l in output.split("\n"):
            match = re.match(r'^.*:.*: (.*) = (.*)', l)
            if match and match.group(1):
                if match.group(1) in variables:
                    (varname, valuefn) = variables[match.group(1).strip()]
                    value = match.group(2).strip() or ""
                    setattr(self, varname, valuefn(value) if valuefn else value)

    def getFilters(self, config):
        return ([], [])

    def getDefaultBuildPlatform(self):
        return self.supportedPlatforms[0]

    def getDefaultBuildConfig(self):
        return self.supportedConfigs[0]

    def getBinSubDir(self, mapping, process, current):
        # Return the bin sub-directory for the given mapping,platform,config,
        # to be overriden by specializations
        return "bin"

    def getLibSubDir(self, mapping, process, current):
        # Return the bin sub-directory for the given mapping,platform,config,
        # to be overriden by specializations
        return "lib"

    def getBuildSubDir(self, name, current):
        # Return the build sub-directory, to be overriden by specializations
        return os.path.join("build", current.config.buildPlatform, current.config.buildConfig)

    def getLdPathEnvName(self):
        return "LD_LIBRARY_PATH"

    def getIceInstallDir(self, mapping, current):
        return os.environ.get("ICE_HOME", "/usr")

    def getSliceDir(self, iceDir):
        if iceDir.startswith("/usr"):
            return os.path.join(iceDir, "share", "ice", "slice")
        else:
            return os.path.join(iceDir, "slice")

    def getDefaultExe(self, name, config):
        if name == "icebox" and config.cpp11:
            name += "++11"
        return name

    def hasOpenSSL(self):
        # This is used by the IceSSL test suite to figure out how to setup certificates
        return False

    def canRun(self, mapping, current):
        return True

class Darwin(Platform):

    def getFilters(self, config):
        if config.buildPlatform in ["iphoneos", "iphonesimulator"]:
            return (["Ice/.*", "IceSSL/configuration"],
                    ["Ice/background",
                     "Ice/echo",
                     "Ice/faultTolerance",
                     "Ice/gc",
                     "Ice/library",
                     "Ice/logger",
                     "Ice/properties",
                     "Ice/plugin",
                     "Ice/stringConverter",
                     "Ice/threadPoolPriority",
                     "Ice/udp"])
        return Platform.getFilters(self, config)

    def getDefaultBuildPlatform(self):
        return "macosx"

    def getLdPathEnvName(self):
        return "DYLD_LIBRARY_PATH"

    def getIceInstallDir(self, mapping, current):
        return os.environ.get("ICE_HOME", "/usr/local")

class AIX(Platform):

    def hasOpenSSL(self):
        return True

class Linux(Platform):

    def __init__(self):
        Platform.__init__(self)
        self.parseBuildVariables({
            "linux_id" : ("linuxId", None),
            "build-platform" : ("buildPlatform", None),
            "foreign-platforms" : ("foreignPlatforms", lambda s : s.split(" ") if s else []),
        })
        self.multiArch = {}
        if self.linuxId in ["ubuntu", "debian"]:
            for p in [self.buildPlatform] + self.foreignPlatforms:
                self.multiArch[p] = run("dpkg-architecture -f -a{0} -qDEB_HOST_MULTIARCH 2> /dev/null".format(p))

    def hasOpenSSL(self):
        return True

    def getBinSubDir(self, mapping, process, current):
        if self.linuxId in ["ubuntu", "debian"] and current.config.buildPlatform in self.foreignPlatforms:
            return os.path.join("bin", self.multiArch[current.config.buildPlatform])
        return "bin"

    def getLibSubDir(self, mapping, process, current):

        # PHP module is always installed in the lib directory for the default build platform
        if isinstance(mapping, PhpMapping) and current.config.buildPlatform == self.getDefaultBuildPlatform():
            return "lib"

        if self.linuxId in ["centos", "rhel", "fedora"]:
            return "lib64" if current.config.buildPlatform == "x64" else "lib"
        elif self.linuxId in ["ubuntu", "debian"]:
            return os.path.join("lib", self.multiArch[current.config.buildPlatform])
        return "lib"

    def getBuildSubDir(self, name, current):
        if self.linuxId in ["ubuntu", "debian"]:
            return os.path.join("build", self.multiArch[current.config.buildPlatform], current.config.buildConfig)
        else:
            return os.path.join("build", current.config.buildPlatform, current.config.buildConfig)

    def getDefaultExe(self, name, config):
        if name == "icebox":
            if self.linuxId in ["centos", "rhel", "fedora"] and config.buildPlatform == "x86":
                name += "32" # Multilib platform
            if config.cpp11:
                name += "++11"
        return name

    def canRun(self, mapping, current):
        if self.linuxId in ["centos", "rhel", "fedora"] and current.config.buildPlatform == "x86":
            #
            # Don't test Glacier2/IceStorm/IceGrid services with multilib platforms. We only
            # build services for the native platform.
            #
            parent = re.match(r'^([\w]*).*', current.testcase.getTestSuite().getId()).group(1)
            if parent in ["Glacier2", "IceStorm", "IceGrid"]:
                return False
        return True

class Windows(Platform):

    def getFilters(self, config):
        if config.uwp:
            return (["Ice/.*", "IceSSL/configuration"],
                    ["Ice/background",
                     #
                     # TODO: Test scripts are killing the Ice/binding test because it takes
                     # too much time to run
                     #
                     "Ice/binding",
                     "Ice/checksum",
                     "Ice/custom",
                     "Ice/defaultServant",
                     "Ice/defaultValue",
                     "Ice/faultTolerance",
                     "Ice/gc",
                     "Ice/interceptor",
                     "Ice/library",
                     "Ice/logger",
                     "Ice/networkProxy",
                     "Ice/properties",
                     "Ice/plugin",
                     "Ice/stringConverter",
                     "Ice/servantLocator",
                     "Ice/services",
                     "Ice/slicing/exceptions",
                     "Ice/slicing/objects",
                     "Ice/threadPoolPriority",
                     # TODO: Only run IceSSL/configuration with remote C++ server.
                     "IceSSL/configuration"])
        return Platform.getFilters(self, config)

    def parseBuildVariables(self, variables):
        pass # Nothing to do, we don't support the make build system on Windows

    def getDefaultBuildPlatform(self):
        return "Win32"

    def getDefaultBuildConfig(self):
        return "Debug"

    def getCompiler(self):
        out = run("cl")
        if out.find("Version 16.") != -1:
            return "v100"
        elif out.find("Version 17.") != -1:
            return "v110"
        elif out.find("Version 18.") != -1:
            return "v120"
        elif out.find("Version 19.") != -1:
            return "v140"

    def getBinSubDir(self, mapping, process, current):
        #
        # Platform/Config taget bin directories.
        #
        platform = current.config.buildPlatform
        config = "Debug" if current.config.buildConfig.find("Debug") >= 0 else "Release"

        if current.driver.useIceBinDist(mapping):
            iceHome = os.environ.get("ICE_HOME")
            v140 = self.getCompiler() == "v140"
            cpp = isinstance(mapping, CppMapping)
            csharp = isinstance(mapping, CSharpMapping)

            if iceHome and ((cpp and v140 and platform == "x64" and config == "Release") or (not csharp and not cpp)):
                return "bin"
            elif csharp or isinstance(process, SliceTranslator):
                return os.path.join("tools")
            else:

                #
                # With Windows binary distribution Glacier2 and IcePatch2 binaries are only included
                # for Release configuration.
                #
                binaries = [Glacier2Router, IcePatch2Calc, IcePatch2Client, IcePatch2Server]
                config = next(("Release" for p in binaries if isinstance(process, p)), config)

                return os.path.join("build", "native", "bin", platform, config)
        else:
            if isinstance(mapping, CppMapping):
                return os.path.join("bin", platform, config)
            elif isinstance(mapping, PhpMapping):
                return os.path.join("lib", platform, config)
            return "bin"

    def getLibSubDir(self, mapping, process, current):
        if isinstance(mapping, PhpMapping):
            return "php" if current.driver.useIceBinDist(mapping) else "lib"
        return self.getBinSubDir(mapping, process, current)

    def getBuildSubDir(self, name, current):
        if os.path.exists(os.path.join(current.testcase.getPath(), "msbuild", name)):
            return os.path.join("msbuild", name, current.config.buildPlatform, current.config.buildConfig)
        else:
            return os.path.join("msbuild", current.config.buildPlatform, current.config.buildConfig)

    def getLdPathEnvName(self):
        return "PATH"

    def getIceInstallDir(self, mapping, current):

        platform = current.config.buildPlatform
        config = "Debug" if current.config.buildConfig.find("Debug") >= 0 else "Release"

        with open(os.path.join(toplevel, "config", "icebuilder.props"), "r") as configFile:
            version = re.search("<IceJSONVersion>(.*)</IceJSONVersion>", configFile.read()).group(1)
        comp = self.getCompiler() if isinstance(mapping, CppMapping) else "net"
        iceHome = os.environ.get("ICE_HOME")

        v140 = self.getCompiler() == "v140"
        cpp = isinstance(mapping, CppMapping)
        csharp = isinstance(mapping, CSharpMapping)

        #
        # Use binary distribution from ICE_HOME if building for C++/VC140/x64/Release or
        # for another mapping than C++ or C#.
        #
        if iceHome and ((cpp and v140 and platform == "x64" and config == "Release") or (not csharp and not cpp)):
            return iceHome

        #
        # Otherwise, use the appropriate nuget package
        #
        return os.path.join(toplevel, mapping.name, "msbuild", "packages", "zeroc.ice.{0}.{1}".format(comp, version))

    def canRun(self, mapping, current):
        #
        # On Windows, if testing with a binary distribution, don't test Glacier2/IceStorm services
        # with the Debug configurations since we don't provide binaries for them.
        #
        if current.driver.useIceBinDist(mapping):
            parent = re.match(r'^([\w]*).*', current.testcase.getTestSuite().getId()).group(1)
            if parent in ["Glacier2", "IceStorm"] and current.config.buildConfig.find("Debug") >= 0:
                return False
        return True

platform = None
if sys.platform == "darwin":
    platform = Darwin()
elif sys.platform.startswith("aix"):
    platform = AIX()
elif sys.platform.startswith("linux") or sys.platform.startswith("gnukfreebsd"):
    platform = Linux()
elif sys.platform == "win32" or sys.platform[:6] == "cygwin":
    platform = Windows()

if not platform:
    print("can't run on unknown platform `{0}'".format(sys.platform))
    sys.exit(1)

def parseOptions(obj, options, mapped={}):
    # Transform configuration options provided on the command line to
    # object data members. The data members must be already set on the
    # object and with the correct type.
    if not hasattr(obj, "parsedOptions"):
        obj.parsedOptions=[]
    remaining = []
    for (o, a) in options:
        if o.startswith("--"): o = o[2:]
        if o.startswith("-"): o = o[1:]
        if o in mapped:
            o = mapped[o]

        if hasattr(obj, o):
            if isinstance(getattr(obj, o), bool):
                setattr(obj, o, a.lower() in ("yes", "true", "1") if a else True)
            elif isinstance(getattr(obj, o), list):
                l = getattr(obj, o)
                l.append(a)
            else:
                if not a and not isinstance(a, str):
                    a = "0"
                setattr(obj, o, type(getattr(obj, o))(a))
            if not o in obj.parsedOptions:
                obj.parsedOptions.append(o)
        else:
            remaining.append((o, a))
    options[:] = remaining


class Mapping:

    mappings = OrderedDict()

    class Config:

        # All option values for Ice/IceBox tests.
        coreOptions = {
            "protocol" : ["tcp", "ssl", "wss", "ws"],
            "compress" : [False, True],
            "ipv6" : [False, True],
            "serialize" : [False, True],
            "mx" : [False, True],
        }

        # All option values for IceGrid/IceStorm/Glacier2/IceDiscovery tests.
        serviceOptions = {
            "protocol" : ["tcp", "wss"],
            "compress" : [False, True],
            "ipv6" : [False, True],
            "serialize" : [False, True],
            "mx" : [False, True],
        }

        @classmethod
        def getOptions(self):
            return ("", ["config=", "platform=", "protocol=", "compress", "ipv6", "serialize", "mx", "cprops=", "sprops=", "uwp"])

        @classmethod
        def usage(self):
            pass

        @classmethod
        def commonUsage(self):
            print("")
            print("Mapping options:")
            print("--protocol=<prot>     Run with the given protocol.")
            print("--compress            Run the tests with protocol compression.")
            print("--ipv6                Use IPv6 addresses.")
            print("--serialize           Run with connection serialization.")
            print("--mx                  Run with metrics enabled.")
            print("--cprops=<properties> Specifies a list of additional client properties.")
            print("--sprops=<properties> Specifies a list of additional server properties.")
            print("--config=<config>     Build configuration for native executables.")
            print("--platform=<platform> Build platform for native executables.")
            print("--uwp                 Run UWP (Universal Windows Platform).")

        def __init__(self, options=[]):
            # Build configuration
            self.parsedOptions = []
            self.buildConfig = os.environ.get("CONFIGS", "").split(" ")[0]
            if self.buildConfig:
                self.parsedOptions.append("buildConfig")
            else:
                self.buildConfig = platform.getDefaultBuildConfig()

            self.buildPlatform = os.environ.get("PLATFORMS", "").split(" ")[0]
            if self.buildPlatform:
                self.parsedOptions.append("buildPlatform")
            else:
                self.buildPlatform = platform.getDefaultBuildPlatform()

            self.protocol = "tcp"
            self.compress = False
            self.serialize = False
            self.ipv6 = False
            self.mx = False
            self.cprops = []
            self.sprops = []
            self.uwp = False
            parseOptions(self, options, { "config" : "buildConfig", "platform" : "buildPlatform", "uwp" : "uwp" })

        def __str__(self):
            s = []
            for o in self.parsedOptions:
                v = getattr(self, o)
                if v: s.append(o if type(v) == bool else str(v))
            return ",".join(s)

        def getAll(self, current, testcase, rand=False):

            #
            # A generator to generate combinations of options (e.g.: tcp/compress/mx, ssl/ipv6/serialize, etc)
            #
            def gen(supportedOptions):

                if not supportedOptions:
                    yield self
                    return

                supportedOptions = supportedOptions.copy()
                supportedOptions.update(testcase.getMapping().getOptions(current))
                supportedOptions.update(testcase.getTestSuite().getOptions(current))
                supportedOptions.update(testcase.getOptions(current))

                for o in self.parsedOptions:
                    # Remove options which were explicitly set
                    if o in supportedOptions:
                        del supportedOptions[o]

                if len(supportedOptions) == 0:
                    yield self
                    return

                # Find the option with the longest list of values
                length = max([len(v) for v in supportedOptions.values()])

                # Replace the values with a cycle iterator on the values
                for (k, v) in supportedOptions.items():
                    supportedOptions[k] = itertools.cycle(random.sample(v, len(v)) if rand else v)

                # Now, for the length of the longest array of values, we return
                # an array with the supported option combinations
                for i in range(0, length):
                    options = []
                    for k, v in supportedOptions.items():
                        v = next(v)
                        if v:
                            if type(v) == bool:
                                options.append(("--{0}".format(k), None))
                            else:
                                options.append(("--{0}".format(k), v))

                    # Add parsed options
                    for o in self.parsedOptions:
                        v = getattr(self, o)
                        if type(v) == bool:
                            options.append(("--{0}".format(o), None))
                        elif type(v) == list:
                            options += [("--{0}".format(o), e) for e in v]
                        else:
                            options.append(("--{0}".format(o), v))

                    yield self.__class__(options)

            options = None
            parent = re.match(r'^([\w]*).*', testcase.getTestSuite().getId()).group(1)
            if isinstance(testcase, ClientServerTestCase) and parent in ["Ice", "IceBox"]:
                options = current.driver.filterOptions(testcase, self.coreOptions)
            elif parent in ["IceGrid", "Glacier2", "IceStorm", "IceDiscovery"]:
                options = current.driver.filterOptions(testcase, self.serviceOptions)

            return [c for c in gen(options)]

        def canRun(self, current):
            if not platform.canRun(self, current):
                return False

            options = {}
            options.update(current.testcase.getMapping().getOptions(current))
            options.update(current.testcase.getTestSuite().getOptions(current))
            options.update(current.testcase.getOptions(current))

            for (k, v) in options.items():
                if not hasattr(self, k):
                    continue
                if not getattr(self, k) in v:
                    return False
            else:
                return True

        def cloneRunnable(self, current):
            #
            # Clone this configuration and make sure all the options are supported
            #
            options = {}
            options.update(current.testcase.getMapping().getOptions(current))
            options.update(current.testcase.getTestSuite().getOptions(current))
            options.update(current.testcase.getOptions(current))
            clone = copy.copy(self)
            for o in self.parsedOptions:
                if o in options and getattr(self, o) not in options[o]:
                    setattr(clone, o, options[o][0] if len(options[o]) > 0 else None)
            return clone

        def cloneAndOverrideWith(self, current):
            #
            # Clone this configuration and override options with options from the given configuration
            # (the parent configuraton). This is usefull when running cross-testing. For example, JS
            # tests don't support all the options so we clone the C++ configuration and override the
            # options that are set on the JS configuration.
            #
            clone = copy.copy(self)
            for o in current.config.parsedOptions + ["protocol"]:
                if o not in ["buildConfig", "buildPlatform"]:
                    setattr(clone, o, getattr(current.config, o))
            clone.parsedOptions = current.config.parsedOptions
            return clone

        def getArgs(self, process, current):
            return []

        def getProps(self, process, current):
            props = {}
            if isinstance(process, IceProcess):
                props["Ice.Warn.Connections"] = True
                if self.protocol:
                    props["Ice.Default.Protocol"] = self.protocol
                if self.compress:
                    props["Ice.Override.Compress"] = "1"
                if self.serialize:
                    props["Ice.ThreadPool.Server.Serialize"] = "1"
                props["Ice.IPv6"] = self.ipv6
                if self.ipv6:
                    props["Ice.PreferIPv6Address"] = True
                if self.mx:
                    props["Ice.Admin.Endpoints"] = "default -h localhost"
                    props["Ice.Admin.InstanceName"] = "Server" if isinstance(process, Server) else "Client"
                    props["IceMX.Metrics.Debug.GroupBy"] ="id"
                    props["IceMX.Metrics.Parent.GroupBy"] = "parent"
                    props["IceMX.Metrics.All.GroupBy"] = "none"

                # Additional properties specified on the command line with --cprops or --sprops
                additionalProps = []
                if self.cprops and isinstance(process, Client):
                    additionalProps = self.cprops
                elif self.sprops and isinstance(process, Server):
                    additionalProps = self.sprops
                for pps in additionalProps:
                    for p in pps.split(" "):
                        if p.find("=") > 0:
                            (k , v) = p.split("=")
                            props[k] = v
                        else:
                            props[p] = True

            return props

    @classmethod
    def getByName(self, name):
        if not name in self.mappings:
            raise RuntimeError("unknown mapping `{0}'".format(name))
        return self.mappings.get(name)

    @classmethod
    def getByPath(self, path):
        path = os.path.abspath(path)
        for m in self.mappings.values():
            if path.startswith(m.getPath() + os.sep):
                return m

    @classmethod
    def add(self, name, mapping):
        self.mappings[name] = mapping.init(name)

    @classmethod
    def getAll(self):
        languages = os.environ.get("LANGUAGES", None)
        return [self.getByName(l) for l in languages.split(" ")] if languages else list(self.mappings.values())

    def __init__(self, path=None):
        self.platform = None
        self.name = None
        self.path = os.path.abspath(path) if path else None
        self.testsuites = {}

    def init(self, name):
        self.name = name
        if not self.path:
            self.path = os.path.join(toplevel, name)
        return self

    def __str__(self):
        return self.name

    def createConfig(self, options):
        return self.Config(options)

    def filterTestSuite(self, testId, config, filters=[], rfilters=[]):
        (pfilters, prfilters) = platform.getFilters(config)
        for includes in [filters, [re.compile(pf) for pf in pfilters]]:
            if len(includes) > 0:
                for f in includes:
                    if f.search(self.name + "/" + testId):
                        break
                else:
                    return True

        for excludes in [rfilters, [re.compile(pf) for pf in prfilters]]:
            if len(excludes) > 0:
                for f in excludes:
                    if f.search(self.name + "/" + testId):
                        return True

        return False

    def loadTestSuites(self, tests, config, filters=[], rfilters=[]):
        for test in tests or [""]:
            for root, dirs, files in os.walk(os.path.join(self.getTestsPath(), test.replace('/', os.sep))):

                testId = root[len(self.getTestsPath()) + 1:]
                if os.sep != "/":
                    testId = testId.replace(os.sep, "/")

                if self.filterTestSuite(testId, config, filters, rfilters):
                    continue

                #
                # First check if there's a test.py file in the directory, if there's one use it.
                #
                if "test.py" in files:
                    #
                    # WORKAROUND for Python issue 15230 (fixed in 3.2) where run_path doesn't work correctly.
                    #
                    #runpy.run_path(os.path.join(root, "test.py"))
                    origsyspath = sys.path
                    sys.path = [root] + sys.path
                    runpy.run_module("test", init_globals=globals(), run_name=root)
                    origsyspath = sys.path
                    continue

                #
                # If there's no test.py file in the test directory, we check if there's a common
                # script for the test in scripts/tests. If there's on we use it.
                #
                script = os.path.join(self.getCommonTestsPath(), testId + ".py")
                if os.path.isfile(script):
                    runpy.run_module("tests." + testId.replace("/", "."), init_globals=globals(), run_name=root)
                    continue

                #
                # Finally, we try to "discover/compute" the test by looking up for well-known
                # files.
                #
                testcases = self.computeTestCases(testId, files)
                if testcases:
                    TestSuite(root, testcases)

    def getTestSuites(self, ids=[]):
        if not ids:
            return self.testsuites.values()
        return [self.testsuites[testSuiteId] for testSuiteId in ids if testSuiteId in self.testsuites]

    def addTestSuite(self, testsuite):
        assert len(testsuite.path) > len(self.getTestsPath()) + 1
        testSuiteId = testsuite.path[len(self.getTestsPath()) + 1:].replace('\\', '/')
        self.testsuites[testSuiteId] = testsuite
        return testSuiteId

    def findTestSuite(self, testsuite):
        return self.testsuites.get(testsuite if isinstance(testsuite, str) else testsuite.id)

    def computeTestCases(self, testId, files):

        # Instantiate a new test suite if the directory contains well-known source files.

        def checkFile(f, m):
            try:
                # If given mapping is same as local mapping, just check the files set, otherwise check
                # with the mapping
                return (self.getDefaultSource(f) in files) if m == self else m.hasSource(testId, f)
            except KeyError:
                # Expected if the mapping doesn't support the process type (such as clientBidir)
                return False

        checkClient = lambda f: checkFile(f, self.getClientMapping())
        checkServer = lambda f: checkFile(f, self.getServerMapping())

        testcases = []
        if checkClient("client") and checkServer("server"):
            testcases.append(ClientServerTestCase())
        if checkClient("client") and checkServer("serveramd") and self.getServerMapping() == self:
            testcases.append(ClientAMDServerTestCase())
        if checkClient("client") and len(testcases) == 0:
            testcases.append(ClientTestCase())
        if checkClient("clientBidir") and self.getServerMapping().hasSource("Ice/echo", "server"):
            testcases.append(ClientEchoServerTestCase())
        if checkClient("collocated"):
            testcases.append(CollocatedTestCase())
        if len(testcases) > 0:
            return testcases

    def hasSource(self, testId, processType):
        try:
            return os.path.exists(os.path.join(self.getTestsPath(), testId, self.getDefaultSource(processType)))
        except KeyError:
            return False

    def getPath(self):
        return self.path

    def getTestsPath(self):
        return os.path.join(self.path, "test")

    def getCommonTestsPath(self):
        return os.path.join(self.path, "..", "scripts", "tests")

    def getTestCwd(self, process, current):
        return current.testcase.getPath()

    def getDefaultSource(self, processType):
        return processType

    def getDefaultProcess(self, processType, testsuite):
        #
        # If no server or client is explicitly set with a testcase, getDefaultProcess is called
        # to figure out which process class to instantiate. Based on the processType and the testsuite
        # we instantiate the right default process class.
        #
        if processType is None:
            return None
        elif testsuite.getId().startswith("IceUtil") or testsuite.getId().startswith("Slice"):
            return SimpleClient()
        elif testsuite.getId().startswith("IceGrid"):
            if processType in ["client", "collocated"]:
                return IceGridClient()
            if processType in ["server", "serveramd"]:
                return IceGridServer()
        else:
            return Server() if processType in ["server", "serveramd"] else Client()

    def getDefaultExe(self, processType, config):
        return processType

    def getClientMapping(self):
        # The client mapping is always the same as this mapping.
        return self

    def getServerMapping(self):
        # Can be overridden for client-only mapping that relies on another mapping for servers
        return self

    def getBinDir(self, process, current):
        return os.path.join(current.driver.getIceDir(self, current), platform.getBinSubDir(self, process, current))

    def getLibDir(self, process, current):
        return os.path.join(current.driver.getIceDir(self, current), platform.getLibSubDir(self, process, current))

    def getBuildDir(self, name, current):
        return platform.getBuildSubDir(name, current)

    def getCommandLine(self, current, process, exe):
        name = exe
        if isinstance(platform, Windows) and not exe.endswith(".exe"):
            exe += ".exe"
        if process.isFromBinDir():
            # If it's a process from the bin directory, the location is platform specific
            # so we check with the platform.
            return os.path.join(self.getBinDir(process, current), exe)
        elif current.testcase:
            # If it's a process from a testcase, the binary is in the test build directory.
            return os.path.join(current.testcase.getPath(), current.getBuildDir(name), exe)
        else:
            return exe

    def getProps(self, process, current):
        props = {}
        if isinstance(process, IceProcess):
            if current.config.protocol in ["bt", "bts"]:
                props["Ice.Plugin.IceBT"] = self.getPluginEntryPoint("IceBT", process, current)
            if current.config.protocol in ["ssl", "wss", "bts", "iaps"]:
                props.update(self.getSSLProps(process, current))
        return props

    def getSSLProps(self, process, current):
        sslProps = {
            "Ice.Plugin.IceSSL" : self.getPluginEntryPoint("IceSSL", process, current),
            "IceSSL.Password": "password",
            "IceSSL.DefaultDir": os.path.join(toplevel, "certs"),
        }

        #
        # If the client doesn't support client certificates, set IceSSL.VerifyPeer to 0
        #
        if isinstance(process, Server):
            if isinstance(current.testsuite.getMapping(), JavaScriptMapping):
                sslProps["IceSSL.VerifyPeer"] = 0

        return sslProps

    def getArgs(self, process, current):
        return []

    def getEnv(self, process, current):
        return {}

    def getOptions(self, current):
        return {}

    def getRunOrder(self):
        return ["Slice", "IceUtil", "Ice", "IceSSL", "IceBox", "Glacier2", "IceGrid", "IceStorm"]

    def getCrossTestSuites(self):
        return [
            "Ice/ami",
            "Ice/info",
            "Ice/exceptions",
            "Ice/enums",
            "Ice/facets",
            "Ice/inheritance",
            "Ice/invoke",
            "Ice/objects",
            "Ice/operations",
            "Ice/proxy",
            "Ice/servantLocator",
            "Ice/slicing/exceptions",
            "Ice/slicing/objects",
            "Ice/optional"
        ]

#
# A Runnable can be used as a "client" for in test cases, it provides
# implements run, setup and teardown methods.
#
class Runnable:

    def __init__(self, desc=None):
        self.desc = desc

    def setup(self, current):
        ### Only called when ran from testcase
        pass

    def teardown(self, current, success):
        ### Only called when ran from testcase
        pass

    def run(self, current):
        pass

#
# A Process describes how to run an executable process.
#
class Process(Runnable):

    processType = None

    def __init__(self, exe=None, outfilters=None, quiet=False, args=None, props=None, envs=None, desc=None, mapping=None):
        Runnable.__init__(self, desc)
        self.exe = exe
        self.outfilters = outfilters or []
        self.quiet = quiet
        self.args = args or []
        self.props = props or {}
        self.envs = envs or {}
        self.process = None
        self.output = None
        self.mapping = mapping

    def __str__(self):
        if not self.exe:
            return str(self.__class__)
        return self.exe + (" ({0})".format(self.desc) if self.desc else "")

    def getOutput(self):
        assert(self.process or self.output is not None)

        def d(s):
            return s if isPython2 else s.decode("utf-8") if isinstance(s, bytes) else s

        output = d(self.process.getOutput() if self.process else self.output)
        try:
            # Apply outfilters to the output
            if len(self.outfilters) > 0:
                lines = output.split('\n')
                newLines = []
                previous = ""
                for line in [line + '\n' for line in lines]:
                    for f in self.outfilters:
                        if isinstance(f, types.LambdaType) or isinstance(f, types.FunctionType):
                            line = f(line)
                        elif f.search(line):
                            break
                    else:
                        if line.endswith('\n'):
                            if previous:
                                newLines.append(previous + line)
                                previous = ""
                            else:
                                newLines.append(line)
                        else:
                            previous += line
                output = "".join(newLines)
            output = output.strip()
            return output + '\n' if output else ""
        except Exception as ex:
            print("unexpected exception while filtering process output:\n" + str(ex))
            raise

    def run(self, current, args=[], props={}, exitstatus=0, timeout=120):
        class WatchDog:

            def __init__(self, timeout):
                self.lastProgressTime = time.time()
                self.timeout = timeout
                self.lock = threading.Lock()

            def reset(self):
                with self.lock: self.lastProgressTime = time.time()

            def timedOut(self):
                with self.lock:
                    return (time.time() - self.lastProgressTime) >= self.timeout

        watchDog = WatchDog(timeout)
        self.start(current, args, props, watchDog=watchDog)
        if not self.quiet and not current.driver.isWorkerThread():
            # Print out the process output to stdout if we're running the client form the main thread.
            self.process.trace(self.outfilters)
        while True:
            try:
                self.process.waitSuccess(exitstatus=exitstatus, timeout=30)
                break
            except Expect.TIMEOUT:
                if watchDog and watchDog.timedOut():
                    raise
        self.stop(current, True, exitstatus)

    def getEffectiveArgs(self, current, args):
        allArgs = []
        allArgs += current.driver.getArgs(self, current)
        allArgs += current.config.getArgs(self, current)
        allArgs += self.getMapping(current).getArgs(self, current)
        allArgs += current.testcase.getArgs(self, current)
        allArgs += self.getArgs(current)
        allArgs += self.args(self, current) if callable(self.args) else self.args
        allArgs += args
        allArgs = [a.encode("utf-8") if type(a) == "unicode" else str(a) for a in allArgs]
        return allArgs

    def getEffectiveProps(self, current, props):
        allProps = {}
        allProps.update(current.driver.getProps(self, current))
        allProps.update(current.config.getProps(self, current))
        allProps.update(self.getMapping(current).getProps(self, current))
        allProps.update(current.testcase.getProps(self, current))
        allProps.update(self.getProps(current))
        allProps.update(self.props(self, current) if callable(self.props) else self.props)
        allProps.update(props)
        return allProps

    def getEffectiveEnv(self, current):
        allEnvs = {}
        allEnvs.update(self.getMapping(current).getEnv(self, current))
        allEnvs.update(current.testcase.getEnv(self, current))
        allEnvs.update(self.getEnv(current))
        allEnvs.update(self.envs(self, current) if callable(self.envs) else self.envs)
        return allEnvs

    def start(self, current, args=[], props={}, watchDog=None):
        allArgs = self.getEffectiveArgs(current, args)
        allProps = self.getEffectiveProps(current, props)
        allEnvs = self.getEffectiveEnv(current)

        self.output = None
        self.process = current.driver.getProcessController(current).start(self, current, allArgs, allProps, allEnvs, watchDog)
        try:
            self.waitForStart(current)
        except:
            self.stop(current)
            raise

    def waitForStart(self, current):
        # To be overridden in specialization to wait for a token indicating the process readiness.
        pass

    def stop(self, current, waitSuccess=False, exitstatus=0):
        if self.process:
            try:
                if waitSuccess: # Wait for the process to exit successfully by itself.
                    self.process.waitSuccess(exitstatus=exitstatus, timeout=60)
            finally:
                self.process.terminate()
                self.output = self.process.getOutput()
                self.process = None
                if not self.quiet: # Write the output to the test case (but not on stdout)
                    current.write(self.getOutput(), stdout=False)

    def expect(self, pattern, timeout=60):
        assert(self.process)
        return self.process.expect(pattern, timeout)

    def sendline(self, data):
        assert(self.process)
        return self.process.sendline(data)

    def isStarted(self):
        return self.process is not None

    def isFromBinDir(self):
        return False

    def getArgs(self, current):
        return []

    def getProps(self, current):
        return {}

    def getEnv(self, current):
        return {}

    def getMapping(self, current):
        return self.mapping or current.testcase.getMapping()

    def getExe(self, current):
        processType = self.processType or current.testcase.getProcessType(self)
        return self.exe or self.getMapping(current).getDefaultExe(processType, current.config)

    def getCommandLine(self, current):
        return self.getMapping(current).getCommandLine(current, self, self.getExe(current))

#
# A simple client (used to run Slice/IceUtil clients for example)
#
class SimpleClient(Process):
    pass

#
# An IceProcess specialization class. This is used by drivers to figure out if
# the process accepts Ice configuration properties.
#
class IceProcess(Process):
    pass

#
# An Ice server process. It's possible to configure when the server is considered
# ready by setting readyCount or ready. The start method will only return once
# the server is considered "ready". It can also be configure to wait (the default)
# or not wait for shutdown when the stop method is invoked.
#
class Server(IceProcess):

    def __init__(self, exe=None, waitForShutdown=True, readyCount=1, ready=None, startTimeout=120, *args, **kargs):
        IceProcess.__init__(self, exe, *args, **kargs)
        self.waitForShutdown = waitForShutdown
        self.readyCount = readyCount
        self.ready = ready
        self.startTimeout = startTimeout

    def getProps(self, current):
        props = IceProcess.getProps(self, current)
        props.update({
            "Ice.ThreadPool.Server.Size": 1,
            "Ice.ThreadPool.Server.SizeMax": 3,
            "Ice.ThreadPool.Server.SizeWarn": 0,
        })
        props.update(current.driver.getProcessProps(current, self.ready, self.readyCount + (1 if current.config.mx else 0)))
        return props

    def waitForStart(self, current):
        # Wait for the process to be ready
        self.process.waitReady(self.ready, self.readyCount + (1 if current.config.mx else 0), self.startTimeout)

        # Filter out remaining ready messages
        self.outfilters.append(re.compile("[^\n]+ ready"))

        # If we are not asked to be quiet and running from the main thread, print the server output
        if not self.quiet and not current.driver.isWorkerThread():
            self.process.trace(self.outfilters)

    def stop(self, current, waitSuccess=False, exitstatus=0):
        IceProcess.stop(self, current, waitSuccess and self.waitForShutdown, exitstatus)

#
# An Ice client process.
#
class Client(IceProcess):
    pass

#
# Executables for processes inheriting this marker class are looked up in the
# Ice distribution bin directory.
#
class ProcessFromBinDir:

    def isFromBinDir(self):
        return True

class SliceTranslator(ProcessFromBinDir, SimpleClient):

    def __init__(self, translator):
        SimpleClient.__init__(self, exe=translator, quiet=True, mapping=Mapping.getByName("cpp"))

    def getCommandLine(self, current):
        translator = self.getMapping(current).getCommandLine(current, self, self.getExe(current))

        #
        # Look for slice2py installed by Pip if not found in the bin directory
        #
        if self.exe == "slice2py" and not os.path.exists(translator):
            if isinstance(platform, Windows):
                return os.path.join(os.path.dirname(sys.executable), "Scripts", "slice2py.exe")
            elif os.path.exists("/usr/local/bin/slice2py"):
                return "/usr/local/bin/slice2py"
            else:
                import slice2py
                return sys.executable + " " + os.path.normpath(
                            os.path.join(slice2py.__file__, "..", "..", "..", "..", "bin", "slice2py"))

        return translator

#
# A test case is composed of servers and clients. When run, all servers are started
# sequentially. When the servers are ready, the clients are also ran sequentially.
# Once all the clients are terminated, the servers are stopped (which waits for the
# successful completion of the server).
#
# A TestCase is also a "Runnable", like the Process class. In other words, it can be
# used a client to allow nested test cases.
#
class TestCase(Runnable):

    def __init__(self, name, client=None, clients=None, server=None, servers=None, args=[], props={}, envs={},
                 options={}, desc=None):
        Runnable.__init__(self, desc)

        self.name = name
        self.parent = None
        self.mapping = None
        self.testsuite = None
        self.options = options
        self.dirs = []
        self.files = []
        self.args = args
        self.props = props
        self.envs = envs

        #
        # Setup client list, "client" can be a string in which case it's assumed to
        # to the client executable name.
        #
        self.clients = clients
        if client:
            client = Client(exe=client) if isinstance(client, str) else client
            self.clients = [client] if not self.clients else self.clients + [client]

        #
        # Setup server list, "server" can be a string in which case it's assumed to
        # to the server executable name.
        #
        self.servers = servers
        if server:
            server = Server(exe=server) if isinstance(server, str) else server
            self.servers = [server] if not self.servers else self.servers + [server]

    def __str__(self):
        return self.name

    def init(self, mapping, testsuite):
        # init is called when the testcase is added to the given testsuite
        self.mapping = mapping
        self.testsuite = testsuite

        #
        # If no clients are explicitly specified, we instantiate one if getClientType()
        # returns the type of client to instantiate (client, collocated, etc)
        #
        if not self.clients:
            client = self.mapping.getDefaultProcess(self.getClientType(), testsuite)
            self.clients = [client] if client else []

        #
        # If no servers are explicitly specified, we instantiate one if getServerType()
        # returns the type of server to instantiate (server, serveramd, etc)
        #
        if not self.servers:
            server = self.mapping.getDefaultProcess(self.getServerType(), testsuite)
            self.servers = [server] if server else []

    def getOptions(self, current):
        return self.options

    def canRun(self, current):
        # Can be overriden
        return True

    def setupServerSide(self, current):
        # Can be overridden to perform setup activities before the server side is started
        pass

    def teardownServerSide(self, current, success):
        # Can be overridden to perform terddown after the server side is stopped
        pass

    def setupClientSide(self, current):
        # Can be overridden to perform setup activities before the client side is started
        pass

    def teardownClientSide(self, current, success):
        # Can be overridden to perform terddown after the client side is stopped
        pass

    def startServerSide(self, current):
        for server in self.servers:
            self._startServer(current, server)

    def stopServerSide(self, current, success):
        for server in reversed(self.servers):
            self._stopServer(current, server, success)

    def runClientSide(self, current):
        for client in self.clients:
            self._runClient(current, client)

    def getTestSuite(self):
        return self.testsuite

    def getParent(self):
        return self.parent

    def getName(self):
        return self.name

    def getPath(self):
        return self.testsuite.getPath()

    def getMapping(self):
        return self.mapping

    def getArgs(self, process, current):
        return self.args

    def getProps(self, process, current):
        return self.props

    def getEnv(self, process, current):
        return self.envs

    def getProcessType(self, process):
        if process in self.clients:
            return self.getClientType()
        elif process in self.servers:
            return self.getServerType()
        elif isinstance(process, Server):
            return self.getServerType()
        else:
            return self.getClientType()

    def getClientType(self):
        # Overridden by test case specialization to specify the type of client to instantiate
        # if no client is explicitly provided
        return None

    def getServerType(self):
        # Overridden by test case specialization to specify the type of client to instantiate
        # if no server is explicitly provided
        return None

    def getServerTestCase(self, cross=None):
        testsuite = (cross or self.mapping.getServerMapping()).findTestSuite(self.testsuite)
        return testsuite.findTestCase(self) if testsuite else None

    def getClientTestCase(self):
        testsuite = self.mapping.getClientMapping().findTestSuite(self.testsuite)
        return testsuite.findTestCase(self) if testsuite else None

    def _startServerSide(self, current):
        # Set the host to use for the server side
        current.push(self)
        current.host = current.driver.getProcessController(current).getHost(current)
        self.setupServerSide(current)
        try:
            self.startServerSide(current)
            return current.host
        except:
            self._stopServerSide(current, False)
            raise
        finally:
            current.pop()

    def _stopServerSide(self, current, success):
        current.push(self)
        try:
            self.stopServerSide(current, success)
        finally:
            for server in reversed(self.servers):
                if server.isStarted():
                    self._stopServer(current, server, False)
            self.teardownServerSide(current, success)
            current.pop()

    def _startServer(self, current, server):
        if server.desc:
            current.write("starting {0}... ".format(server.desc))
        server.setup(current)
        server.start(current)
        if server.desc:
            current.writeln("ok")

    def _stopServer(self, current, server, success):
        try:
            server.stop(current, success)
        except:
            success = False
            raise
        finally:
            server.teardown(current, success)

    def _runClientSide(self, current, host=None):
        current.push(self, host)
        self.setupClientSide(current)
        success = False
        try:
            self.runClientSide(current)
            success = True
        finally:
            self.teardownClientSide(current, success)
            current.pop()

    def _runClient(self, current, client):
        success = False
        if client.desc:
            current.writeln("running {0}...".format(client.desc))
        client.setup(current)
        try:
            client.run(current)
            success = True
        finally:
            client.teardown(current, success)

    def run(self, current):
        try:
            current.push(self)
            current.result.started(self)
            self.runWithDriver(current)
            current.result.succeeded(self)
        except Exception as ex:
            current.result.failed(self, traceback.format_exc() if current.driver.debug else str(ex))
            raise
        finally:
            current.pop()
            for d in self.dirs:
                if os.path.exists(d): shutil.rmtree(d)
            for f in self.files:
                if os.path.exists(f): os.unlink(f)

    def createFile(self, path, lines, encoding=None):
        path = os.path.join(self.getPath(), path.decode("utf-8") if isPython2 else path)
        with open(path, "w", encoding=encoding) if not isPython2 and encoding else open(path, "w") as file:
            for l in lines:
                file.write("%s\n" % l)
        self.files.append(path)

    def mkdirs(self, dirs):
        for d in dirs if isinstance(dirs, list) else [dirs]:
            d = os.path.join(self.getPath(), d)
            self.dirs.append(d)
            if not os.path.exists(d):
                os.makedirs(d)

class ClientTestCase(TestCase):

    def __init__(self, name="client", *args, **kargs):
        TestCase.__init__(self, name, *args, **kargs)

    def runWithDriver(self, current):
        current.driver.runTestCase(current)

    def getClientType(self):
        return "client"

class ClientServerTestCase(ClientTestCase):

    def __init__(self, name="client/server", *args, **kargs):
        TestCase.__init__(self, name, *args, **kargs)

    def runWithDriver(self, current):
        current.driver.runClientServerTestCase(current)

    def getServerType(self):
        return "server"

class ClientEchoServerTestCase(ClientServerTestCase):

    def __init__(self, name="client/echo server", *args, **kargs):
        ClientServerTestCase.__init__(self, name, *args, **kargs)

    def getServerTestCase(self, cross=None):
        ts = Mapping.getByName("cpp").findTestSuite("Ice/echo")
        if ts:
            return ts.findTestCase("server")
        return None

    def getClientType(self):
        return "clientBidir"

class CollocatedTestCase(ClientTestCase):

    def __init__(self, name="collocated", *args, **kargs):
        TestCase.__init__(self, name, *args, **kargs)

    def getClientType(self):
        return "collocated"

class ClientAMDServerTestCase(ClientServerTestCase):

    def __init__(self, name="client/amd server", *args, **kargs):
        ClientServerTestCase.__init__(self, name, *args, **kargs)

    def getServerType(self):
        return "serveramd"

class Result:

    def __init__(self, testsuite, writeToStdout):
        self.testsuite = testsuite
        self._skipped = []
        self._failed = {}
        self._succeeded = []
        self._stdout = StringIO()
        self._writeToStdout = writeToStdout
        self._testcases = {}

    def started(self, testcase):
        self._start = self._stdout.tell()

    def failed(self, testcase, exception):
        self.writeln("\ntest in {0} failed:\n{1}".format(self.testsuite, exception))
        self._testcases[testcase] = (self._start, self._stdout.tell())
        self._failed[testcase] = exception

    def succeeded(self, testcase):
        self._testcases[testcase] = (self._start, self._stdout.tell())
        self._succeeded.append(testcase)

    def isSuccess(self):
        return len(self._failed) == 0

    def getFailed(self):
        return self._failed

    def getOutput(self, testcase=None):
        if testcase:
            if testcase in self._testcases:
                (start, end) = self._testcases[testcase]
                self._stdout.seek(start)
                try:
                    return self._stdout.read(end - start)
                finally:
                    self._stdout.seek(os.SEEK_END)

        return self._stdout.getvalue()

    def write(self, msg, stdout=True):
        if self._writeToStdout and stdout:
            try:
                sys.stdout.write(msg)
            except UnicodeEncodeError:
                #
                # The console doesn't support the encoding of the message, we convert the message
                # to an UTF-8 byte sequence and print out the byte sequence. We replace all the
                # double backslash from the byte sequence string representation to single back
                # slash.
                #
                sys.stdout.write(str(msg.encode("utf-8")).replace("\\\\", "\\"))
            sys.stdout.flush()
        self._stdout.write(msg)

    def writeln(self, msg, stdout=True):
        if self._writeToStdout and stdout:
            try:
                print(msg)
            except UnicodeEncodeError:
                #
                # The console doesn't support the encoding of the message, we convert the message
                # to an UTF-8 byte sequence and print out the byte sequence. We replace all the
                # double backslash from the byte sequence string representation to single back
                # slash.
                #
                print(str(msg.encode("utf-8")).replace("\\\\", "\\"))
        self._stdout.write(msg)
        self._stdout.write("\n")

class TestSuite:

    def __init__(self, path, testcases=None, options={}, libDirs=[], runOnMainThread=False, chdir=False, multihost=True):
        self.path = os.path.dirname(path) if os.path.basename(path) == "test.py" else path
        self.mapping = Mapping.getByPath(self.path)
        self.id = self.mapping.addTestSuite(self)
        self.options = options
        self.libDirs = libDirs
        self.runOnMainThread = runOnMainThread
        self.chdir = chdir
        self.multihost = multihost
        if self.chdir:
            # Only tests running on main thread can change the current working directory
            self.runOnMainThread = True
        self.files = []
        if testcases is None:
            files = [f for f in os.listdir(self.path) if os.path.isfile(os.path.join(self.path, f))]
            testcases = self.mapping.computeTestCases(self.id, files)
        self.testcases = OrderedDict()
        for testcase in testcases if testcases else []:
            testcase.init(self.mapping, self)
            if testcase.name in self.testcases:
                raise RuntimeError("duplicate testcase {0} in testsuite {1}".format(testcase, self))
            self.testcases[testcase.name] = testcase

    def __str__(self):
        return self.id

    def getId(self):
        return self.id

    def getOptions(self, current):
        return self.options

    def getPath(self):
        return self.path

    def getMapping(self):
        return self.mapping

    def getLibDirs(self):
        return self.libDirs

    def isMainThreadOnly(self):
        for m in [CppMapping, JavaMapping, CSharpMapping]:
            if isinstance(self.mapping, m):
                return self.runOnMainThread
        else:
            return True

    def addTestCase(self, testcase):
        if testcase.name in self.testcases:
            raise RuntimeError("duplicate testcase {0} in testsuite {1}".format(testcase, self))
        testcase.init(self.mapping, self)
        self.testcases[testcase.name] = testcase

    def findTestCase(self, testcase):
        return self.testcases.get(testcase if isinstance(testcase, str) else testcase.name)

    def getTestCases(self):
        return self.testcases.values()

    def setup(self, current):
        pass

    def run(self, current):
        try:
            cwd=None
            if self.chdir:
                cwd = os.getcwd()
                os.chdir(self.path)
            current.driver.runTestSuite(current)
        finally:
            if cwd: os.chdir(cwd)
            for f in self.files:
                if os.path.exists(f): os.remove(f)

    def teardown(self, current, success):
        pass

    def createFile(self, path, lines, encoding=None):
        path = os.path.join(self.path, path.decode("utf-8") if isPython2 else path)
        with open(path, "w", encoding=encoding) if not isPython2 and encoding else open(path, "w") as file:
            for l in lines:
                file.write("%s\n" % l)
        self.files.append(path)

    def isMultiHost(self):
        return self.multihost

    def isCross(self):
        # Only run the tests that support cross testing --all-cross or --cross
        return self.id in self.mapping.getCrossTestSuites()

class ProcessController:

    def __init__(self, current):
        pass

    def start(self, process, current, args, props, envs, watchDog):
        raise NotImplemented()

    def destroy(self, driver):
        pass

class LocalProcessController(ProcessController):

    class Process(Expect.Expect):

        def waitReady(self, ready, readyCount, startTimeout):
            if ready:
                self.expect("%s ready\n" % ready, timeout = startTimeout)
            else:
                while readyCount > 0:
                    self.expect("[^\n]+ ready\n", timeout = startTimeout)
                    readyCount -= 1

    def getHost(self, current):
        # Depending on the configuration, either use an IPv4, IPv6 or BT address for Ice.Default.Host
        if current.config.ipv6:
            return current.driver.hostIPv6
        elif current.config.protocol == "bt":
            if not current.driver.hostBT:
                raise Test.Common.TestCaseFailedException("no Bluetooth address set with --host-bt")
            return current.driver.hostBT
        else:
            return current.driver.host if current.driver.host else current.driver.interface

    def start(self, process, current, args, props, envs, watchDog):

        #
        # Props and arguments can use the format parameters set below in the kargs
        # dictionary. It's time to convert them to their values.
        #
        kargs = {
            "process": process,
            "testcase": current.testcase,
            "testdir": current.testcase.getPath(),
            "builddir": current.getBuildDir(process.getExe(current)),
            "icedir" : current.driver.getIceDir(current.testcase.getMapping(), current),
        }

        args = ["--{0}={1}".format(k, val(v)) for k,v in props.items()] + [val(a) for a in args]
        for k, v in envs.items():
            envs[k] = val(v, quoteValue=False)

        cmd = (process.getCommandLine(current) + (" " + " ".join(args) if len(args) > 0 else "")).format(**kargs)
        if current.driver.debug:
            if len(envs) > 0:
                current.writeln("({0} env={1})".format(cmd, envs))
            else:
                current.writeln("({0})".format(cmd))

        env = os.environ.copy()
        env.update(envs)
        cwd = process.getMapping(current).getTestCwd(process, current)
        process = LocalProcessController.Process(cmd, startReader=False, env=env, cwd=cwd, desc=process.desc)
        process.startReader(watchDog)
        return process

class RemoteProcessController(ProcessController):

    class Process:
        def __init__(self, exe, proxy):
            self.exe = exe
            self.proxy = proxy
            self.stdout = False

        def waitReady(self, ready, readyCount, startTimeout):
            self.proxy.waitReady(startTimeout)

        def waitSuccess(self, exitstatus=0, timeout=60):
            try:
                result = self.proxy.waitSuccess(timeout)
            except:
                raise Except.TIMEOUT("waitSuccess timeout")
            if exitstatus != result:
                raise RuntimeError("unexpected exit status: expected: %d, got %d\n" % (exitstatus, result))

        def getOutput(self):
            return self.output

        def trace(self, outfilters):
            self.stdout = True

        def terminate(self):
            self.output = self.proxy.terminate().strip()
            if self.stdout and self.output:
                print(self.output)

    def __init__(self, current, endpoints=None):
        self.processControllerProxies = {}
        self.controllerApps = []
        self.cond = threading.Condition()
        if endpoints:
            comm = current.driver.getCommunicator()
            import Test

            class ProcessControllerRegistryI(Test.Common.ProcessControllerRegistry):

                def __init__(self, remoteProcessController):
                    self.remoteProcessController = remoteProcessController

                def setProcessController(self, proxy, current):
                    import Test
                    proxy = Test.Common.ProcessControllerPrx.uncheckedCast(current.con.createProxy(proxy.ice_getIdentity()))
                    self.remoteProcessController.setProcessController(proxy)

            self.adapter = comm.createObjectAdapterWithEndpoints("Adapter", endpoints)
            self.adapter.add(ProcessControllerRegistryI(self), comm.stringToIdentity("Util/ProcessControllerRegistry"))
            self.adapter.activate()
        else:
            self.adapter = None

    def __str__(self):
        return "remote controller"

    def getHost(self, current):
        return self.getController(current).getHost(current.config.protocol, current.config.ipv6)

    def getController(self, current):
        ident = self.getControllerIdentity(current)
        if type(ident) == str:
            ident = current.driver.getCommunicator().stringToIdentity(ident)

        with self.cond:
            if ident in self.processControllerProxies:
                return self.processControllerProxies[ident]

        comm = current.driver.getCommunicator()
        import Ice
        import Test

        if current.driver.controllerApp:
            self.controllerApps.append(ident)
            self.startControllerApp(current, ident)

        if not self.adapter:
            # Use well-known proxy and IceDiscovery to discover the process controller object from the app.
            proxy = Test.Common.ProcessControllerPrx.uncheckedCast(comm.stringToProxy(comm.identityToString(ident)))
            try:
                proxy.ice_ping()
            except Exception as ex:
                raise RuntimeError("couldn't reach the remote controller `{0}'".format(proxy))

            with self.cond:
                self.processControllerProxies[ident] = proxy
                return self.processControllerProxies[ident]
        else:
            # Wait 10 seconds for a process controller to be registered with the ProcessControllerRegistry
            with self.cond:
                if not ident in self.processControllerProxies:
                    self.cond.wait(10)
                if ident in self.processControllerProxies:
                    return self.processControllerProxies[ident]
            raise RuntimeError("couldn't reach the remote controller `{0}'".format(ident))


    def setProcessController(self, proxy):
        with self.cond:
            self.processControllerProxies[proxy.ice_getIdentity()] = proxy
            conn = proxy.ice_getConnection()
            if(hasattr(conn, "setCloseCallback")):
                proxy.ice_getConnection().setCloseCallback(lambda conn : self.clearProcessController(proxy, conn))
            else:
                import Ice
                class CallbackI(Ice.ConnectionCallback):
                    def __init__(self, registry):
                        self.registry = registry

                    def heartbeath(self, conn):
                        pass

                    def closed(self, conn):
                        self.registry.clearProcessController(proxy, conn)

                proxy.ice_getConnection().setCallback(CallbackI(self))

            self.cond.notifyAll()

    def clearProcessController(self, proxy, conn):
        with self.cond:
            if proxy.ice_getIdentity() in self.processControllerProxies:
                if conn == self.processControllerProxies[proxy.ice_getIdentity()].ice_getCachedConnection():
                    del self.processControllerProxies[proxy.ice_getIdentity()]

    def startControllerApp(self, current, ident):
        pass

    def stopControllerApp(self, ident):
        pass

    def start(self, process, current, args, props, envs, watchDog):
        # Get the process controller
        processController = self.getController(current)

        # TODO: support envs?

        exe = process.getExe(current)
        args = ["--{0}={1}".format(k, val(v, quoteValue=False)) for k,v in props.items()] + [val(a) for a in args]
        if current.driver.debug:
            current.writeln("(executing `{0}/{1}' on `{2}' args = {3})".format(current.testsuite, exe, self, args))
        prx = processController.start(str(current.testsuite), exe, args)

        # Create bi-dir proxy in case we're talking to a bi-bir process controller.
        if self.adapter:
            prx = processController.ice_getConnection().createProxy(prx.ice_getIdentity())
        import Test
        return RemoteProcessController.Process(exe, Test.Common.ProcessPrx.uncheckedCast(prx))

    def destroy(self, driver):
        if driver.controllerApp:
            for ident in self.controllerApps:
                self.stopControllerApp(ident)
            self.controllerApps = []
        if self.adapter:
            self.adapter.destroy()

class iOSSimulatorProcessController(RemoteProcessController):

    device = "iOSSimulatorProcessController"
    deviceID = "com.apple.CoreSimulator.SimDeviceType.iPhone-6"
    runtimeID = "com.apple.CoreSimulator.SimRuntime.iOS-10-2"
    appPath = "ios/controller/build/Products"

    def __init__(self, current):
        RemoteProcessController.__init__(self, current)
        self.simulatorID = None

    def __str__(self):
        return "iOS Simulator"

    def getControllerIdentity(self, current):
        if isinstance(current.testcase.getMapping(), ObjCMapping):
            if current.config.arc:
                return "iPhoneSimulator/com.zeroc.ObjC-ARC-Test-Controller"
            else:
                return "iPhoneSimulator/com.zeroc.ObjC-Test-Controller"
        else:
            assert(isinstance(current.testcase.getMapping(), CppMapping))
            if current.config.cpp11:
                return "iPhoneSimulator/com.zeroc.Cpp11-Test-Controller"
            else:
                return "iPhoneSimulator/com.zeroc.Cpp98-Test-Controller"

    def startControllerApp(self, current, ident):
        mapping = current.testcase.getMapping()
        if isinstance(mapping, ObjCMapping):
            appName = "Objective-C ARC Test Controller.app" if current.config.arc else "Objective-C Test Controller.app"
        else:
            assert(isinstance(mapping, CppMapping))
            appName = "C++11 Test Controller.app" if current.config.cpp11 else "C++98 Test Controller.app"

        sys.stdout.write("launching simulator... ")
        sys.stdout.flush()
        try:
            run("xcrun simctl boot \"{0}\"".format(self.device))
        except Exception as ex:
            if str(ex).find("Booted") >= 0:
                pass
            elif str(ex).find("Invalid device") >= 0:
                # Create the simulator device if it doesn't exist
                self.simulatorID = run("xcrun simctl create \"{0}\" {1} {2}".format(self.device, self.deviceID, self.runtimeID))
                run("xcrun simctl boot \"{0}\"".format(self.device))
            else:
                raise
        print("ok")

        sys.stdout.write("launching {0}... ".format(appName))
        sys.stdout.flush()
        path = os.path.join(mapping.getTestsPath(), self.appPath, "Debug-iphonesimulator", appName)
        if not os.path.exists(path):
            path = os.path.join(mapping.getTestsPath(), self.appPath, "Release-iphonesimulator", appName)
        if not os.path.exists(path):
            raise RuntimeError("couldn't find iOS simulator controller application, did you build it?")
        run("xcrun simctl install \"{0}\" \"{1}\"".format(self.device, path))
        run("xcrun simctl launch \"{0}\" {1}".format(self.device, ident.name))
        print("ok")

    def stopControllerApp(self, ident):
        try:
            run("xcrun simctl uninstall \"{0}\" {1}".format(self.device, ident.name))
        except:
            pass

    def destroy(self, driver):
        RemoteProcessController.destroy(self, driver)
        if self.simulatorID:
            sys.stdout.write("destroying simulator... ")
            sys.stdout.flush()
            try:
                run("xcrun simctl shutdown \"{0}\"".format(self.simulatorID))
            except:
                pass
            try:
                run("xcrun simctl delete \"{0}\"".format(self.simulatorID))
            except:
                pass
            print("ok")

class iOSDeviceProcessController(RemoteProcessController):

    appPath = "cpp/test/ios/controller/build/Products"

    def __init__(self, current):
        RemoteProcessController.__init__(self, current)

    def __str__(self):
        return "iOS Device"

    def getControllerIdentity(self, current):
        if isinstance(current.testcase.getMapping(), ObjCMapping):
            return "iPhoneOS/com.zeroc.ObjC-Test-Controller"
        else:
            assert(isinstance(current.testcase.getMapping(), CppMapping))
            if current.config.cpp11:
                return "iPhoneOS/com.zeroc.Cpp11-Test-Controller"
            else:
                return "iPhoneOS/com.zeroc.Cpp98-Test-Controller"

    def startControllerApp(self, current, ident):
        # TODO: use ios-deploy to deploy and run the application on an attached device?
        pass

    def stopControllerApp(self, ident):
        pass

class UWPProcessController(RemoteProcessController):

    def __init__(self, current):
        RemoteProcessController.__init__(self, current, "tcp -h 127.0.0.1 -p 15001")
        self.name = "ice-uwp-controller"
        self.appUserModelId = "ice-uwp-controller_3qjctahehqazm"

    def __str__(self):
        return "UWP"

    def getControllerIdentity(self, current):
        return "UWP/ProcessController"

    def startControllerApp(self, current, ident):
        platform = current.config.buildPlatform
        config = current.config.buildConfig
        layout = os.path.join(toplevel, "cpp", "test", platform, config, "AppX")

        self.packageFullName = "{0}_1.0.0.0_{1}__3qjctahehqazm".format(
            self.name, "x86" if platform == "Win32" else platform)

        prefix = "controller_1.0.0.0_{0}{1}".format(platform, "{0}_".format(config) if config == "Debug" else "")
        package = os.path.join(toplevel, "cpp", "msbuild", "AppPackages", "controller",
            "{0}_Test".format(prefix), "{0}.appx".format(prefix))

        #
        # If the application is already installed remove it, this will also take care
        # of closing it.
        #
        if self.name in run("powershell Get-AppxPackage -Name {0}".format(self.name)):
            run("powershell Remove-AppxPackage {0}".format(self.packageFullName))

        #
        # Remove any previous package we have extracted to ensure we use a
        # fresh build
        #
        if os.path.exists(layout):
            shutil.rmtree(layout)
        os.makedirs(layout)

        print("Unpackaing package: {0} to {1}....".format(os.path.basename(package), layout))
        run("MakeAppx.exe unpack /p \"{0}\" /d \"{1}\" /l".format(package, layout))

        print("Registering application to run from layout...")
        run("powershell Add-AppxPackage -Register \"{0}/AppxManifest.xml\"".format(layout))

        run("CheckNetIsolation LoopbackExempt -a -n={0}".format(self.appUserModelId))

        #
        # microsoft.windows.softwarelogo.appxlauncher.exe returns the PID as return code
        # and 0 on case of failures. We pass err=True to run to handle this.
        #
        print("staring UWP controller app...")
        run('"{0}" {1}!App'.format(
            "C:/Program Files (x86)/Windows Kits/10/App Certification Kit/microsoft.windows.softwarelogo.appxlauncher.exe",
            self.appUserModelId), err=True)

    def stopControllerApp(self, ident):
        try:
            run("powershell Remove-AppxPackage {0}".format(self.packageFullName))
            run("CheckNetIsolation LoopbackExempt -c -n={0}".format(self.appUserModelId))
        except:
            pass

class BrowserProcessController(RemoteProcessController):

    def __init__(self, current):
        RemoteProcessController.__init__(self, current, "ws -h 127.0.0.1 -p 15002:wss -h 127.0.0.1 -p 15003")

        try:
            from selenium import webdriver
            if not hasattr(webdriver, current.config.browser):
                raise RuntimeError("unknown browser `{0}'".format(current.config.browser))

            if current.config.browser == "Firefox":
                #
                # We need to specify a profile for Firefox. This profile only provides the cert8.db which
                # contains our Test CA cert. It should be possible to avoid this by setting the webdriver
                # acceptInsecureCerts capability but it's only supported by latest Firefox releases.
                #
                # capabilities = webdriver.DesiredCapabilities.FIREFOX.copy()
                # capabilities["marionette"] = True
                # capabilities["acceptInsecureCerts"] = True
                # capabilities["moz:firefoxOptions"] = {}
                # capabilities["moz:firefoxOptions"]["binary"] = "/Applications/FirefoxNightly.app/Contents/MacOS/firefox-bin"
                profile = webdriver.FirefoxProfile(os.path.join(toplevel, "scripts", "selenium", "firefox"))
                self.driver = webdriver.Firefox(firefox_profile=profile)
            else:
                self.driver = getattr(webdriver, current.config.browser)()

            cmd = "node -e \"require('./bin/HttpServer')()\"";
            cwd = current.testsuite.getMapping().getPath()
            self.httpServer = Expect.Expect(cmd, cwd=cwd)
            self.httpServer.expect("listening on ports")
        except:
            self.destroy(current.driver)
            raise

    def __str__(self):
        return str(self.driver)

    def getControllerIdentity(self, current):

        #
        # Load the controller page each time we're asked for the controller, the controller page
        # will connect to the process controller registry to register itself with this script.
        #
        testsuite = ("es5/" if current.config.es5 else "") + str(current.testsuite)
        if current.config.protocol == "wss":
            protocol = "https"
            port = "9090"
            cport = "15003"
        else:
            protocol = "http"
            port = "8080"
            cport = "15002"

        self.driver.get("{0}://127.0.0.1:{1}/test/{2}/controller.html?port={3}&worker={4}".format(protocol,
                                                                                                  port,
                                                                                                  testsuite,
                                                                                                  cport,
                                                                                                  current.config.worker))
        return "Browser/ProcessController"

    def destroy(self, driver):
        if self.httpServer:
            self.httpServer.terminate()
            self.httpServer = None

        try:
            self.driver.quit()
        except:
            pass

class Driver:

    class Current:

        def __init__(self, driver, testsuite, result):
            self.driver = driver
            self.testsuite = testsuite
            self.config = driver.configs[testsuite.getMapping()]
            self.result = result
            self.host = None
            self.testcase = None
            self.testcases = []

        def getTestEndpoint(self, *args, **kargs):
            return self.driver.getTestEndpoint(*args, **kargs)

        def getBuildDir(self, name):
            return self.testcase.getMapping().getBuildDir(name, self)

        def getPluginEntryPoint(self, plugin, process):
            return self.testcase.getMapping().getPluginEntryPoint(plugin, process, self)

        def write(self, *args, **kargs):
            self.result.write(*args, **kargs)

        def writeln(self, *args, **kargs):
            self.result.writeln(*args, **kargs)

        def push(self, testcase, host=None):
            if not testcase.mapping:
                assert(not testcase.parent and not testcase.testsuite)
                testcase.mapping = self.testcase.getMapping()
                testcase.testsuite = self.testcase.getTestSuite()
                testcase.parent = self.testcase
            self.testcases.append((self.testcase, self.config, self.host))
            self.testcase = testcase
            self.config = self.driver.configs[self.testcase.getMapping()].cloneAndOverrideWith(self)
            self.host = host

        def pop(self):
            assert(self.testcase)
            testcase = self.testcase
            (self.testcase, self.config, self.host) = self.testcases.pop()
            if testcase.parent and self.testcase != testcase:
                testcase.mapping = None
                testcase.testsuite = None
                testcase.parent = None

    drivers = {}
    driver = "local"

    @classmethod
    def add(self, name, driver, default=False):
        if default:
            Driver.driver = name
        self.driver = name
        self.drivers[name] = driver

    @classmethod
    def getAll(self):
        return list(self.drivers.values())

    @classmethod
    def create(self, options):
        parseOptions(self, options)
        driver = self.drivers.get(self.driver)
        if not driver:
            raise RuntimeError("unknown driver `{0}'".format(self.driver))
        return driver(options)

    @classmethod
    def getOptions(self):
        return ("dlrR", ["debug", "driver=", "filter=", "rfilter=", "host=", "host-ipv6=", "host-bt=", "interface=",
                         "controller-app"])

    @classmethod
    def usage(self):
        pass

    @classmethod
    def commonUsage(self):
        print("")
        print("Driver options:")
        print("-d | --debug          Verbose information.")
        print("--driver=<driver>     Use the given driver (local, client, server or remote).")
        print("--filter=<regex>      Run all the tests that match the given regex.")
        print("--rfilter=<regex>     Run all the tests that do not match the given regex.")
        print("--host=<addr>         The IPv4 address to use for Ice.Default.Host.")
        print("--host-ipv6=<addr>    The IPv6 address to use for Ice.Default.Host.")
        print("--host-bt=<addr>      The Bluetooth address to use for Ice.Default.Host.")
        print("--interface=<IP>      The multicast interface to use to discover controllers.")
        print("--controller-app      Start the process controller application.")

    def __init__(self, options):
        self.debug = False
        self.filters = []
        self.rfilters = []
        self.host = ""
        self.hostIPv6 = ""
        self.hostBT = ""
        self.controllerApp = False

        self.failures = []
        parseOptions(self, options, { "d": "debug",
                                      "r" : "filters",
                                      "R" : "rfilters",
                                      "filter" : "filters",
                                      "rfilter" : "rfilters",
                                      "host-ipv6" : "hostIPv6",
                                      "host-bt" : "hostBT",
                                      "controller-app" : "controllerApp"})

        self.filters = [re.compile(a) for a in self.filters]
        self.rfilters = [re.compile(a) for a in self.rfilters]

        self.communicator = None
        self.interface = ""
        self.processControllers = {}

    def setConfigs(self, configs):
        self.configs = configs

    def useIceBinDist(self, mapping):
        env = os.environ.get("ICE_BIN_DIST", "").split()
        return 'all' in env or mapping in env

    def getIceDir(self, mapping, current):
        if self.useIceBinDist(mapping):
            return platform.getIceInstallDir(mapping, current)
        elif mapping:
            return mapping.getPath()
        else:
            return toplevel

    def getSliceDir(self, mapping, current):
        return platform.getSliceDir(self.getIceDir(mapping, current) if self.useIceBinDist(mapping) else toplevel)

    def isWorkerThread(self):
        return False

    def getTestEndpoint(self, portnum, protocol="default"):
        return "{0} -p {1}".format(protocol, self.getTestPort(portnum))

    def getTestPort(self, portnum):
        return 12010 + portnum

    def getArgs(self, process, current):
        ### Return driver specific arguments
        return []

    def getProps(self, process, current):
        props = {}
        if isinstance(process, IceProcess):
            if not self.host:
                props["Ice.Default.Host"] = "0:0:0:0:0:0:0:1" if current.config.ipv6 else "127.0.0.1"
            else:
                props["Ice.Default.Host"] = self.host
        return props

    def getMappings(self):
        ### Return additional mappings to load required by the driver
        return []

    def getCommunicator(self):
        self.initCommunicator()
        return self.communicator

    def initCommunicator(self):
        if self.communicator:
            return

        try:
            import Ice
        except ImportError:
            # Try to add the local Python build to the sys.path
            pythonMapping = Mapping.getByName("python")
            if pythonMapping:
                for p in pythonMapping.getPythonDirs(pythonMapping.getPath(), self.configs[pythonMapping]):
                    sys.path.append(p)

        import Ice
        Ice.loadSlice(os.path.join(toplevel, "scripts", "Controller.ice"))

        initData = Ice.InitializationData()
        initData.properties = Ice.createProperties()

        # Load IceSSL, this is useful to talk with WSS for JavaScript
        initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL")
        initData.properties.setProperty("IceSSL.DefaultDir", os.path.join(toplevel, "certs"))
        initData.properties.setProperty("IceSSL.CertFile", "server.p12")
        initData.properties.setProperty("IceSSL.Password", "password")
        initData.properties.setProperty("IceSSL.Keychain", "test.keychain")
        initData.properties.setProperty("IceSSL.KeychainPassword", "password")
        initData.properties.setProperty("IceSSL.VerifyPeer", "0");

        initData.properties.setProperty("Ice.Plugin.IceDiscovery", "IceDiscovery:createIceDiscovery")
        initData.properties.setProperty("IceDiscovery.DomainId", "TestController")
        initData.properties.setProperty("IceDiscovery.Interface", self.interface or "127.0.0.1")
        initData.properties.setProperty("Ice.Default.Host", self.interface or "127.0.0.1")
        initData.properties.setProperty("Ice.ThreadPool.Server.Size", "10")
        #initData.properties.setProperty("Ice.Trace.Protocol", "1")
        #initData.properties.setProperty("Ice.Trace.Network", "2")
        initData.properties.setProperty("Ice.Override.Timeout", "10000")
        self.communicator = Ice.initialize(initData)

        self.ctrlCHandler = Ice.CtrlCHandler()

        def signal(sig):
            self.communicator.destroy()
        self.ctrlCHandler.setCallback(signal)

    def getProcessController(self, current):
        processController = None
        if current.config.buildPlatform == "iphonesimulator":
            processController = iOSSimulatorProcessController
        elif current.config.buildPlatform == "iphoneos":
            processController = iOSDeviceProcessController
        elif current.config.uwp:
            processController = UWPProcessController
        elif isinstance(current.testcase.getMapping(), JavaScriptMapping) and current.config.browser:
            processController = BrowserProcessController
        else:
            processController = LocalProcessController

        if processController in self.processControllers:
            return self.processControllers[processController]

        # Instantiate the controller
        self.processControllers[processController] = processController(current)
        return self.processControllers[processController]

    def getProcessProps(self, current, ready, readyCount):
        props = {}
        if ready or readyCount > 0:
            if current.config.buildPlatform not in ["iphonesimulator", "iphoneos"]:
                props["Ice.PrintAdapterReady"] = 1
        return props

    def destroy(self):
        for controller in self.processControllers.values():
            controller.destroy(self)

        if self.communicator:
            self.communicator.destroy()
            self.ctrlCHandler.destroy()


class CppMapping(Mapping):

    class Config(Mapping.Config):

        @classmethod
        def getOptions(self):
            return ("", ["cpp-config=", "cpp-platform="])

        @classmethod
        def usage(self):
            print("")
            print("C++ Mapping options:")
            print("--cpp-config=<config>     C++ build configuration for native executables (overrides --config).")
            print("--cpp-platform=<platform> C++ build platform for native executables (overrides --platform).")

        def __init__(self, options=[]):
            Mapping.Config.__init__(self, options)

            # Derive from the build config the cpp11 option. This is used by canRun to allow filtering
            # tests on the cpp11 value in the testcase options specification
            self.cpp11 = self.buildConfig.lower().find("cpp11") >= 0

            parseOptions(self, options, { "cpp-config" : "buildConfig", "cpp-platform" : "buildPlatform" })

        def canRun(self, current):
            if not Mapping.Config.canRun(self, current):
                return False

            # No C++11 tests for IceStorm, IceGrid, etc
            parent = re.match(r'^([\w]*).*', current.testcase.getTestSuite().getId()).group(1)
            if self.cpp11 and not parent in ["IceUtil", "Slice", "Ice", "IceSSL", "IceDiscovery", "IceBox"]:
                return False

            return True

    def getDefaultExe(self, processType, config):
        return platform.getDefaultExe(processType, config)

    def getProps(self, process, current):
        props = Mapping.getProps(self, process, current)
        if isinstance(process, IceProcess):
            props["Ice.NullHandleAbort"] = True
        return props

    def getSSLProps(self, process, current):
        props = Mapping.getSSLProps(self, process, current)
        server = isinstance(process, Server)
        uwp = current.config.buildPlatform == "UWP"

        props.update({
            "IceSSL.CAs": "cacert.pem",
            "IceSSL.CertFile": "server.p12" if server else "ms-appx:///client.p12" if uwp else "client.p12"
        })
        if isinstance(platform, Darwin):
            keychainFile = "server.keychain" if server else "client.keychain"
            props.update({
                "IceSSL.KeychainPassword" : "password",
                "IceSSL.Keychain": keychainFile
             })
        return props

    def getPluginEntryPoint(self, plugin, process, current):
        return {
            "IceSSL" : "IceSSL:createIceSSL",
            "IceBT" : "IceBT:createIceBT",
            "IceDiscovery" : "IceDiscovery:createIceDiscovery"
        }[plugin]

    def getEnv(self, process, current):

        #
        # On Windows, add the testcommon directories to the PATH
        #
        libPaths = []
        if isinstance(platform, Windows):
            libPaths.append(self.getLibDir(process, current))
            testcommon = os.path.join(self.path, "test", "Common")
            libPaths.append(os.path.join(testcommon, self.getBuildDir("testcommon", current)))

        #
        # Add the test suite library directories to the platform library path environment variable.
        #
        if current.testcase:
            for d in set([current.getBuildDir(d) for d in current.testcase.getTestSuite().getLibDirs()]):
                libPaths.append(d)

        env = {}
        if len(libPaths) > 0:
            env[platform.getLdPathEnvName()] = os.pathsep.join(libPaths)
        return env

    def getDefaultSource(self, processType):
        return {
            "client" : "Client.cpp",
            "server" : "Server.cpp",
            "serveramd" : "ServerAMD.cpp",
            "collocated" : "Collocated.cpp",
        }[processType]

class JavaMapping(Mapping):

    def getCommandLine(self, current, process, exe):
        javaHome = os.getenv("JAVA_HOME", "")
        java = os.path.join(javaHome, "bin", "java") if javaHome else "java"
        if process.isFromBinDir():
            return "{0} {1}".format(java, exe)

        assert(current.testcase.getPath().startswith(self.getTestsPath()))
        package = "test." + current.testcase.getPath()[len(self.getTestsPath()) + 1:].replace(os.sep, ".")
        javaArgs = self.getJavaArgs(process, current)
        if javaArgs:
            return "{0} {1} {2}.{3}".format(java, " ".join(javaArgs), package, exe)
        else:
            return "{0} {1}.{2}".format(java, package, exe)

    def getJavaArgs(self, process, current):
        return []

    def getSSLProps(self, process, current):
        props = Mapping.getSSLProps(self, process, current)
        props.update({
            "IceSSL.Keystore": "server.jks" if isinstance(process, Server) else "client.jks",
        })
        return props

    def getPluginEntryPoint(self, plugin, process, current):
        return {
            "IceSSL" : "com.zeroc.IceSSL.PluginFactory",
            "IceBT" : "com.zeroc.IceBT.PluginFactory",
            "IceDiscovery" : "com.zeroc.IceDiscovery.PluginFactory"
        }[plugin]

    def getEnv(self, process, current):
        return { "CLASSPATH" : os.path.join(self.path, "lib", "test.jar") }

    def getTestsPath(self):
        return os.path.join(self.path, "test/src/main/java/test")

    def getDefaultSource(self, processType):
        return self.getDefaultExe(processType) + ".java"

    def getDefaultExe(self, processType, config=None):
        return {
            "client" : "Client",
            "server" : "Server",
            "serveramd" : "AMDServer",
            "collocated" : "Collocated",
            "icebox": "com.zeroc.IceBox.Server",
            "iceboxadmin" : "com.zeroc.IceBox.Admin",
        }[processType]

class JavaCompatMapping(JavaMapping):

    def getPluginEntryPoint(self, plugin, process, current):
        return {
            "IceSSL" : "IceSSL.PluginFactory",
            "IceBT" : "IceBT.PluginFactory",
            "IceDiscovery" : "IceDiscovery.PluginFactory"
        }[plugin]

    def getDefaultExe(self, processType, config=None):
        return {
            "client" : "Client",
            "server" : "Server",
            "serveramd" : "AMDServer",
            "collocated" : "Collocated",
            "icebox": "IceBox.Server",
            "iceboxadmin" : "IceBox.Admin",
        }[processType]

class CSharpMapping(Mapping):

    def getBuildDir(self, name, current):
        return os.path.join("msbuild", name)

    def getSSLProps(self, process, current):
        props = Mapping.getSSLProps(self, process, current)
        props.update({
            "IceSSL.Password": "password",
            "IceSSL.DefaultDir": os.path.join(toplevel, "certs"),
            "IceSSL.CAs": "cacert.pem",
            "IceSSL.VerifyPeer": "0" if current.config.protocol == "wss" else "2",
            "IceSSL.CertFile": "server.p12" if isinstance(process, Server) else "client.p12",
        })
        return props

    def getPluginEntryPoint(self, plugin, process, current):
        plugindir = "{0}/{1}".format(current.driver.getIceDir(self, current),
                                     "lib" if current.driver.useIceBinDist(self) else "Assemblies")
        return {
            "IceSSL" : plugindir + "/IceSSL.dll:IceSSL.PluginFactory",
            "IceDiscovery" : plugindir + "/IceDiscovery.dll:IceDiscovery.PluginFactory"
        }[plugin]

    def getEnv(self, process, current):
        if current.driver.useIceBinDist(self):
            bzip2 = os.path.join(platform.getIceInstallDir(self, current), "tools")
            assembliesDir = os.path.join(platform.getIceInstallDir(self, current), "lib")
        else:
            bzip2 = os.path.join(toplevel, "cpp", "msbuild", "packages",
                                 "bzip2.{0}.1.0.6.4".format(platform.getCompiler()),
                                 "build", "native", "bin", "x64", "Release")
            assembliesDir = os.path.join(current.driver.getIceDir(self, current), "Assemblies")
        return { "DEVPATH" : assembliesDir, "PATH" : bzip2 };

    def getDefaultSource(self, processType):
        return {
            "client" : "Client.cs",
            "server" : "Server.cs",
            "serveramd" : "ServerAMD.cs",
            "collocated" : "Collocated.cs",
        }[processType]

    def getDefaultExe(self, processType, config):
        return "iceboxnet" if processType == "icebox" else processType

class CppBasedMapping(Mapping):

    class Config(Mapping.Config):

        @classmethod
        def getOptions(self):
            return ("", [self.mappingName + "-config=", self.mappingName + "-platform="])

        @classmethod
        def usage(self):
            print("")
            print(self.mappingDesc + " mapping options:")
            print("--{0}-config=<config>     {1} build configuration for native executables (overrides --config)."
                .format(self.mappingName, self.mappingDesc))
            print("--{0}-platform=<platform> {1} build platform for native executables (overrides --platform)."
                .format(self.mappingName, self.mappingDesc))

        def __init__(self, options=[]):
            Mapping.Config.__init__(self, options)
            parseOptions(self, options,
                { self.mappingName + "-config" : "buildConfig",
                  self.mappingName + "-platform" : "buildPlatform" })

    def getSSLProps(self, process, current):
        return Mapping.getByName("cpp").getSSLProps(process, current)

    def getPluginEntryPoint(self, plugin, process, current):
        return Mapping.getByName("cpp").getPluginEntryPoint(plugin, process, current)

    def getEnv(self, process, current):
        env = Mapping.getEnv(self, process, current)
        if current.driver.getIceDir(self, current) != platform.getIceInstallDir(self, current):
            # If not installed in the default platform installation directory, add
            # the Ice C++ library directory to the library path
            env[platform.getLdPathEnvName()] = Mapping.getByName("cpp").getLibDir(process, current)
        return env

class ObjCMapping(CppBasedMapping):

    class Config(CppBasedMapping.Config):
        mappingName = "objc"
        mappingDesc = "Objective-C"

        def __init__(self, options=[]):
            Mapping.Config.__init__(self, options)
            self.arc = self.buildConfig.lower().find("arc") >= 0

    def getDefaultSource(self, processType):
        return {
            "client" : "Client.m",
            "server" : "Server.m",
            "collocated" : "Collocated.m",
        }[processType]

class PythonMapping(CppBasedMapping):

    class Config(CppBasedMapping.Config):
        mappingName = "python"
        mappingDesc = "Python"

    def getCommandLine(self, current, process, exe):
        return "\"{0}\" {1}".format(sys.executable, exe)

    def getEnv(self, process, current):
        env = CppBasedMapping.getEnv(self, process, current)
        if current.driver.getIceDir(self, current) != platform.getIceInstallDir(self, current):
            # If not installed in the default platform installation directory, add
            # the Ice python directory to PYTHONPATH
            dirs = self.getPythonDirs(current.driver.getIceDir(self, current), current.config)
            env["PYTHONPATH"] = os.pathsep.join(dirs)
        return env

    def getPythonDirs(self, iceDir, config):
        dirs = []
        if isinstance(platform, Windows):
            dirs.append(os.path.join(iceDir, "python", config.buildPlatform, config.buildConfig))
        dirs.append(os.path.join(iceDir, "python"))
        return dirs

    def getDefaultExe(self, processType, config):
        return self.getDefaultSource(processType)

    def getDefaultSource(self, processType):
        return {
            "client" : "Client.py",
            "server" : "Server.py",
            "serveramd" : "ServerAMD.py",
            "collocated" : "Collocated.py",
        }[processType]

class CppBasedClientMapping(CppBasedMapping):

    def loadTestSuites(self, tests, config, filters, rfilters):
        Mapping.loadTestSuites(self, tests, config, filters, rfilters)
        self.getServerMapping().loadTestSuites(self.testsuites.keys(), config)

    def getServerMapping(self):
        return Mapping.getByName("cpp") # By default, run clients against C++ mapping executables

    def getDefaultExe(self, processType, config):
        return self.getDefaultSource(processType)

class RubyMapping(CppBasedClientMapping):

    class Config(CppBasedClientMapping.Config):
        mappingName = "ruby"
        mappingDesc = "Ruby"

    def getCommandLine(self, current, process, exe):
        return "ruby " + exe

    def getEnv(self, process, current):
        env = CppBasedMapping.getEnv(self, process, current)
        if current.driver.getIceDir(self, current) != platform.getIceInstallDir(self, current):
            # If not installed in the default platform installation directory, add
            # the Ice ruby directory to RUBYLIB
            env["RUBYLIB"] = os.path.join(self.path, "ruby")
        return env

    def getDefaultSource(self, processType):
        return { "client" : "Client.rb" }[processType]

class PhpMapping(CppBasedClientMapping):

    class Config(CppBasedClientMapping.Config):
        mappingName = "php"
        mappingDesc = "PHP"

    def getEnv(self, process, current):
        env = CppBasedMapping.getEnv(self, process, current)
        if isinstance(platform, Windows) and current.driver.useIceBinDist(self):
            env[platform.getLdPathEnvName()] = self.getBinDir(process, current)
        return env

    def getCommandLine(self, current, process, exe):
        args = []
        if current.driver.getIceDir(self, current) == platform.getIceInstallDir(self, current):
            #
            # If installed in the platform system directory and on Linux, we rely
            # on ice.ini to find the extension. On OS X, we still need to setup
            # the properties.
            #
            if(isinstance(platform, Darwin)):
                args += ["-n"] # Do not load any php.ini files
                args += ["-d", "extension_dir=/usr/local/lib/php/extensions"]
                args += ["-d", "include_path=/usr/local/share/php"]
                args += ["-d", "extension=IcePHP.so"]
        else:
            useBinDist = current.driver.useIceBinDist(self)
            if isinstance(platform, Windows):
                extension = "php_ice_nts.dll" if "NTS" in run("php -v") else "php_ice.dll"
                extensionDir = self.getBinDir(process, current)
                includePath = self.getLibDir(process, current)
            else:
                extension = "IcePHP.so"
                extensionDir = self.getLibDir(process, current)
                includePath = "{0}/{1}".format(current.driver.getIceDir(self, current), "php" if useBinDist else "lib")

            args += ["-n"] # Do not load any php.ini files
            args += ["-d", "extension_dir='{0}'".format(extensionDir)]
            args += ["-d", "extension='{0}'".format(extension)]
            args += ["-d", "include_path='{0}'".format(includePath)]
        if hasattr(process, "getPhpArgs"):
            args += process.getPhpArgs(current)
        return "php {0} -f {1} -- ".format(" ".join(args), exe)

    def getDefaultSource(self, processType):
        return { "client" : "Client.php" }[processType]

class JavaScriptMapping(Mapping):

    class Config(Mapping.Config):

        @classmethod
        def getOptions(self):
            return ("", ["es5", "browser=", "worker"])

        @classmethod
        def usage(self):
            print("")
            print("JavaScript mapping options:")
            print("--es5                 Use JavaScript ES5 (Babel compiled code).")
            print("--browser=<name>      Run with the given browser.")
            print("--worker              Run with Web workers enabled.")

        def __init__(self, options=[]):
            Mapping.Config.__init__(self, options)
            self.es5 = False
            self.browser = ""
            self.worker = False
            parseOptions(self, options)
            if self.browser and self.protocol == "tcp":
                self.protocol = "ws"
            if self.browser in ["Edge", "Ie"]:
                self.es5 = True

    def loadTestSuites(self, tests, config, filters, rfilters):
        Mapping.loadTestSuites(self, tests, config, filters, rfilters)
        self.getServerMapping().loadTestSuites(list(self.testsuites.keys()) + ["Ice/echo"], config, filters, rfilters)

    def getServerMapping(self):
        return Mapping.getByName("cpp") # By default, run clients against C++ mapping executables

    def getCommandLine(self, current, process, exe):
        if current.config.es5:
            return "node {0}/test/Common/run.js --es5 {1}".format(self.path, exe)
        else:
            return "node {0}/test/Common/run.js {1}".format(self.path, exe)

    def getDefaultSource(self, processType):
        return { "client" : "Client.js", "clientBidir" : "ClientBidir.js" }[processType]

    def getDefaultExe(self, processType, config=None):
        return self.getDefaultSource(processType).replace(".js", "")

    def getEnv(self, process, current):
        env = Mapping.getEnv(self, process, current)
        env["NODE_PATH"] = self.getTestCwd(process, current)
        return env

    def getSSLProps(self, process, current):
        return {}

    def getTestCwd(self, process, current):
        if current.config.es5:
            # Change to the ES5 test directory if testing ES5
            return os.path.join(self.path, "test", "es5", current.testcase.getTestSuite().getId())
        else:
            return os.path.join(self.path, "test", current.testcase.getTestSuite().getId())

    def computeTestCases(self, testId, files):
        if testId.find("es5") > -1:
            return # Ignore es5 directories
        return Mapping.computeTestCases(self, testId, files)

    def getOptions(self, current):
        options = {
            "protocol" : ["ws", "wss"] if current.config.browser else ["tcp"],
            "compress" : [False],
            "ipv6" : [False],
            "serialize" : [False],
            "mx" : [False],
            "es5" : [False, True],
            "worker" : [False, True] if current.config.browser else [False],
        }

        # Edge and Ie only support ES5 for now
        if current.config.browser in ["Edge", "Ie"]:
            options["es5"] = [True]

        # TODO: Fix Safari issue where tests hang when ran with --worker
        if current.config.browser == "Safari":
            options["worker"] = [False]

        return options

from Glacier2Util import *
from IceBoxUtil import *
from IcePatch2Util import *
from IceGridUtil import *
from IceStormUtil import *
from LocalDriver import *

#
# Supported mappings
#
Mapping.add("cpp", CppMapping())
Mapping.add("java", JavaMapping())
Mapping.add("java-compat", JavaCompatMapping())
Mapping.add("python", PythonMapping())
Mapping.add("ruby", RubyMapping())
Mapping.add("php", PhpMapping())
Mapping.add("js", JavaScriptMapping())
if isinstance(platform, Windows):
    Mapping.add("csharp", CSharpMapping())
if isinstance(platform, Darwin):
    Mapping.add("objective-c", ObjCMapping())

def runTestsWithPath(path):
    runTests([Mapping.getByPath(path)])

def runTests(mappings=None, drivers=None):
    if not mappings:
        mappings = Mapping.getAll()
    if not drivers:
        drivers = Driver.getAll()

    #
    # All mappings contains all the mappings necessary to run the tests from the given mappings. Some
    # mappings depend on other mappings for running (e.g.: Ruby needs the C++ mapping).
    #
    allMappings = list(set([m.getClientMapping() for m in mappings] + [m.getServerMapping() for m in mappings]))

    def usage():
        print("Usage: " + sys.argv[0] + " [options] [tests]")
        print("")
        print("Options:")
        print("-h | --help        Show this message")

        Driver.commonUsage()
        for driver in drivers:
            driver.usage()

        Mapping.Config.commonUsage()
        for mapping in allMappings:
            mapping.Config.usage()

        print("")

    driver = None
    try:
        options = [Driver.getOptions(), Mapping.Config.getOptions()]
        options += [driver.getOptions() for driver in drivers]
        options += [mapping.Config.getOptions() for mapping in Mapping.getAll()]
        shortOptions = "h"
        longOptions = ["help"]
        for so, lo in options:
            shortOptions += so
            longOptions += lo
        opts, args = getopt.gnu_getopt(sys.argv[1:], shortOptions, longOptions)

        for (o, a) in opts:
            if o in ["-h", "--help"]:
                usage()
                sys.exit(0)

        #
        # Create the driver
        #
        driver = Driver.create(opts)

        #
        # Create the configurations for each mapping (we always parse the configuration for the
        # python mapping because we might use the local IcePy build to initialize a communicator).
        #
        configs = {}
        for mapping in allMappings + driver.getMappings() + [Mapping.getByName("python")]:
            if mapping not in configs:
                configs[mapping] = mapping.createConfig(opts[:])

        #
        # Provide the configurations to the driver and load the test suites for each mapping.
        #
        driver.setConfigs(configs)
        for mapping in mappings + driver.getMappings():
            mapping.loadTestSuites(args, configs[mapping], driver.filters, driver.rfilters)

        #
        # Finally, run the test suites with the driver.
        #
        try:
            sys.exit(driver.run(mappings, args))
        except KeyboardInterrupt:
            pass
        finally:
            driver.destroy()

    except Exception as e:
        print(sys.argv[0] + ": unexpected exception raised:\n" + traceback.format_exc())
        sys.exit(1)
