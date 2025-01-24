# Copyright (c) ZeroC, Inc.

import os
import re

# Import as modules to allow for circular imports
import Util
import IceGridUtil


class Ice(Util.Component):
    # Options for all transports (ran only with Ice client/server tests defined for cross testing)
    transportOptions = {
        "protocol": ["tcp", "ssl", "wss", "ws"],
        "compress": [False, True],
        "ipv6": [False, True],
        "serialize": [False, True],
        "mx": [False, True],
    }

    # Options for Ice tests, run tests with ssl and ws/ipv6/serial/mx/compress
    coreOptions = {
        "protocol": ["ssl", "ws"],
        "compress": [False, True],
        "ipv6": [False, True],
        "serialize": [False, True],
        "mx": [False, True],
    }

    # Options for Ice services, run tests with ssl + mx
    serviceOptions = {
        "protocol": ["ssl"],
        "compress": [False],
        "ipv6": [False],
        "serialize": [False],
        "mx": [True],
    }

    def useBinDist(self, mapping, current):
        return Util.Component._useBinDist(self, mapping, current, "ICE_BIN_DIST")

    def getInstallDir(self, mapping, current):
        # On Windows, the Ice MSI installation can only be used for C++
        envHomeName = (
            None
            if isinstance(Util.platform, Util.Windows)
            and not isinstance(mapping, Util.CppMapping)
            else "ICE_HOME"
        )
        return Util.Component._getInstallDir(self, mapping, current, envHomeName)

    def getPhpExtension(self, mapping, current):
        if isinstance(Util.platform, Util.Windows):
            return (
                "php_ice.dll"
                if current.driver.configs[mapping].buildConfig in ["Debug", "Release"]
                else "php_ice_nts.dll"
            )
        else:
            return "ice.so"

    def getNugetPackageVersionFile(self, mapping):
        if isinstance(mapping, Util.CSharpMapping):
            return os.path.join(
                Util.toplevel, "csharp", "msbuild", "zeroc.ice.net.nuspec"
            )
        else:
            return os.path.join(
                Util.toplevel,
                "cpp",
                "msbuild",
                "zeroc.ice.{0}.nuspec".format(Util.platform.getPlatformToolset()),
            )

    def getFilters(self, mapping, config):
        if config.buildPlatform in ["iphoneos", "iphonesimulator"]:
            return (
                ["Ice/.*", "IceSSL/configuration"],
                [
                    "Ice/background",
                    "Ice/echo",
                    "Ice/faultTolerance",
                    "Ice/library",
                    "Ice/logger",
                    "Ice/properties",
                    "Ice/plugin",
                    "Ice/stringConverter",
                    "Ice/threadPoolPriority",
                    "Ice/services"
                ],
            )
        elif "static" in config.buildConfig:
            return (
                [
                    "Ice/.*",
                    "IceSSL/configuration",
                    "IceDiscovery/simple",
                    "IceGrid/simple",
                    "IceUtil/stacktrace",
                    "Glacier2/application",
                ],
                ["Ice/library", "Ice/plugin"],
            )
        elif isinstance(mapping, Util.JavaMapping) and config.android:
            return (
                ["Ice/.*"],
                [
                    "Ice/hash",
                    "Ice/faultTolerance",
                    "Ice/metrics",
                    "Ice/networkProxy",
                    "Ice/throughput",
                    "Ice/plugin",
                    "Ice/logger",
                    "Ice/properties",
                ],
            )
        elif isinstance(mapping, Util.JavaScriptMapping):
            return ([], ["typescript/.*"])
        elif isinstance(mapping, Util.SwiftMapping) and config.buildPlatform in [
            "iphonesimulator",
            "iphoneos",
        ]:
            return (
                ["Ice/.*", "IceSSL/configuration", "Slice/*"],
                ["Ice/properties", "Ice/udp"],
            )
        return ([], [])

    def canRun(self, testId, mapping, current):
        parent = re.match(r"^([\w]*).*", testId).group(1)
        if isinstance(Util.platform, Util.Linux):
            if (
                Util.platform.getLinuxId() in ["centos", "rhel", "fedora"]
                and current.config.buildPlatform == "x86"
            ):
                #
                # Don't test Glacier2/IceStorm/IceGrid services with multilib platforms. We only
                # build services for the native platform.
                #
                if parent in ["Glacier2", "IceStorm", "IceGrid"]:
                    return False
        elif isinstance(Util.platform, Util.Windows):
            #
            # On Windows, if testing with a binary distribution, don't test Glacier2/IceBridge services
            # with the Debug configurations since we don't provide binaries for them.
            #
            if self.useBinDist(mapping, current):
                if (
                    parent in ["Glacier2", "IceBridge"]
                    and current.config.buildConfig.find("Debug") >= 0
                ):
                    return False

        return True

    def isMainThreadOnly(self, testId):
        return False  # By default, tests support being run concurrently

    def getDefaultProcesses(self, mapping, processType, testId):
        if testId.startswith("IceUtil") or testId.startswith("Slice"):
            return [Util.SimpleClient()]
        elif testId.startswith("IceGrid"):
            if processType in ["client", "collocated"]:
                return [IceGridUtil.IceGridClient()]
            if processType in ["server", "serveramd"]:
                return [IceGridUtil.IceGridServer()]

    def getOptions(self, testcase, current):
        parent = re.match(r"^([\w]*).*", testcase.getTestSuite().getId()).group(1)
        if parent not in [
            "Ice",
            "IceBox",
            "IceGrid",
            "Glacier2",
            "IceStorm",
            "IceDiscovery",
            "IceBridge",
        ]:
            return None

        if isinstance(testcase, Util.CollocatedTestCase):
            return None

        # Define here Ice tests which are slow to execute and for which it's not useful to test different options
        if testcase.getTestSuite().getId() in [
            "Ice/binding",
            "Ice/faultTolerance",
            "Ice/location",
        ]:
            return self.serviceOptions

        # We only run the client/server tests defined for cross testing with all transports
        if isinstance(testcase, Util.ClientServerTestCase) and self.isCross(
            testcase.getTestSuite().getId()
        ):
            return self.transportOptions
        elif parent in ["Ice", "IceBox"]:
            return self.coreOptions
        else:
            return self.serviceOptions

    def getRunOrder(self):
        return [
            "Slice",
            "IceUtil",
            "Ice",
            "IceSSL",
            "IceBox",
            "Glacier2",
            "IceGrid",
            "IceStorm",
        ]

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
        with open(
            os.path.join(Util.toplevel, "cpp", "include", "Ice", "Config.h"), "r"
        ) as config:
            intVersion = int(
                re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1)
            )
            majorVersion = int(intVersion / 10000)
            minorVersion = int(intVersion / 100) - 100 * majorVersion
            patchVersion = intVersion % 100
            if patchVersion < 50:
                return "%d" % (majorVersion * 10 + minorVersion)
            elif patchVersion < 60:
                return "%da%d" % (majorVersion * 10 + minorVersion, patchVersion - 50)
            else:
                return "%db%d" % (majorVersion * 10 + minorVersion, patchVersion - 60)


