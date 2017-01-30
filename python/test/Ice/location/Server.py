#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Test.ice")
import Test

class ServerLocatorRegistry(Test._TestLocatorRegistryDisp):
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

class ServerLocator(Test._TestLocatorDisp):

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

class ServerManagerI(Test._ServerManagerDisp):
    def __init__(self, registry, initData):
        self._registry = registry
        self._communicators = []
        self._initData = initData
        self._initData.properties.setProperty("TestAdapter.Endpoints", "default")
        self._initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter")
        self._initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter")
        self._initData.properties.setProperty("TestAdapter2.Endpoints", "default")
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
        serverCommunicator = Ice.initialize(data=initData)
        self._communicators.append(serverCommunicator)
        adapter = serverCommunicator.createObjectAdapter("TestAdapter")

        adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2")

        locator = serverCommunicator.stringToProxy("locator:default -p 12010")
        adapter.setLocator(Ice.LocatorPrx.uncheckedCast(locator))
        adapter2.setLocator(Ice.LocatorPrx.uncheckedCast(locator))

        object = TestI(adapter, adapter2, self._registry)
        self._registry.addObject(adapter.add(object, Ice.stringToIdentity("test")))
        self._registry.addObject(adapter.add(object, Ice.stringToIdentity("test2")))
        adapter.add(object, Ice.stringToIdentity("test3"))

        adapter.activate()
        adapter2.activate()

    def shutdown(self, current=None):
        for i in self._communicators:
            i.destroy()
        current.adapter.getCommunicator().shutdown()

class HelloI(Test._HelloDisp):
    def sayHello(self, current=None):
        pass

class TestI(Test._TestIntfDisp):
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

def run(args, communicator, initData):
    #
    # Register the server manager. The server manager creates a new
    # 'server' (a server isn't a different process, it's just a new
    # communicator and object adapter).
    #
    properties = communicator.getProperties()
    properties.setProperty("Ice.ThreadPool.Server.Size", "2")
    properties.setProperty("ServerManager.Endpoints", "default -p 12010:udp")

    adapter = communicator.createObjectAdapter("ServerManager")

    #
    # We also register a sample server locator which implements the
    # locator interface, this locator is used by the clients and the
    # 'servers' created with the server manager interface.
    #
    registry = ServerLocatorRegistry()
    registry.addObject(adapter.createProxy(Ice.stringToIdentity("ServerManager")))
    object = ServerManagerI(registry, initData)
    adapter.add(object, Ice.stringToIdentity("ServerManager"))

    registryPrx = Ice.LocatorRegistryPrx.uncheckedCast(adapter.add(registry, Ice.stringToIdentity("registry")))

    locator = ServerLocator(registry, registryPrx)
    adapter.add(locator, Ice.stringToIdentity("locator"))

    adapter.activate()
    communicator.waitForShutdown()

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator, initData)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
