#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.insert(0, os.path.join(toplevel, "python"))
sys.path.insert(0, os.path.join(toplevel, "lib"))

import Ice

#
# Find Slice directory.
#
slice_dir = os.getenv('ICEPY_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, "slice")):
    slice_dir = os.getenv('ICE_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, "slice")):
    print sys.argv[0] + ': Slice directory not found. Define ICEPY_HOME or ICE_HOME.'
    sys.exit(1)

Ice.loadSlice('-I' + slice_dir + '/slice Test.ice')
import Test

class ServerLocatorRegistry(Ice.LocatorRegistry):
    def __init__(self):
        self._adapters = {}
        self._objects = {}

    def setAdapterDirectProxy_async(self, cb, adapter, obj, current=None):
        self._adapters[adapter] = obj
        cb.ice_response()

    def setReplicatedAdapterDirectProxy_async(self, cb, adapter, replica, obj, current=None):
        self._adapters[adapter] = obj
        cb.ice_response()

    def setServerProcessProxy_async(self, id, proxy, current=None):
        cb.ice_response()

    def addObject(self, obj, current=None):
        self._objects[obj.ice_getIdentity()] = obj

    def getAdapter(self, adapter):
        if not self._adapters.has_key(adapter):
            raise Ice.AdapterNotFoundException()
        return self._adapters[adapter]

    def getObject(self, id):
        if not self._objects.has_key(id):
            raise Ice.ObjectNotFoundException()
        return self._objects[id]

class ServerLocator(Ice.Locator):

    def __init__(self, registry, registryPrx):
        self._registry = registry
        self._registryPrx = registryPrx

    def findObjectById_async(self, response, id, current=None):
        response.ice_response(self._registry.getObject(id))

    def findAdapterById_async(self, response, id, current=None):
        response.ice_response(self._registry.getAdapter(id))

    def getRegistry(self, current=None):
        return self._registryPrx

class ServerManagerI(Test.ServerManager):
    def __init__(self, adapter, registry, initData):
        self._adapter = adapter
        self._registry = registry
        self._communicators = []
        self._initData = initData
        self._initData.properties.setProperty("TestAdapter.Endpoints", "default")
        self._initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter")
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
        self._registry.addObject(adapter.add(object, communicator.stringToIdentity("test")))
        self._registry.addObject(adapter.add(object, communicator.stringToIdentity("test2")))

        adapter.activate()
        adapter2.activate()

    def shutdown(self, current=None):
        for i in self._communicators:
            i.destroy()
        self._adapter.getCommunicator().shutdown()

class HelloI(Test.Hello):
    def sayHello(self, current=None):
        pass

class TestI(Test.TestIntf):
    def __init__(self, adapter, adapter2, registry):
        self._adapter1 = adapter
        self._adapter2 = adapter2
        self._registry = registry
        self._registry.addObject(self._adapter1.add(HelloI(), communicator.stringToIdentity("hello")))

    def shutdown(self, current=None):
        self._adapter1.getCommunicator().shutdown()

    def getHello(self, current=None):
        return Test.HelloPrx.uncheckedCast(self._adapter1.createProxy(communicator.stringToIdentity("hello")))

    def migrateHello(self, current=None):
        id = communicator.stringToIdentity("hello")
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
    registry.addObject(adapter.createProxy(communicator.stringToIdentity("ServerManager")))
    object = ServerManagerI(adapter, registry, initData)
    adapter.add(object, communicator.stringToIdentity("ServerManager"))

    registryPrx = Ice.LocatorRegistryPrx.uncheckedCast(adapter.add(registry, communicator.stringToIdentity("registry")))

    locator = ServerLocator(registry, registryPrx)
    adapter.add(locator, communicator.stringToIdentity("locator"))

    adapter.activate()
    communicator.waitForShutdown()

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator, initData)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
