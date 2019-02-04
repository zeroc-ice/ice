#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import *

class IcePatch2Calc(ProcessFromBinDir, ProcessIsReleaseOnly, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2calc", mapping=Mapping.getByName("cpp"), *args, **kargs)

class IcePatch2Client(ProcessFromBinDir, ProcessIsReleaseOnly, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2client", mapping=Mapping.getByName("cpp"), *args, **kargs)

class IcePatch2Server(ProcessFromBinDir, ProcessIsReleaseOnly, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2server", mapping=Mapping.getByName("cpp"), *args, **kargs)
