"""
Ice module
"""

import sys, exceptions, string, imp, os
import IcePy

#
# Add some symbols to the Ice module.
#
ObjectPrx = IcePy.ObjectPrx
identityToString = IcePy.identityToString
stringToIdentity = IcePy.stringToIdentity
loadSlice = IcePy.loadSlice

#
# Core Ice types.
#
class Object(object):
    def ice_isA(self, id, current=None):
        return id in self.ice_ids()

    def ice_ping(self, current=None):
        pass

    def ice_ids(self, current=None):
        return [ self.ice_id() ]

    def ice_id(self, current=None):
        return '::Ice::Object'

    #
    # Do not define these here. They will be invoked if defined by a subclass.
    #
    #def ice_preMarshal(self):
    #    pass
    #
    #def ice_postUnmarshal(self):
    #    pass

class LocalObject(object):
    pass

#
# Exceptions.
#
class Exception(exceptions.Exception):
    def __str__(self):
        return self.__class__.__name__

class LocalException(Exception):
    def __init__(self, args=''):
        self.args = args

class UserException(Exception):
    pass

#
# Utilities.
#
def openModule(name):
    if sys.modules.has_key(name):
        result = sys.modules[name]
    else:
        result = createModule(name)

    return result

def createModule(name):
    l = string.split(name, ".")
    curr = ''
    mod = None

    for s in l:
        curr = curr + s

        if sys.modules.has_key(curr):
            mod = sys.modules[curr]
        else:
            nmod = imp.new_module(curr)
            if mod:
                setattr(mod, s, nmod)
            sys.modules[curr] = nmod
            mod = nmod

        curr = curr + "."

    return mod

def createTempClass():
    class __temp: pass
    return __temp

#
# Forward declarations.
#
IcePy._t_Object = IcePy.declareClass('::Ice::Object')
IcePy._t_ObjectPrx = IcePy.declareProxy('::Ice::Object')

#
# Slice checksum dictionary.
#
sliceChecksums = {}

#
# Import generated Ice modules.
#
import Ice_BuiltinSequences_ice
import Ice_Communicator_ice
import Ice_Current_ice
import Ice_Identity_ice
import Ice_LocalException_ice
import Ice_Locator_ice
import Ice_Logger_ice
import Ice_ObjectAdapter_ice
import Ice_ObjectFactory_ice
import Ice_Properties_ice
import Ice_Router_ice
import Ice_ServantLocator_ice

#
# Communicator wrapper.
#
class CommunicatorI(Communicator):
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
        return ObjectAdapterI(adapter)

    def createObjectAdapterWithEndpoints(self, name, endpoints):
        adapter = self._impl.createObjectAdapterWithEndpoints(name, endpoints)
        return ObjectAdapterI(adapter)

    def addObjectFactory(self, factory, id):
        self._impl.addObjectFactory(factory, id)

    def removeObjectFactory(self, id):
        self._impl.removeObjectFactory(id)

    def findObjectFactory(self, id):
        return self._impl.findObjectFactory(id)

    def getProperties(self):
        properties = self._impl.getProperties()
        return PropertiesI(properties)

    def getLogger(self):
        logger = self._impl.getLogger()
        return LoggerI(logger)

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

#
# Ice.initialize()
#
def initialize(args=[]):
    communicator = IcePy.initialize(args)
    return CommunicatorI(communicator)

#
# Ice.initializeWithProperties()
#
def initializeWithProperties(args, properties):
    communicator = IcePy.initializeWithProperties(args, properties._impl)
    return CommunicatorI(communicator)

#
# ObjectAdapter wrapper.
#
class ObjectAdapterI(ObjectAdapter):
    def __init__(self, impl):
        self._impl = impl

    def getName(self):
        return self._impl.getName()

    def getCommunicator(self):
        communicator = self._impl.getCommunicator()
        return CommunicatorI(communicator)

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

#
# Properties wrapper.
#
class PropertiesI(Properties):
    def __init__(self, impl):
        self._impl = impl

    def getProperty(self, key):
        return self._impl.getProperty(key)

    def getPropertyWithDefault(self, key, value):
        return self._impl.getPropertyWithDefault(key, value)

    def getPropertyAsInt(self, key):
        return self._impl.getPropertyAsInt(key)

    def getPropertyAsIntWithDefault(self, key, value):
        return self._impl.getPropertyAsIntWithDefault(key, value)

    def getPropertiesForPrefix(self, prefix):
        return self._impl.getPropertiesForPrefix(prefix)

    def setProperty(self, key, value):
        self._impl.setProperty(key, value)

    def getCommandLineOptions(self):
        return self._impl.getCommandLineOptions()

    def parseCommandLineOptions(self, prefix, options):
        self._impl.parseCommandLineOptions(prefix, options)

    def parseIceCommandLineOptions(self, options):
        self._impl.parseIceCommandLineOptions(options)

    def load(self, file):
        self._impl.load(file)

    def clone(self):
        properties = self._impl.clone()
        return PropertiesI(properties)

    def __iter__(self):
        dict = self._impl.getPropertiesForPrefix('')
        return iter(dict)

    def __str__(self):
        return str(self._impl)

#
# Ice.createProperties()
#
def createProperties(args=[]):
    properties = IcePy.createProperties(args)
    return PropertiesI(properties)

