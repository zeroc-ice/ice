# Copyright (c) ZeroC, Inc.

import Ice
import sys
import os
import traceback
import importlib


class TestHelper:
    def __init__(self):
        self._communicator: Ice.Communicator | None = None

    def getTestEndpoint(
        self, properties: Ice.Properties | None = None, num=0, protocol=""
    ) -> str:
        if properties is None:
            # Note that self._communicator is sometimes None when this method is called with properties != None.
            assert self._communicator is not None, "Communicator must be initialized"
            properties = self._communicator.getProperties()

        if protocol == "":
            protocol = properties.getPropertyWithDefault(
                "Ice.Default.Protocol", "default"
            )

        port = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num

        return "{0} -p {1}".format(protocol, port)

    def getTestHost(self, properties: Ice.Properties | None = None) -> str:
        assert self._communicator is not None, "Communicator must be initialized"

        if properties is None:
            properties = self._communicator.getProperties()

        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1")

    def getTestProtocol(self, properties: Ice.Properties | None = None) -> str:
        assert self._communicator is not None, "Communicator must be initialized"
        if properties is None:
            properties = self._communicator.getProperties()

        return properties.getIceProperty("Ice.Default.Protocol")

    def getTestPort(self, properties: Ice.Properties | None = None, num=0) -> int:
        assert self._communicator is not None, "Communicator must be initialized"
        if properties is None:
            properties = self._communicator.getProperties()

        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num

    def createTestProperties(self, args: list[str] | None = None) -> Ice.Properties:
        if args is None:
            args = []
        properties = Ice.createProperties(args)
        args = properties.parseCommandLineOptions("Test", args)
        return properties

    def initialize(
        self,
        initData: Ice.InitializationData | None = None,
        properties: Ice.Properties | None = None,
        args: list[str] | None = None,
    ) -> Ice.Communicator:
        if args is None:
            args = []
        if initData is None:
            initData = Ice.InitializationData()
            if properties:
                initData.properties = properties
            else:
                initData.properties = self.createTestProperties(args)

        communicator = Ice.initialize(initData=initData)

        if self._communicator is None:
            self._communicator = communicator

        return communicator

    def communicator(self) -> Ice.Communicator:
        assert self._communicator is not None, "Communicator must be initialized"
        return self._communicator

    def shutdown(self):
        if self._communicator:
            self._communicator.shutdown()

    @classmethod
    def loadSlice(cls, args):
        sliceDir = Ice.getSliceDir()
        if not sliceDir:
            print(sys.argv[0] + ": Slice directory not found.")
            sys.exit(1)
        Ice.loadSlice("'-I{0}' {1}".format(sliceDir, args))

    @classmethod
    def run(cls) -> int:
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


if __name__ == "__main__":
    sys.exit(TestHelper.run())
