# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os
from Util import *

class IceBox(ProcessFromBinDir, Server):

    processType = "icebox"

class IceBoxAdmin(ProcessFromBinDir, Client):

    processType = "iceboxadmin"

    def getMapping(self, current):
        # IceBox admin is only provided with the C++/Java, not C#
        mapping = self.mapping or current.testcase.getMapping()
        if isinstance(mapping, CppMapping) or isinstance(mapping, JavaMapping):
            return mapping
        else:
            return Mapping.getByName("cpp")
