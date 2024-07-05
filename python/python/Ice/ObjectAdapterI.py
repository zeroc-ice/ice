#
# ObjectAdapter wrapper.
#

from .ObjectAdapter import ObjectAdapter

class ObjectAdapterI(ObjectAdapter):
    def __init__(self, impl):
        self._impl = impl

    def getName(self):
        return self._impl.getName()

    def getCommunicator(self):
        communicator = self._impl.getCommunicator()
        return communicator._getWrapper()

    def activate(self):
        self._impl.activate()

    def hold(self):
        self._impl.hold()

    def waitForHold(self):
        #
        # If invoked by the main thread, waitForHold only blocks for
        # the specified timeout in order to give us a chance to handle
        # signals.
        #
        while not self._impl.waitForHold(1000):
            pass

    def deactivate(self):
        self._impl.deactivate()

    def waitForDeactivate(self):
        #
        # If invoked by the main thread, waitForDeactivate only blocks for
        # the specified timeout in order to give us a chance to handle
        # signals.
        #
        while not self._impl.waitForDeactivate(1000):
            pass

    def isDeactivated(self):
        self._impl.isDeactivated()

    def destroy(self):
        self._impl.destroy()

    def add(self, servant, id):
        return self._impl.add(servant, id)

    def addFacet(self, servant, id, facet):
        return self._impl.addFacet(servant, id, facet)

    def addWithUUID(self, servant):
        return self._impl.addWithUUID(servant)

    def addFacetWithUUID(self, servant, facet):
        return self._impl.addFacetWIthUUID(servant, facet)

    def addDefaultServant(self, servant, category):
        self._impl.addDefaultServant(servant, category)

    def remove(self, id):
        return self._impl.remove(id)

    def removeFacet(self, id, facet):
        return self._impl.removeFacet(id, facet)

    def removeAllFacets(self, id):
        return self._impl.removeAllFacets(id)

    def removeDefaultServant(self, category):
        return self._impl.removeDefaultServant(category)

    def find(self, id):
        return self._impl.find(id)

    def findFacet(self, id, facet):
        return self._impl.findFacet(id, facet)

    def findAllFacets(self, id):
        return self._impl.findAllFacets(id)

    def findByProxy(self, proxy):
        return self._impl.findByProxy(proxy)

    def findDefaultServant(self, category):
        return self._impl.findDefaultServant(category)

    def addServantLocator(self, locator, category):
        self._impl.addServantLocator(locator, category)

    def removeServantLocator(self, category):
        return self._impl.removeServantLocator(category)

    def findServantLocator(self, category):
        return self._impl.findServantLocator(category)

    def createProxy(self, id):
        return self._impl.createProxy(id)

    def createDirectProxy(self, id):
        return self._impl.createDirectProxy(id)

    def createIndirectProxy(self, id):
        return self._impl.createIndirectProxy(id)

    def createReverseProxy(self, id):
        return self._impl.createReverseProxy(id)

    def setLocator(self, loc):
        self._impl.setLocator(loc)

    def getLocator(self):
        return self._impl.getLocator()

    def getEndpoints(self):
        return self._impl.getEndpoints()

    def refreshPublishedEndpoints(self):
        self._impl.refreshPublishedEndpoints()

    def getPublishedEndpoints(self):
        return self._impl.getPublishedEndpoints()

    def setPublishedEndpoints(self, newEndpoints):
        self._impl.setPublishedEndpoints(newEndpoints)