#
# Ice.getDefaultProperties()
#
def getDefaultProperties(args=[]):
    properties = IcePy.getDefaultProperties(args)
    return PropertiesI(properties)

#
# Application class.
#
class Application(object):
    def __init__(self):
        pass

    def __del__(self):
        pass

    def main(self, args, configFile=None):
        if Application._communicator:
            print args[0] + ": only one instance of the Application class can be used"
            return False

        Application._interrupted = False
        Application._appName = args[0]

        status = 0

        try:
            if configFile:
                properties = createProperties(args)
                properties.load(configFile)
                Application._communicator = initializeWithProperties(args, properties)
            else:
                Application._communicator = initialize(args)

            #
            # The default is to destroy when a signal is received.
            #
            Application.destroyOnInterrupt()

            status = self.run(args)
        except Exception, ex:
            print Application._appName + ": " + str(ex)
            status = 1
        except exceptions.Exception, ex:
            print Application._appName + ": " + str(ex)
            status = 1

        if Application._communicator:
            #
            # We don't want to handle signals anymore.
            #
            Application.ignoreInterrupt()

            try:
                Application._communicator.destroy()
            except Exception, ex:
                print Application._appName + ": " + str(ex)
                status = 1

            Application._communicator = None

        return status

    def run(self, args):
        raise RuntimeError('run() not implemented')

    def appName():
        return Application._appName
    appName = staticmethod(appName)

    def communicator():
        return Application._communicator
    communicator = staticmethod(communicator)

    def destroyOnInterrupt():
        pass
    destroyOnInterrupt = staticmethod(destroyOnInterrupt)

    def shutdownOnInterrupt():
        pass
    shutdownOnInterrupt = staticmethod(shutdownOnInterrupt)

    def ignoreInterrupt():
        pass
    ignoreInterrupt = staticmethod(ignoreInterrupt)

    def holdInterrupt():
        pass
    holdInterrupt = staticmethod(holdInterrupt)

    def releaseInterrupt():
        pass
    releaseInterrupt = staticmethod(releaseInterrupt)

    def interrupted():
        pass
    interrupted = staticmethod(interrupted)

    _appName = None
    _communicator = None
    _interrupted = False
    _released = False

#
# Define Ice::Object and Ice::ObjectPrx.
#
IcePy._t_Object = IcePy.defineClass('::Ice::Object', Object, False, None, (), ())
IcePy._t_ObjectPrx = IcePy.defineProxy('::Ice::Object', ObjectPrx)

Object._op_ice_isA = IcePy.Operation('ice_isA', OperationMode.Nonmutating, False, (IcePy._t_string,), (), IcePy._t_bool, ())
Object._op_ice_ping = IcePy.Operation('ice_ping', OperationMode.Nonmutating, False, (), (), None, ())
Object._op_ice_ids = IcePy.Operation('ice_ids', OperationMode.Nonmutating, False, (), (), _t_StringSeq, ())
Object._op_ice_id = IcePy.Operation('ice_id', OperationMode.Nonmutating, False, (), (), IcePy._t_string, ())

#
# Annotate Ice::Identity.
#
def Identity__str__(self):
    return IcePy.identityToString(self)
Identity.__str__ = Identity__str__
del Identity__str__

def Identity__hash__(self):
    #
    # Return 32-bit integer.
    #
    return (5 * hash(self.category) + hash(self.name)) % 0x7fffffff
Identity.__hash__ = Identity__hash__
del Identity__hash__

def Identity__lt__(self, other):
    if self.category < other.category:
        return True
    elif self.category == other.category:
        return self.name < other.name
    return False
Identity.__lt__ = Identity__lt__
del Identity__lt__

def Identity__le__(self, other):
    return self.__lt__(other) or self.__eq__(other)
Identity.__le__ = Identity__le__
del Identity__le__

def Identity__eq__(self, other):
    return self.category == other.category and self.name == other.name
Identity.__eq__ = Identity__eq__
del Identity__eq__

def Identity__ne__(self, other):
    return not self.__eq__(other)
Identity.__ne__ = Identity__ne__
del Identity__ne__

def Identity__gt__(self, other):
    if self.category > other.category:
        return True
    elif self.category == other.category:
        return self.name > other.name
    return False
Identity.__gt__ = Identity__gt__
del Identity__gt__

def Identity__ge__(self, other):
    return self.__gt__(other) or self.__eq__(other)
Identity.__ge__ = Identity__ge__
del Identity__ge__

#
# Proxy comparison functions.
#
def proxyIdentityEqual(lhs, rhs):
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError('argument is not a proxy')
    if not lhs and not rhs:
        return True
    elif not lhs and rhs:
        return False
    elif lhs and not rhs:
        return False
    else:
        return lhs.ice_getIdentity() == rhs.ice_getIdentity()

def proxyIdentityAndFacetEqual(lhs, rhs):
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError('argument is not a proxy')
    if not lhs and not rhs:
        return True
    elif not lhs and rhs:
        return False
    elif lhs and not rhs:
        return False
    else:
        return lhs.ice_getIdentity() == rhs.ice_getIdentity() and lhs.ice_getFacet() == rhs.ice_getFacet()
