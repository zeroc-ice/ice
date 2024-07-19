# Copyright (c) ZeroC, Inc. All rights reserved.

from .Communicator import Communicator
from .PropertiesI import PropertiesI
from .ObjectAdapterI import ObjectAdapterI
from .ImplicitContextI import ImplicitContextI
from .LoggerI import LoggerI
from .Logger import Logger

class CommunicatorI(Communicator):
    def __init__(self, impl):
        self._impl = impl
        impl._setWrapper(self)

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self._impl.destroy()

    def getImpl(self):
        return self._impl

    def destroy(self):
        self._impl.destroy()

    def shutdown(self):
        self._impl.shutdown()

    def waitForShutdown(self):
        #
        # If invoked by the main thread, waitForShutdown only blocks for
        # the specified timeout in order to give us a chance to handle
        # signals.
        #
        while not self._impl.waitForShutdown(500):
            pass

    def isShutdown(self):
        return self._impl.isShutdown()

    def stringToProxy(self, str):
        return self._impl.stringToProxy(str)

    def proxyToString(self, obj):
        return self._impl.proxyToString(obj)

    def propertyToProxy(self, str):
        return self._impl.propertyToProxy(str)

    def proxyToProperty(self, obj, str):
        return self._impl.proxyToProperty(obj, str)

    def stringToIdentity(self, str):
        return self._impl.stringToIdentity(str)

    def identityToString(self, ident):
        return self._impl.identityToString(ident)

    def createObjectAdapter(self, name):
        adapter = self._impl.createObjectAdapter(name)
        return ObjectAdapterI(adapter)

    def createObjectAdapterWithEndpoints(self, name, endpoints):
        adapter = self._impl.createObjectAdapterWithEndpoints(name, endpoints)
        return ObjectAdapterI(adapter)

    def createObjectAdapterWithRouter(self, name, router):
        adapter = self._impl.createObjectAdapterWithRouter(name, router)
        return ObjectAdapterI(adapter)

    def getValueFactoryManager(self):
        return self._impl.getValueFactoryManager()

    def getImplicitContext(self):
        context = self._impl.getImplicitContext()
        if context is None:
            return None
        else:
            return ImplicitContextI(context)

    def getProperties(self):
        properties = self._impl.getProperties()
        return PropertiesI(properties)

    def getLogger(self):
        logger = self._impl.getLogger()
        if isinstance(logger, Logger):
            return logger
        else:
            return LoggerI(logger)

    def getDefaultRouter(self):
        return self._impl.getDefaultRouter()

    def setDefaultRouter(self, rtr):
        self._impl.setDefaultRouter(rtr)

    def getDefaultLocator(self):
        return self._impl.getDefaultLocator()

    def setDefaultLocator(self, loc):
        self._impl.setDefaultLocator(loc)

    def flushBatchRequests(self, compress):
        self._impl.flushBatchRequests(compress)

    def flushBatchRequestsAsync(self, compress):
        return self._impl.flushBatchRequestsAsync(compress)

    def createAdmin(self, adminAdapter, adminIdentity):
        return self._impl.createAdmin(adminAdapter, adminIdentity)

    def getAdmin(self):
        return self._impl.getAdmin()

    def addAdminFacet(self, servant, facet):
        self._impl.addAdminFacet(servant, facet)

    def findAdminFacet(self, facet):
        return self._impl.findAdminFacet(facet)

    def findAllAdminFacets(self):
        return self._impl.findAllAdminFacets()

    def removeAdminFacet(self, facet):
        return self._impl.removeAdminFacet(facet)