component = Ice()

#
# Supported mappings
#
for m in filter(
    lambda x: os.path.isdir(os.path.join(Util.toplevel, x)), os.listdir(Util.toplevel)
):
    if m == "cpp" or re.match("cpp-.*", m):
        Util.Mapping.add(m, Util.CppMapping(), component)
    elif m == "java" or re.match("java-.*", m):
        Util.Mapping.add(m, Util.JavaMapping(), component)
    elif m == "python" or re.match("python-.*", m):
        Util.Mapping.add(m, Util.PythonMapping(), component)
    elif m == "ruby" or re.match("ruby-.*", m):
        Util.Mapping.add(
            m,
            Util.RubyMapping(),
            component,
            enable=not isinstance(Util.platform, Util.Windows),
        )
    elif m == "php" or re.match("php-.*", m):
        Util.Mapping.add(
            m,
            Util.PhpMapping(),
            component,
            enable=not isinstance(Util.platform, Util.Windows),
        )
    elif m == "js" or re.match("js-.*", m):
        Util.Mapping.add(
            m, Util.JavaScriptMapping(), component, enable=Util.platform.hasNodeJS()
        )
    elif m == "swift" or re.match("swift-.*", m):
        # Swift mapping requires Swift 5.0 or greater
        Util.Mapping.add(
            "swift",
            Util.SwiftMapping(),
            component,
            enable=Util.platform.hasSwift((5, 0)),
        )
    elif m == "csharp" or re.match("charp-.*", m):
        Util.Mapping.add(
            "csharp",
            Util.CSharpMapping(),
            component,
            enable=isinstance(Util.platform, Util.Windows) or Util.platform.hasDotNet(),
        )

#
# Check if Matlab is installed and eventually add the Matlab mapping
#
try:
    Util.run(
        "where matlab" if isinstance(Util.platform, Util.Windows) else "which matlab"
    )
    Util.Mapping.add("matlab", Util.MatlabMapping(), component)
except Exception:
    pass
