# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, IcePy
import Communicator_ice
import ObjectAdapterImpl
import PropertiesImpl
import LoggerImpl

__name__ = 'Ice'
_M_Ice = Ice.openModule('Ice')

class CommunicatorI(_M_Ice.Communicator):
    def __init__(self, impl):
        self._impl = impl

    def destroy(self):
        self._impl.destroy()

    def shutdown(self):
        self._impl.shutdown()

    def waitForShutdown(self):
        self._impl.waitForShutdown()

    def stringToProxy(self, str):
        return self._impl.stringToProxy(str)

    def proxyToString(self, obj):
        return self._impl.proxyToString(obj)

    def createObjectAdapter(self, name):
        adapter = self._impl.createObjectAdapter(name)
        return _M_Ice.ObjectAdapterI(adapter)

    def createObjectAdapterWithEndpoints(self, name, endpoints):
        adapter = self._impl.createObjectAdapterWithEndpoints(name, endpoints)
        return _M_Ice.ObjectAdapterI(adapter)

    def addObjectFactory(self, factory, id):
        self._impl.addObjectFactory(factory, id)

    def removeObjectFactory(self, id):
        self._impl.removeObjectFactory(id)

    def findObjectFactory(self, id):
        return self._impl.findObjectFactory(id)

    def getProperties(self):
        properties = self._impl.getProperties()
        return _M_Ice.PropertiesI(properties)

    def getLogger(self):
        logger = self._impl.getLogger()
        return _M_Ice.LoggerI(logger)

    def setLogger(self, log):
        self._impl.setLogger(log)

    def getStats(self):
        raise RuntimeError("operation `getStats' not implemented")

    def setStats(self, st):
        raise RuntimeError("operation `setStats' not implemented")

    def getDefaultRouter(self):
        return self._impl.getDefaultRouter()

    def setDefaultRouter(self, rtr):
        self._impl.setDefaultRouter(rtr)

    def getDefaultLocator(self):
        return self._impl.getDefaultLocator()

    def setDefaultLocator(self, loc):
        self._impl.setDefaultLocator(loc)

    def getPluginManager(self):
        raise RuntimeError("operation `getPluginManager' not implemented")

    def flushBatchRequests(self):
        self._impl.flushBatchRequests()

_M_Ice.CommunicatorI = CommunicatorI
del CommunicatorI

def initialize(args=[]):
    communicator = IcePy.initialize(args)
    return _M_Ice.CommunicatorI(communicator)

_M_Ice.initialize = initialize
del initialize

def initializeWithProperties(args, properties):
    communicator = IcePy.initializeWithProperties(args, properties._impl)
    return _M_Ice.CommunicatorI(communicator)

_M_Ice.initializeWithProperties = initializeWithProperties
del initializeWithProperties
