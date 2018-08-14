#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Test


class ServerLocatorRegistry(Test.TestLocatorRegistry):
    def __init__(self):
        self._adapters = {}
        self._objects = {}

    def setAdapterDirectProxy(self, adapter, obj, current=None):
        if obj:
            self._adapters[adapter] = obj
        else:
            self._adapters.pop(adapter)
        return None

    def setReplicatedAdapterDirectProxy(self, adapter, replica, obj, current=None):
        if obj:
            self._adapters[adapter] = obj
            self._adapters[replica] = obj
        else:
            self._adapters.pop(adapter)
            self._adapters.pop(replica)
        return None

    def setServerProcessProxy(self, id, proxy, current=None):
        return None

    def addObject(self, obj, current=None):
        self._objects[obj.ice_getIdentity()] = obj

    def getAdapter(self, adapter):
        if adapter not in self._adapters:
            raise Ice.AdapterNotFoundException()
        return self._adapters[adapter]

    def getObject(self, id):
        if id not in self._objects:
            raise Ice.ObjectNotFoundException()
        return self._objects[id]


class ServerLocator(Test.TestLocator):

    def __init__(self, registry, registryPrx):
        self._registry = registry
        self._registryPrx = registryPrx
        self._requestCount = 0

    def findObjectById(self, id, current=None):
        self._requestCount += 1
        return Ice.Future.completed(self._registry.getObject(id))

    def findAdapterById(self, id, current=None):
        self._requestCount += 1
        return Ice.Future.completed(self._registry.getAdapter(id))

    def getRegistry(self, current=None):
        return self._registryPrx

    def getRequestCount(self, current=None):
        return self._requestCount


class ServerManagerI(Test.ServerManager):
    def __init__(self, registry, initData, helper):
        self._registry = registry
        self._communicators = []
        self._initData = initData
        self._nextPort = 1
        self._helper = helper;
        self._initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter")
        self._initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter")
        self._initData.properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2")

    def startServer(self, current=None):

        #
        # Simulate a server: create a new communicator and object
        # adapter. The object adapter is started on a system allocated
        # port. The configuration used here contains the Ice.Locator
        # configuration variable. The new object adapter will register
        # its endpoints with the locator and create references containing
        # the adapter id instead of the endpoints.
        #
        serverCommunicator = Ice.initialize(self._initData)
        self._communicators.append(serverCommunicator)

        nRetry = 10
        while --nRetry > 0:
            adapter = None
            adapter2 = None
            try:
                serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                               self._helper.getTestEndpoint(num=self._nextPort))
                self._nextPort += 1
                serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints",
                                                               self._helper.getTestEndpoint(num=self._nextPort))
                self._nextPort += 1

                adapter = serverCommunicator.createObjectAdapter("TestAdapter")
                adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2")

                locator = serverCommunicator.stringToProxy("locator:{0}".format(self._helper.getTestEndpoint()))
                adapter.setLocator(Ice.LocatorPrx.uncheckedCast(locator))
                adapter2.setLocator(Ice.LocatorPrx.uncheckedCast(locator))

                object = TestI(adapter, adapter2, self._registry)
                self._registry.addObject(adapter.add(object, Ice.stringToIdentity("test")))
                self._registry.addObject(adapter.add(object, Ice.stringToIdentity("test2")))
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

    def shutdown(self, current=None):
        for i in self._communicators:
            i.destroy()
        current.adapter.getCommunicator().shutdown()


class HelloI(Test.Hello):
    def sayHello(self, current=None):
        pass


class TestI(Test.TestIntf):
    def __init__(self, adapter, adapter2, registry):
        self._adapter1 = adapter
        self._adapter2 = adapter2
        self._registry = registry
        self._registry.addObject(self._adapter1.add(HelloI(), Ice.stringToIdentity("hello")))

    def shutdown(self, current=None):
        self._adapter1.getCommunicator().shutdown()

    def getHello(self, current=None):
        return Test.HelloPrx.uncheckedCast(self._adapter1.createIndirectProxy(Ice.stringToIdentity("hello")))

    def getReplicatedHello(self, current=None):
        return Test.HelloPrx.uncheckedCast(self._adapter1.createProxy(Ice.stringToIdentity("hello")))

    def migrateHello(self, current=None):
        id = Ice.stringToIdentity("hello")
        try:
            self._registry.addObject(self._adapter2.add(self._adapter1.remove(id), id))
        except Ice.NotRegisteredException:
            self._registry.addObject(self._adapter1.add(self._adapter2.remove(id), id))


class Server(TestHelper):

    def run(self, args):

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
            registry.addObject(adapter.createProxy(Ice.stringToIdentity("ServerManager")))
            adapter.add(ServerManagerI(registry, initData, self), Ice.stringToIdentity("ServerManager"))

            registryPrx = Ice.LocatorRegistryPrx.uncheckedCast(adapter.add(registry, Ice.stringToIdentity("registry")))

            locator = ServerLocator(registry, registryPrx)
            adapter.add(locator, Ice.stringToIdentity("locator"))

            adapter.activate()
            communicator.waitForShutdown()
