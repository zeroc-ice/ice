# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

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
