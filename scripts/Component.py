#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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

    def getNugetPackageVersionFile(self, mapping):
        if isinstance(mapping, CSharpMapping):
            return os.path.join(toplevel, "csharp", "msbuild", "zeroc.ice.net.nuspec")
        else:
            return os.path.join(toplevel, "cpp", "msbuild", "zeroc.ice.{0}.nuspec".format(platform.getPlatformToolset()))

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
        elif isinstance(mapping, CppMapping) and config.uwp:
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
        elif isinstance(platform, Windows) and platform.getCompiler() in ["v100"]:
            return (["Ice/.*", "IceSSL/.*", "IceBox/.*", "IceDiscovery/.*", "IceUtil/.*", "Slice/.*"], [])
        elif isinstance(mapping, CSharpMapping) and config.xamarin:
            return (["Ice/.*"],
                    ["Ice/hash",
                     "Ice/faultTolerance",
                     "Ice/metrics",
                     "Ice/assemblies",
                     "Ice/background",
                     "Ice/dispatcher",
                     "Ice/networkProxy",
                     "Ice/throughput",
                     "Ice/plugin",
                     "Ice/logger",
                     "Ice/properties",
                     "Ice/slicing/*"])
        elif isinstance(mapping, JavaMapping) and config.android:
            return (["Ice/.*"],
                    ["Ice/hash",
                     "Ice/faultTolerance",
                     "Ice/metrics",
                     "Ice/networkProxy",
                     "Ice/throughput",
                     "Ice/plugin",
                     "Ice/logger",
                     "Ice/properties"])
        elif isinstance(mapping, JavaScriptMapping):
            return ([], ["typescript/.*", "es5/*"])
        elif isinstance(mapping, SwiftMapping) and config.buildPlatform in ["iphonesimulator", "iphoneos"]:
            return (["Ice/.*", "IceSSL/configuration"], ["Ice/properties", "Ice/udp"])
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

        if current.config.xamarin and not current.config.uwp:
            #
            # With Xamarin on Android and iOS Ice/udp is only supported with IPv4
            #
            if current.config.ipv6 and testId in ["Ice/udp"]:
                return False


        # IceSSL test doesn't work on macOS/.NET Core
        if isinstance(mapping, CSharpMapping) and isinstance(platform, Darwin) and parent in ["IceSSL"]:
            return False

        return True

    def isMainThreadOnly(self, testId):
        #return testId.startswith("IceStorm") # TODO: WORKAROUND for ICE-8175
        return False # By default, tests support being run concurrently

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
            "Ice/optional",
        ]

    def getSoVersion(self):
        with open(os.path.join(toplevel, "cpp", "include", "IceUtil", "Config.h"), "r") as config:
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
        Mapping.add(m, CppMapping(), component)
    elif m == "java-compat" or re.match("java-compat-.*", m):
        Mapping.add(m, JavaCompatMapping(), component)
    elif m == "java" or re.match("java-.*", m):
        Mapping.add(m, JavaMapping(), component)
    elif m == "python" or re.match("python-.*", m):
        Mapping.add(m, PythonMapping(), component)
    elif m == "ruby" or re.match("ruby-.*", m):
        Mapping.add(m, RubyMapping(), component)
    elif m == "php" or re.match("php-.*", m):
        Mapping.add(m, PhpMapping(), component)
    elif m == "js" or re.match("js-.*", m):
        Mapping.add(m, JavaScriptMapping(), component)
        Mapping.add("typescript", TypeScriptMapping(), component, "js")
    elif m == "objective-c" or re.match("objective-c-*", m):
        Mapping.add(m, ObjCMapping(), component)
    elif m == "csharp" or re.match("csharp-.*", m):
        Mapping.add("csharp", CSharpMapping(), component)
    elif m == "swift" or re.match("swift-.*", m):
        Mapping.add("swift", SwiftMapping(), component)

if isinstance(platform, Windows):
    # Windows doesn't support all the mappings, we take them out here.
    Mapping.remove("ruby")
    if platform.getCompiler() != "v140":
        Mapping.remove("python")
    if platform.getCompiler() not in ["v140", "v141"]:
        Mapping.remove("php")
elif not platform.hasDotNet():
    # Remove C# if Dot Net Core isn't supported
    Mapping.remove("csharp")

#
# Check if Matlab is installed and eventually add the Matlab mapping
#
try:
    run("where matlab" if isinstance(platform, Windows) else "which matlab")
    Mapping.add("matlab", MatlabMapping(), component)
except:
    pass
