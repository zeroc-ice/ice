#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, os
from Util import *
from Component import component

class IceBox(ProcessFromBinDir, Server):

    def __init__(self, configFile=None, *args, **kargs):
        Server.__init__(self, *args, **kargs)
        self.configFile = configFile

    def getExe(self, current):
        mapping = self.getMapping(current)
        if isinstance(mapping, JavaMapping):
            return "com.zeroc.IceBox.Server"
        elif isinstance(mapping, CSharpMapping):
            return "iceboxnet"
        else:
            name = "icebox"
            if isinstance(platform, Linux) and \
               platform.getLinuxId() in ["centos", "rhel", "fedora"] and \
               current.config.buildPlatform == "x86":
                name += "32" # Multilib platform
            if isinstance(platform, AIX) and \
               current.config.buildPlatform == "ppc":
                name += "_32"
            return name

    def getEffectiveArgs(self, current, args):
        args = Server.getEffectiveArgs(self, current, args)
        if self.configFile:
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
        elif isinstance(platform, AIX) and \
             current.config.buildPlatform == "ppc" and not component.useBinDist(mapping, current):
            return "iceboxadmin_32"
        else:
            return "iceboxadmin"
