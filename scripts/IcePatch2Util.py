#
# Copyright (c) ZeroC, Inc. All rights reserved.
#


from Util import Mapping, Process, ProcessFromBinDir, ProcessIsReleaseOnly


class IcePatch2Calc(ProcessFromBinDir, ProcessIsReleaseOnly, Process):
    def __init__(self, *args, **kargs):
        Process.__init__(
            self,
            exe="icepatch2calc++11",
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe


class IcePatch2Client(ProcessFromBinDir, ProcessIsReleaseOnly, Process):
    def __init__(self, *args, **kargs):
        Process.__init__(
            self,
            exe="icepatch2client++11",
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe


class IcePatch2Server(ProcessFromBinDir, ProcessIsReleaseOnly, Process):
    def __init__(self, *args, **kargs):
        Process.__init__(
            self,
            exe="icepatch2server++11",
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe
