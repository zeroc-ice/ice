# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, IcePy
import ObjectAdapter_ice
import CommunicatorImpl

__name__ = 'Ice'
_M_Ice = Ice.openModule('Ice')

class ObjectAdapterI(_M_Ice.ObjectAdapter):
    def __init__(self, impl):
        self._impl = impl

    def getName(self):
        return self._impl.getName()

    def getCommunicator(self):
        communicator = self._impl.getCommunicator()
        return _M_Ice.CommunicatorI(communicator)

    def activate(self):
        self._impl.activate()

    def hold(self):
        self._impl.hold()

    def waitForHold(self):
        self._impl.waitForHold()

    def deactivate(self):
        self._impl.deactivate()

    def waitForDeactivate(self):
        self._impl.waitForDeactivate()

    def add(self, servant, id):
        return self._impl.add(servant, id)

    def addFacet(self, servant, id, facet):
        return self._impl.addFacet(servant, id, facet)

    def addWithUUID(self, servant):
        return self._impl.addWithUUID(servant)

    def addFacetWithUUID(self, servant, facet):
        return self._impl.addFacetWIthUUID(servant, facet)

    def remove(self, id):
        return self._impl.remove(id)

    def removeFacet(self, id, facet):
        return self._impl.removeFacet(id, facet)

    def removeAllFacets(self, id):
        return self._impl.removeAllFacets(id)

    def find(self, id):
        return self._impl.find(id)

    def findFacet(self, id, facet):
        return self._impl.findFacet(id, facet)

    def findAllFacets(self, id):
        return self._impl.findAllFacets(id)

    def findByProxy(self, proxy):
        return self._impl.findByProxy(proxy)

    def addServantLocator(self, locator, category):
        self._impl.addServantLocator(locator, category)

    def findServantLocator(self, category):
        return self._impl.findServantLocator(category)

    def createProxy(self, id):
        return self._impl.createProxy(id)

    def createDirectProxy(self, id):
        return self._impl.createDirectProxy(id)

    def createReverseProxy(self, id):
        return self._impl.createReverseProxy(id)

    def addRouter(self, rtr):
        self._impl.addRouter(rtr)

    def setLocator(self, loc):
        self._impl.setLocator(loc)

_M_Ice.ObjectAdapterI = ObjectAdapterI
del ObjectAdapterI
