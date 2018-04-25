# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os
from Util import *

class IceBox(ProcessFromBinDir, Server):

    processType = "icebox"

    def __init__(self, *args, **kargs):
        Server.__init__(self, *args, **kargs)

        # Find config file
        if 'args' in kargs:
            for v in kargs['args']:
                if "--Ice.Config=" in v:
                    self.config = v.replace("--Ice.Config=", "")
                    break

    def setup(self, current):
        mapping = self.mapping or current.testcase.getMapping()
        #
        # If running IceBox tests with .NET Core we need to generate a config
        # file that use the service for the .NET Framework used to build the
        # tests
        #
        if isinstance(mapping, CSharpMapping) and current.config.netframework:

            targetConfig = os.path.abspath(
                self.config.strip('"').format(testdir=current.testsuite.getPath(),
                                   iceboxconfigext=".{0}".format(current.config.netframework)))
            baseConfig = targetConfig.rstrip(".{0}".format(current.config.netframework))
            with open(baseConfig, 'r') as config:
                with open(targetConfig, 'w') as target:
                    for line in config.readlines():
                        target.write(line.replace("\\net45\\", "\\netstandard2.0\\{0}\\".format(current.config.netframework)))
                    current.files.append(targetConfig)

class IceBoxAdmin(ProcessFromBinDir, Client):

    processType = "iceboxadmin"

    def getMapping(self, current):
        # IceBox admin is only provided with the C++/Java, not C#
        mapping = self.mapping or current.testcase.getMapping()
        if isinstance(mapping, CppMapping) or isinstance(mapping, JavaMapping):
            return mapping
        else:
            return Mapping.getByName("cpp")
