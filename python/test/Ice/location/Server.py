#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from generated.test.Ice.location import Test

import Ice


class ServerLocatorRegistry(Test.TestLocatorRegistry):
    def __init__(self):
        self._adapters = {}
        self._objects = {}

    @override
    def setAdapterDirectProxy(self, id: str, proxy: Ice.ObjectPrx | None, current: Ice.Current):
        if proxy:
            self._adapters[id] = proxy
        else:
            self._adapters.pop(id)
        return None

    @override
    def setReplicatedAdapterDirectProxy(
        self, adapterId: str, replicaGroupId: str, proxy: Ice.ObjectPrx | None, current: Ice.Current
    ):
        if proxy:
            self._adapters[adapterId] = proxy
            self._adapters[replicaGroupId] = proxy
        else:
            self._adapters.pop(adapterId)
            self._adapters.pop(replicaGroupId)
        return None

    @override
    def setServerProcessProxy(self, id: str, proxy: Ice.ObjectPrx | None, current: Ice.Current):
        return None

    @override
    def addObject(self, obj: Ice.ObjectPrx | None, current: Ice.Current):
        assert obj is not None
        self._addObject(obj)

    def _addObject(self, obj: Ice.ObjectPrx):
        self._objects[obj.ice_getIdentity()] = obj

    def getAdapter(self, adapter: str) -> Ice.ObjectAdapter:
        if adapter not in self._adapters:
            raise Ice.AdapterNotFoundException()
        return self._adapters[adapter]

    def getObject(self, id: Ice.Identity) -> Ice.ObjectPrx:
        if id not in self._objects:
            raise Ice.ObjectNotFoundException()
        return self._objects[id]


class ServerLocator(Test.TestLocator):
    def __init__(self, registry: ServerLocatorRegistry, registryPrx: Ice.LocatorRegistryPrx):
        self._registry = registry
        self._registryPrx = registryPrx
        self._requestCount = 0

    @override
    def findObjectById(self, id: Ice.Identity, current: Ice.Current):
        self._requestCount += 1
        return Ice.Future.completed(self._registry.getObject(id))

    def findAdapterById(self, id: str, current: Ice.Current):
        self._requestCount += 1
        return Ice.Future.completed(self._registry.getAdapter(id))

    def getRegistry(self, current: Ice.Current):
        return self._registryPrx

    def getRequestCount(self, current: Ice.Current):
        return self._requestCount


class ServerManagerI(Test.ServerManager):
    def __init__(self, registry: ServerLocatorRegistry, initData: Ice.InitializationData, helper: TestHelper):
        self._registry = registry
        self._communicators = []
        self._initData = initData
        self._nextPort = 1
        self._helper = helper
        assert self._initData.properties is not None
        self._initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter")
        self._initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter")
        self._initData.properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2")

    @override
    def startServer(self, current: Ice.Current):
        #
        # Simulate a server: create a new communicator and object
        # adapter. The object adapter is started on a system allocated
        # port. The configuration used here contains the Ice.Locator
        # configuration variable. The new object adapter will register
        # its endpoints with the locator and create references containing
        # the adapter id instead of the endpoints.
        #
        serverCommunicator = Ice.initialize(initData=self._initData)
        self._communicators.append(serverCommunicator)

        nRetry = 10
        while nRetry > 0:
            nRetry -= 1
            adapter = None
            adapter2 = None
            try:
                serverCommunicator.getProperties().setProperty(
                    "TestAdapter.Endpoints",
                    self._helper.getTestEndpoint(num=self._nextPort),
                )
                self._nextPort += 1
                serverCommunicator.getProperties().setProperty(
                    "TestAdapter2.Endpoints",
                    self._helper.getTestEndpoint(num=self._nextPort),
                )
                self._nextPort += 1

                adapter = serverCommunicator.createObjectAdapter("TestAdapter")
                adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2")

                locator = serverCommunicator.stringToProxy("locator:{0}".format(self._helper.getTestEndpoint()))
                assert locator is not None
                adapter.setLocator(Ice.LocatorPrx.uncheckedCast(locator))
                adapter2.setLocator(Ice.LocatorPrx.uncheckedCast(locator))

                object = TestI(adapter, adapter2, self._registry)
                self._registry._addObject(adapter.add(object, Ice.stringToIdentity("test")))
                self._registry._addObject(adapter.add(object, Ice.stringToIdentity("test2")))
                adapter.add(object, Ice.stringToIdentity("test3"))

                adapter.activate()
                adapter2.activate()
                break
            except Ice.SocketException as ex:
                if nRetry == 0:
                    raise ex

                # Retry, if OA creation fails with EADDRINUSE (this can occur when running with JS web
                # browser clients if the driver uses ports in the same range as this test, ICE-8148)
                if adapter:
                    adapter.destroy()
                if adapter2:
                    adapter2.destroy()

    @override
    def shutdown(self, current: Ice.Current):
        for i in self._communicators:
            i.destroy()
        current.adapter.getCommunicator().shutdown()


class HelloI(Test.Hello):
    @override
    def sayHello(self, current: Ice.Current):
        pass


class TestI(Test.TestIntf):
    def __init__(self, adapter: Ice.ObjectAdapter, adapter2: Ice.ObjectAdapter, registry: ServerLocatorRegistry):
        self._adapter1 = adapter
        self._adapter2 = adapter2
        self._registry = registry
        self._registry._addObject(self._adapter1.add(HelloI(), Ice.stringToIdentity("hello")))

    @override
    def shutdown(self, current: Ice.Current):
        self._adapter1.getCommunicator().shutdown()

    @override
    def getHello(self, current: Ice.Current):
        return Test.HelloPrx.uncheckedCast(self._adapter1.createIndirectProxy(Ice.stringToIdentity("hello")))

    @override
    def getReplicatedHello(self, current: Ice.Current):
        return Test.HelloPrx.uncheckedCast(self._adapter1.createProxy(Ice.stringToIdentity("hello")))

    @override
    def migrateHello(self, current: Ice.Current):
        id = Ice.stringToIdentity("hello")
        try:
            self._registry._addObject(self._adapter2.add(self._adapter1.remove(id), id))
        except Ice.NotRegisteredException:
            self._registry._addObject(self._adapter1.add(self._adapter2.remove(id), id))


class Server(TestHelper):
    def run(self, args: list[str]):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)

        with self.initialize(initData=initData) as communicator:
            #
            # Register the server manager. The server manager creates a new
            # 'server' (a server isn't a different process, it's just a new
            # communicator and object adapter).
            #
            communicator.getProperties().setProperty("Ice.ThreadPool.Server.Size", "2")
            communicator.getProperties().setProperty("ServerManager.Endpoints", self.getTestEndpoint())

            adapter = communicator.createObjectAdapter("ServerManager")

            #
            # We also register a sample server locator which implements the
            # locator interface, this locator is used by the clients and the
            # 'servers' created with the server manager interface.
            #
            registry = ServerLocatorRegistry()
            registry._addObject(adapter.createProxy(Ice.stringToIdentity("ServerManager")))
            adapter.add(
                ServerManagerI(registry, initData, self),
                Ice.stringToIdentity("ServerManager"),
            )

            registryPrx = Ice.LocatorRegistryPrx.uncheckedCast(adapter.add(registry, Ice.stringToIdentity("registry")))

            locator = ServerLocator(registry, registryPrx)
            adapter.add(locator, Ice.stringToIdentity("locator"))

            adapter.activate()
            communicator.waitForShutdown()
