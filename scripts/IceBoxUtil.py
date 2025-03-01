# Copyright (c) ZeroC, Inc.

from Util import (
    CSharpMapping,
    Client,
    CppMapping,
    JavaMapping,
    Linux,
    Mapping,
    ProcessFromBinDir,
    ProcessIsReleaseOnly,
    Server,
    platform,
)
import os


class IceBox(ProcessFromBinDir, Server):
    def __init__(self, configFile=None, *args, **kargs):
        Server.__init__(self, *args, **kargs)
        self.configFile = configFile
        self.binDir = None

    def getExe(self, current):
        mapping = self.getMapping(current)
        if isinstance(mapping, JavaMapping):
            return "com.zeroc.IceBox.Server"
        elif isinstance(mapping, CSharpMapping):
            self.binDir = os.path.join("src", "iceboxnet", "bin", "Release", "net8.0")
            return "iceboxnet"
        else:
            name = "icebox"
            if (
                isinstance(platform, Linux)
                and platform.getLinuxId() in ["centos", "rhel", "fedora"]
                and current.config.buildPlatform == "x86"
            ):
                name += "32"  # Multilib platform
            return name

    def getEffectiveArgs(self, current, args):
        args = Server.getEffectiveArgs(self, current, args)
        if self.configFile:
            mapping = self.getMapping(current)
            if isinstance(mapping, CSharpMapping) and current.config.dotnet:
                args.append(
                    "--Ice.Config={0}.{1}".format(
                        self.configFile, mapping.getTargetFramework(current)
                    )
                )
            else:
                args.append("--Ice.Config={0}".format(self.configFile))
        return args


class IceBoxAdmin(ProcessFromBinDir, ProcessIsReleaseOnly, Client):
    def getMapping(self, current):
        # IceBox admin is only provided with the C++/Java, not C#
        mapping = Client.getMapping(self, current)
        if isinstance(mapping, CppMapping) or isinstance(mapping, JavaMapping):
            return mapping
        else:
            return Mapping.getByName("cpp")

    def getExe(self, current):
        mapping = self.getMapping(current)
        if isinstance(mapping, JavaMapping):
            return "com.zeroc.IceBox.Admin"
        else:
            return "iceboxadmin"
