#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os

from Util import *

class Ice(Component):

    # Options for all transports (ran only with Ice client/server tests defined for cross testing)
    transportOptions = {
        "transport" : ["tcp", "ssl", "wss", "ws"],
        "protocol" : ["ice1", "ice2"],
        "compress" : [False, True],
        "ipv6" : [False, True],
        "serialize" : [False, True],
        "mx" : [False, True],
    }

    # Options for Ice tests, run tests with ssl and ws/ipv6/serial/mx/compress
    coreOptions = {
        "transport" : ["ssl", "ws"],
        "protocol" : ["ice1", "ice2"],
        "compress" : [False, True],
        "ipv6" : [False, True],
        "serialize" : [False, True],
        "mx" : [False, True],
    }

    # Options for Ice services, run tests with ssl + mx
    serviceOptions = {
        "transport" : ["ssl"],
        "protocol" : ["ice1"],
        "compress" : [False],
        "ipv6" : [False],
        "serialize" : [False],
        "mx" : [True],
    }

    def useBinDist(self, mapping, current):
        return Component._useBinDist(self, mapping, current, "ICE_BIN_DIST")

    def getInstallDir(self, mapping, current):
        # On Windows, the Ice MSI installation can only be used for C++
        envHomeName = None if isinstance(platform, Windows) and not isinstance(mapping, CppMapping) else "ICE_HOME"
        return Component._getInstallDir(self, mapping, current, envHomeName)

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
                     "Ice/library",
                     "Ice/logger",
                     "Ice/properties",
                     "Ice/plugin",
                     "Ice/stringConverter",
                     "Ice/threadPoolPriority",
                     "Ice/udp"])
        elif "static" in config.buildConfig:
            return (["Ice/.*", "IceSSL/configuration", "IceDiscovery/simple", "IceGrid/simple"],
                    ["Ice/library", "Ice/plugin"])
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
            return (["Ice/.*", "IceSSL/configuration", "Slice/*"], ["Ice/properties", "Ice/udp"])
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
        elif isinstance(platform, Windows):
            #
            # On Windows, if testing with a binary distribution, don't test Glacier2/IceBridge services
            # with the Debug configurations since we don't provide binaries for them.
            #
            if self.useBinDist(mapping, current):
                if parent in ["Glacier2", "IceBridge"] and current.config.buildConfig.find("Debug") >= 0:
                    return False
        elif isinstance(platform, AIX):
            if current.config.buildPlatform == "ppc" and self.useBinDist(mapping, current):
                #
                # Don't test Glacier2, IceBridge and IceGrid services on ppc with bindist. We only ship
                # ppc64 binaries for these services
                #
                if parent in ["Glacier2", "IceBridge", "IceGrid"]:
                    return False
                if testId == "IceStorm/repgrid":
                    return False

        if current.config.xamarin and not current.config.uwp:
            #
            # With Xamarin on Android and iOS Ice/udp is only supported with IPv4
            #
            if current.config.ipv6 and testId in ["Ice/udp"]:
                return False

        # Only IceGrid/Glacier2 tests don't support running with the ice2 protocol for now
        # TODO: remove once ice2 is supported with all the mappings
        if parent in ["IceGrid", "Glacier2"]:
            return current.config.protocol == "ice1"

        return True

    def isMainThreadOnly(self, testId):
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
        if parent not in ["Ice", "IceBox", "IceGrid", "Glacier2", "IceStorm", "IceDiscovery", "IceBridge"]:
            return None

        # No specific options for collocated tests
        if isinstance(testcase, CollocatedTestCase):
            return None

        # Define here Ice tests which are slow to execute and for which it's not useful to test different options
        if testcase.getTestSuite().getId() in ["Ice/binding", "Ice/faultTolerance"]:
            return self.serviceOptions

        # We only run the client/server tests defined for cross testing with all transports (we skip them for
        # AMD/Tie client/server tests however).
        if type(testcase) is ClientServerTestCase and self.isCross(testcase.getTestSuite().getId()):
            return self.transportOptions
        elif parent in ["Ice", "IceBox", "IceDiscovery"]:
            return self.coreOptions
        else:
            return self.serviceOptions

    def getRunOrder(self):
        return ["Slice", "IceUtil", "Ice", "IceSSL", "IceBox", "Glacier2", "IceGrid", "IceStorm"]

    def isCross(self, testId):
        return testId in [
            "Ice/ami",
            "Ice/exceptions",
            "Ice/enums",
            "Ice/facets",
            "Ice/inheritance",
            "Ice/invoke",
            "Ice/objects",
            "Ice/operations",
            "Ice/proxy",
            "Ice/slicing/exceptions",
            "Ice/slicing/objects",
            "Ice/optional",
        ]

    def getSoVersion(self):
        with open(os.path.join(toplevel, "cpp", "include", "IceUtil", "Config.h"), "r") as config:
            return re.search("ICE_SO_VERSION \"([0-9ab]+)\"", config.read()).group(1)

component = Ice()

from Glacier2Util import *
from IceBoxUtil import *
from IceBridgeUtil import *
from IceGridUtil import *
from IceStormUtil import *

#
# Supported mappings
#

# The order that the languages will be tested in.
lang_order = ("cpp", "java", "python", "js", "csharp", "swift")

# Filters out any files that aren't directories or don't start with one of the prefixes specified in 'lang_order'.
mapping_filter = lambda x: os.path.isdir(os.path.join(toplevel, x)) and x.startswith(lang_order)
# Returns the build-ordered index for a mapping named 'x'.
mapping_orderer = lambda x: [index for index, name in enumerate(lang_order) if x.startswith(name)][0]

for m in sorted(filter(mapping_filter, os.listdir(toplevel)), key=mapping_orderer):
    if m == "cpp" or re.match("cpp-.*", m):
        Mapping.add(m, CppMapping(), component)
    elif m == "java" or re.match("java-.*", m):
        Mapping.add(m, JavaMapping(), component)
#    TODO temporarily disabled testing of Python mapping.
#    elif m == "python" or re.match("python-.*", m):
#        Mapping.add(m, PythonMapping(), component)
    elif m == "js" or re.match("js-.*", m):
        Mapping.add(m, JavaScriptMapping(), component, enable=platform.hasNodeJS())
        Mapping.add("typescript", TypeScriptMapping(), component, "js", enable=platform.hasNodeJS())
    elif m == "csharp" or re.match("charp-.*", m):
        Mapping.add("csharp", CSharpMapping(), component, enable=isinstance(platform, Windows) or platform.hasDotNet())
    elif m == "swift" or re.match("swift-.*", m):
        # Swift mapping requires Swift 5.0 or greater
        Mapping.add("swift", SwiftMapping(), component, enable=platform.hasSwift((5, 0)))

if isinstance(platform, Windows):
    # Windows doesn't support all the mappings, we take them out here.
#    TODO temporarily disabled testing of Python mapping.
#    if platform.getCompiler() not in ["v141"]:
#        Mapping.disable("python")
    if platform.getCompiler() not in ["v142"]:
        Mapping.disable("csharp")
#
# Check if Matlab is installed and eventually add the Matlab mapping
#
try:
    run("where matlab" if isinstance(platform, Windows) else "which matlab")
    Mapping.add("matlab", MatlabMapping(), component)
except:
    pass
