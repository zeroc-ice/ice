#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import *

class IcePatch2Calc(ProcessFromBinDir, ProcessIsReleaseOnly, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2calc", mapping=Mapping.getByName("cpp"), *args, **kargs)

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

class IcePatch2Client(ProcessFromBinDir, ProcessIsReleaseOnly, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2client", mapping=Mapping.getByName("cpp"), *args, **kargs)

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

class IcePatch2Server(ProcessFromBinDir, ProcessIsReleaseOnly, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2server", mapping=Mapping.getByName("cpp"), *args, **kargs)

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe
