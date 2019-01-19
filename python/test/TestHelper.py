#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
import sys
import os
import traceback
import importlib


class TestHelper:

    def __init__(self):
        self._communicator = None

    def getTestEndpoint(self, properties=None, num=0, protocol=""):

        if properties is None:
            properties = self._communicator.getProperties()

        if protocol == "":
            protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "default")

        port = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num

        return "{0} -p {1}".format(protocol, port)

    def getTestHost(self, properties=None):

        if properties is None:
            properties = self._communicator.getProperties()

        return properties.getPropertyWithDefaul("Ice.Default.Host", "127.0.0.1")

    def getTestProtocol(self, properties=None):

        if properties is None:
            properties = self._communicator.getProperties()

        return properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp")

    def getTestPort(self, properties=None, num=0):

        if properties is None:
            properties = self._communicator.getProperties()

        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num

    def createTestProperties(self, args=[]):

        properties = Ice.createProperties(args)
        args = properties.parseCommandLineOptions("Test", args)
        return properties

    def initialize(self, initData=None, properties=None, args=[]):

        if initData is None:
            initData = Ice.InitializationData()
            if properties:
                initData.properties = properties
            else:
                initData.properties = self.createTestProperties(args)

        communicator = Ice.initialize(initData)

        if self._communicator is None:
            self._communicator = communicator

        return communicator

    def communicator(self):
        return self._communicator

    def shutdown(self):
        if self._communicator:
            self._communicator.shutdown()

    @classmethod
    def loadSlice(self, args):
        sliceDir = Ice.getSliceDir()
        if not sliceDir:
            print(sys.argv[0] + ': Slice directory not found.')
            sys.exit(1)
        Ice.loadSlice("'-I{0}' {1}".format(sliceDir, args))

    @classmethod
    def run(self):
        try:
            moduleName = os.path.splitext(sys.argv[1])[0]
            module = importlib.import_module(moduleName)
            cls = getattr(module, moduleName)
            helper = cls()
            helper.run(sys.argv[2:])
            return 0
        except Exception:
            traceback.print_exc()
            return 1


if __name__ == '__main__':
    sys.exit(TestHelper.run())
