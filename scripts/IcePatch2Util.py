# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from Util import *

class IcePatch2Calc(ProcessFromBinDir, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2calc", mapping=Mapping.getByName("cpp"), *args, **kargs)

class IcePatch2Client(ProcessFromBinDir, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2client", mapping=Mapping.getByName("cpp"), *args, **kargs)

class IcePatch2Server(ProcessFromBinDir, Process):

    def __init__(self, *args, **kargs):
        Process.__init__(self, exe="icepatch2server", mapping=Mapping.getByName("cpp"), *args, **kargs)
