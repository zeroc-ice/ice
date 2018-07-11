# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os

from Util import *

class Ice(Component):

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

    def __init__(self):
        self.nugetVersion = None

    def useBinDist(self, mapping, current):
        return Component._useBinDist(self, mapping, current, "ICE_BIN_DIST")

    def getInstallDir(self, mapping, current):
        # On Windows, the Ice MSI installation can only be used for C++
        envHomeName = None if isinstance(platform, Windows) and not isinstance(mapping, CppMapping) else "ICE_HOME"
        return Component._getInstallDir(self, mapping, current, envHomeName)

    def getPhpExtension(self, mapping, current):
        if isinstance(platform, Windows):
            return "php_ice.dll" if current.driver.configs[mapping].buildConfig in ["Debug", "Release"] else "php_ice_nts.dll"
        else:
            return "ice.so"

    def getNugetPackage(self, mapping, compiler=None):
        if isinstance(mapping, CSharpMapping):
            return "zeroc.ice.net"
        else:
            return "zeroc.ice.{0}".format(compiler)

    def getNugetPackageVersion(self, mapping):
        if not self.nugetVersion:
            if isinstance(mapping, CSharpMapping):
                with open(os.path.join(toplevel, "csharp", "msbuild", "zeroc.ice.net.nuspec"), "r") as configFile:
                    self.nugetVersion = re.search("<version>(.*)</version>", configFile.read()).group(1)
            else:
                with open(os.path.join(toplevel, "config", "icebuilder.props"), "r") as configFile:
                    self.nugetVersion = re.search("<IceJSONVersion>(.*)</IceJSONVersion>", configFile.read()).group(1)
        return self.nugetVersion

    def getFilters(self, mapping, config):
        if "xcodesdk" in config.buildConfig:
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
        elif "static" in config.buildConfig:
            return (["Ice/.*", "IceSSL/configuration", "IceDiscovery/simple", "IceGrid/simple", "Glacier2/application"],
                    ["Ice/library", "Ice/plugin"])
        elif config.uwp:
            return (["Ice/.*", "IceSSL/configuration"],
                    ["Ice/background",
                     "Ice/echo",
                     "Ice/faultTolerance",
                     "Ice/gc",
                     "Ice/library",
                     "Ice/logger",
                     "Ice/networkProxy",        # SOCKS proxy not supported with UWP
                     "Ice/properties",          # Property files are not supported with UWP
                     "Ice/plugin",
                     "Ice/threadPoolPriority"])
        elif isinstance(platform, Windows) and platform.getCompiler() in ["VC100"]:
            return (["Ice/.*", "IceSSL/.*", "IceBox/.*", "IceDiscovery/.*", "IceUtil/.*", "Slice/.*"], [])
        elif isinstance(mapping, AndroidMappingMixin):
            return (["Ice/.*"],
                    ["Ice/hash",
                     "Ice/faultTolerance",
                     "Ice/metrics",
                     "Ice/networkProxy",
                     "Ice/throughput",
                     "Ice/plugin",
                     "Ice/logger",
                     "Ice/properties"])
        return ([], [])

    def canRun(self, testId, mapping, current):
        parent = re.match(r'^([\w]*).*', testId).group(1)
        if isinstance(platform, Linux):
            if platform.getLinuxId() in ["centos", "rhel", "fedora"] and current.config.buildPlatform == "x86":
                #
                # Don't test Glacier2/IceStorm/IceGrid services with multilib platforms. We only
                # build services for the native platform.
                #
                if parent in ["Glacier2", "IceStorm", "IceGrid"]:
                    return False
        elif isinstance(platform, Linux):
            #
            # On Windows, if testing with a binary distribution, don't test Glacier2/IceBridge services
            # with the Debug configurations since we don't provide binaries for them.
            #
            if self.useBinDist(mapping, current):
                if parent in ["Glacier2", "IceBridge"] and current.config.buildConfig.find("Debug") >= 0:
                    return False

        # No C++11 tests for IceStorm, IceGrid, etc
        if isinstance(mapping, CppMapping) and current.config.cpp11:
            if parent in ["IceStorm", "IceBridge"]:
                return False
            elif parent in ["IceGrid"] and testId not in ["IceGrid/simple"]:
                return False
            elif parent in ["Glacier2"] and testId not in ["Glacier2/application", "Glacier2/sessionHelper"]:
                return False

        if isinstance(mapping, CSharpMapping) and current.config.netframework:
            #
            # The following tests require multicast, on Unix platforms it's currently only supported
            # with IPv4 due to .NET Core bug https://github.com/dotnet/corefx/issues/25525
            #
            if not isinstance(platform, Windows) and current.config.ipv6 and testId in ["Ice/udp",
                                                                                        "IceDiscovery/simple",
                                                                                        "IceGrid/simple"]:
                return False

            if isinstance(platform, Darwin):
                if parent in ["IceSSL", "IceDiscovery"]:
                    return False

                # TODO: Remove once https://github.com/dotnet/corefx/issues/28759 is fixed
                if testId == "Ice/adapterDeactivation" and current.config.protocol in ["ssl", "wss"]:
                    return False

        return True

    def isMainThreadOnly(self, testId):
        return testId.startswith("IceStorm") # TODO: WORKAROUND for ICE-8175

    def getDefaultProcesses(self, mapping, processType, testId):
        if testId.startswith("IceUtil") or testId.startswith("Slice"):
            return [SimpleClient()]
        elif testId.startswith("IceGrid"):
            if processType in ["client", "collocated"]:
                return [IceGridClient()]
            if processType in ["server", "serveramd"]:
                return [IceGridServer()]

    def getOptions(self, testcase, current):
        parent = re.match(r'^([\w]*).*', testcase.getTestSuite().getId()).group(1)
        if isinstance(testcase, ClientServerTestCase) and parent in ["Ice", "IceBox"]:
            return self.coreOptions
        elif parent in ["IceGrid", "Glacier2", "IceStorm", "IceDiscovery", "IceBridge"]:
            return self.serviceOptions

    def getRunOrder(self):
        return ["Slice", "IceUtil", "Ice", "IceSSL", "IceBox", "Glacier2", "IceGrid", "IceStorm"]

    def isCross(self, testId):
        return testId in [
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

    def getSoVersion(self):
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

component = Ice()

from Glacier2Util import *
from IceBoxUtil import *
from IceBridgeUtil import *
from IcePatch2Util import *
from IceGridUtil import *
from IceStormUtil import *

#
# Supported mappings
#
for m in filter(lambda x: os.path.isdir(os.path.join(toplevel, x)), os.listdir(toplevel)):
    if m == "cpp" or re.match("cpp-.*", m):
        Mapping.add(m, CppMapping())
    elif m == "java-compat" or re.match("java-compat-.*", m):
        Mapping.add(m, JavaCompatMapping())
    elif m == "java" or re.match("java-.*", m):
        Mapping.add(m, JavaMapping())
    elif m == "python" or re.match("python-.*", m):
        Mapping.add(m, PythonMapping())
    elif m == "ruby" or re.match("ruby-.*", m):
        Mapping.add(m, RubyMapping())
    elif m == "php" or re.match("php-.*", m):
        Mapping.add(m, PhpMapping())
    elif m == "js" or re.match("js-.*", m):
        Mapping.add(m, JavaScriptMapping())
    elif m == "objective-c" or re.match("objective-c-*", m):
        Mapping.add(m, ObjCMapping())
    elif m == "csharp" or re.match("charp-.*", m):
        Mapping.add("csharp", CSharpMapping())

if isinstance(platform, Windows):
    # Windows doesn't support all the mappings, we take them out here.
    Mapping.remove("ruby")
    if platform.getCompiler() != "VC140":
        Mapping.remove("python")
    if platform.getCompiler() not in ["VC140", "VC141"]:
        Mapping.remove("php")
elif not platform.hasDotNet():
    # Remove C# if Dot Net Core isn't supported
    Mapping.remove("csharp")

#
# Check if the Android SDK is installed and eventually add the Android mappings
#
try:
    run("adb version")
    Mapping.add(os.path.join("java-compat", "android"), AndroidCompatMapping())
    Mapping.add(os.path.join("java", "android"), AndroidMapping())
except:
    pass

#
# Check if Matlab is installed and eventually add the Matlab mapping
#
try:
    run("where matlab" if isinstance(platform, Windows) else "which matlab")
    Mapping.add("matlab", MatlabMapping())
except:
    pass
