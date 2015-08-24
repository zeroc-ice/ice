# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

"""
Ice module
"""

import sys, string, imp, os, threading, warnings, datetime

#
# RTTI problems can occur in C++ code unless we modify Python's dlopen flags.
# Note that changing these flags might cause problems for other extensions
# loaded by the application (see bug 3660), so we restore the original settings
# after loading IcePy.
#
_dlopenflags = -1
try:
    _dlopenflags = sys.getdlopenflags()

    try:
        import dl
        sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)
    except ImportError:
        #
        # If the dl module is not available and we're running on a Linux
        # platform, use the hard coded value of RTLD_NOW|RTLD_GLOBAL.
        #
        if sys.platform.startswith("linux"):
            sys.setdlopenflags(258)
        pass

except AttributeError:
    #
    # sys.getdlopenflags() is not supported (we're probably running on Windows).
    #
    pass

#
# Import the Python extension.
#
import IcePy

#
# Restore the dlopen flags.
#
if _dlopenflags >= 0:
    sys.setdlopenflags(_dlopenflags)

#
# Give the extension an opportunity to clean up before a graceful exit.
#
import atexit
atexit.register(IcePy.cleanup)

#
# Add some symbols to the Ice module.
#
ObjectPrx = IcePy.ObjectPrx
stringVersion = IcePy.stringVersion
intVersion = IcePy.intVersion
currentProtocol = IcePy.currentProtocol
currentProtocolEncoding = IcePy.currentProtocolEncoding
currentEncoding = IcePy.currentEncoding
stringToProtocolVersion = IcePy.stringToProtocolVersion
protocolVersionToString = IcePy.protocolVersionToString
stringToEncodingVersion = IcePy.stringToEncodingVersion
encodingVersionToString = IcePy.encodingVersionToString
generateUUID = IcePy.generateUUID
loadSlice = IcePy.loadSlice
AsyncResult = IcePy.AsyncResult
Unset = IcePy.Unset

#
# This value is used as the default value for struct types in the constructors
# of user-defined types. It allows us to determine whether the application has
# supplied a value. (See bug 3676)
#
_struct_marker = object()

#
# Core Ice types.
#
class Object(object):
    def ice_isA(self, id, current=None):
        '''Determines whether the target object supports the interface denoted
by the given Slice type id.
Arguments:
    id The Slice type id
Returns:
    True if the target object supports the interface, or false otherwise.
'''
        return id in self.ice_ids(current)

    def ice_ping(self, current=None):
        '''A reachability test for the target object.'''
        pass

    def ice_ids(self, current=None):
        '''Obtains the type ids corresponding to the Slice interface
that are supported by the target object.
Returns:
    A list of type ids.
'''
        return [ self.ice_id(current) ]

    def ice_id(self, current=None):
        '''Obtains the type id corresponding to the most-derived Slice
interface supported by the target object.
Returns:
    The type id.
'''
        return '::Ice::Object'

    def ice_staticId():
        '''Obtains the type id of this Slice class or interface.
Returns:
    The type id.
'''
        return '::Ice::Object'
    ice_staticId = staticmethod(ice_staticId)

    #
    # Do not define these here. They will be invoked if defined by a subclass.
    #
    #def ice_preMarshal(self):
    #    pass
    #
    #def ice_postUnmarshal(self):
    #    pass

#
# LocalObject is deprecated; use the Python base 'object' type instead.
#
class LocalObject(object):
    pass

class Blobject(Object):
    '''Special-purpose servant base class that allows a subclass to
handle synchronous Ice invocations as "blobs" of bytes.'''

    def ice_invoke(self, bytes, current):
        '''Dispatch a synchronous Ice invocation. The operation's
arguments are encoded in the bytes argument. The return
value must be a tuple of two values: the first is a
boolean indicating whether the operation succeeded (True)
or raised a user exception (False), and the second is
the encoded form of the operation's results or the user
exception.
'''
        pass

class BlobjectAsync(Object):
    '''Special-purpose servant base class that allows a subclass to
handle asynchronous Ice invocations as "blobs" of bytes.'''

    def ice_invoke_async(self, cb, bytes, current):
        '''Dispatch an asynchronous Ice invocation. The operation's
arguments are encoded in the bytes argument. When the
dispatch is complete, the subclass can invoke either
ice_response or ice_exception on the supplied callback
object.
'''
        pass

#
# Exceptions.
#
class Exception(Exception):     # Derives from built-in base 'Exception' class.
    '''The base class for all Ice exceptions.'''
    def __str__(self):
        return self.__class__.__name__

    def ice_name(self):
        '''Returns the type name of this exception.'''
        return self._ice_name

class LocalException(Exception):
    '''The base class for all Ice run-time exceptions.'''
    def __init__(self, args=''):
        self.args = args

class UserException(Exception):
    '''The base class for all user-defined exceptions.'''
    pass

class EnumBase(object):
    def __init__(self, _n, _v):
        self._name = _n
        self._value = _v

    def __str__(self):
        return self._name

    __repr__ = __str__

    def __hash__(self):
        return self._value

    def __lt__(self, other):
        if isinstance(other, self.__class__):
            return self._value < other._value;
        elif other == None:
            return False
        return NotImplemented

    def __le__(self, other):
        if isinstance(other, self.__class__):
            return self._value <= other._value;
        elif other == None:
            return False
        return NotImplemented

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self._value == other._value;
        elif other == None:
            return False
        return NotImplemented

    def __ne__(self, other):
        if isinstance(other, self.__class__):
            return self._value != other._value;
        elif other == None:
            return False
        return NotImplemented

    def __gt__(self, other):
        if isinstance(other, self.__class__):
            return self._value > other._value;
        elif other == None:
            return False
        return NotImplemented

    def __ge__(self, other):
        if isinstance(other, self.__class__):
            return self._value >= other._value;
        elif other == None:
            return False
        return NotImplemented

    def _getName(self):
        return self._name

    def _getValue(self):
        return self._value

    name = property(_getName)
    value = property(_getValue)

class SlicedData(object):
    #
    # Members:
    #
    # slices - tuple of SliceInfo
    #
    pass

class SliceInfo(object):
    #
    # Members:
    #
    # typeId - string
    # compactId - int
    # bytes - string
    # objects - tuple of Ice.Object
    pass

#
# Native PropertiesAdmin admin facet.
#
NativePropertiesAdmin = IcePy.NativePropertiesAdmin

class PropertiesAdminUpdateCallback(object):
    '''Callback class to get notifications of property updates passed
    through the Properties admin facet'''

    def updated(self, props):
        pass

class UnknownSlicedObject(Object):
    #
    # Members:
    #
    # unknownTypeId - string
    pass

def getSliceDir():
    '''Convenience function for locating the directory containing the Slice files.'''

    #
    # Detect setup.py installation in site-packages. The slice
    # files live along side Ice.py
    #
    dir = os.path.join(os.path.dirname(__file__), "slice")
    if os.path.isdir(dir):
        return dir

    #
    # Get the parent of the directory containing this file (Ice.py).
    #
    pyHome = os.path.join(os.path.dirname(__file__), "..")

    #
    # For an installation from a source distribution, a binary tarball, or a
    # Windows installer, the "slice" directory is a sibling of the "python"
    # directory.
    #
    dir = os.path.join(pyHome, "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    #
    # In a source distribution, the "slice" directory is one level higher.
    #
    dir = os.path.join(pyHome, "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    iceVer = stringVersion()

    if sys.platform[:5] == "linux":
        #
        # Check the default RPM location.
        #
        dir = os.path.join("/", "usr", "share", "Ice-" + iceVer, "slice")
        if os.path.exists(dir):
            return dir

    elif sys.platform == "darwin":
        #
        # Check the default OS X location.
        #
        dir = os.path.join("/", "Library", "Developer", "Ice-" + iceVer, "slice")
        if os.path.exists(dir):
            return dir

    return None

#
# Utilities for use by generated code.
#

_pendingModules = {}

def openModule(name):
    global _pendingModules
    if name in sys.modules:
        result = sys.modules[name]
    elif name in _pendingModules:
        result = _pendingModules[name]
    else:
        result = createModule(name)

    return result

def createModule(name):
    global _pendingModules
    l = name.split(".")
    curr = ''
    mod = None

    for s in l:
        curr = curr + s

        if curr in sys.modules:
            mod = sys.modules[curr]
        elif curr in _pendingModules:
            mod = _pendingModules[curr]
        else:
            nmod = imp.new_module(curr)
            _pendingModules[curr] = nmod
            mod = nmod

        curr = curr + "."

    return mod

def updateModule(name):
    global _pendingModules
    if name in _pendingModules:
        pendingModule = _pendingModules[name]
        mod = sys.modules[name]
        mod.__dict__.update(pendingModule.__dict__)
        del _pendingModules[name]

def updateModules():
    global _pendingModules
    for name in _pendingModules.keys():
        if name in sys.modules:
            sys.modules[name].__dict__.update(_pendingModules[name].__dict__)
        else:
            sys.modules[name] = _pendingModules[name]
    _pendingModules = {}

def createTempClass():
    class __temp: pass
    return __temp

class FormatType(object):
    def __init__(self, val):
        assert(val >= 0 and val < 3)
        self.value = val

FormatType.DefaultFormat = FormatType(0)
FormatType.CompactFormat = FormatType(1)
FormatType.SlicedFormat = FormatType(2)

#
# Forward declarations.
#
IcePy._t_Object = IcePy.declareClass('::Ice::Object')
IcePy._t_ObjectPrx = IcePy.declareProxy('::Ice::Object')
IcePy._t_LocalObject = IcePy.declareClass('::Ice::LocalObject')

#
# Sequence mappings.
#
IcePy.SEQ_DEFAULT = 0
IcePy.SEQ_TUPLE = 1
IcePy.SEQ_LIST = 2
#IcePy.SEQ_ARRAY = 3

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
import Ice_ImplicitContext_ice
import Ice_Endpoint_ice
import Ice_EndpointTypes_ice
import Ice_Identity_ice
import Ice_LocalException_ice
import Ice_Locator_ice
import Ice_Logger_ice
import Ice_ObjectAdapter_ice
import Ice_ObjectFactory_ice
import Ice_Process_ice
import Ice_Properties_ice
import Ice_RemoteLogger_ice
import Ice_Router_ice
import Ice_ServantLocator_ice
import Ice_Connection_ice
import Ice_Version_ice
import Ice_Instrumentation_ice
import Ice_Metrics_ice

#
# Replace EndpointInfo with our implementation.
#
del EndpointInfo
EndpointInfo =  IcePy.EndpointInfo
del IPEndpointInfo
IPEndpointInfo =  IcePy.IPEndpointInfo
del TCPEndpointInfo
TCPEndpointInfo =  IcePy.TCPEndpointInfo
del UDPEndpointInfo
UDPEndpointInfo =  IcePy.UDPEndpointInfo
del WSEndpointInfo
WSEndpointInfo =  IcePy.WSEndpointInfo
del OpaqueEndpointInfo
OpaqueEndpointInfo =  IcePy.OpaqueEndpointInfo

SSLEndpointInfo = IcePy.SSLEndpointInfo
WSSEndpointInfo = IcePy.WSSEndpointInfo

#
# Replace ConnectionInfo with our implementation.
#
del ConnectionInfo
ConnectionInfo =  IcePy.ConnectionInfo
del IPConnectionInfo
IPConnectionInfo =  IcePy.IPConnectionInfo
del TCPConnectionInfo
TCPConnectionInfo =  IcePy.TCPConnectionInfo
del UDPConnectionInfo
UDPConnectionInfo =  IcePy.UDPConnectionInfo
del WSConnectionInfo
WSConnectionInfo =  IcePy.WSConnectionInfo

SSLConnectionInfo =  IcePy.SSLConnectionInfo
WSSConnectionInfo =  IcePy.WSSConnectionInfo

class ThreadNotification(object):
    '''Base class for thread notification callbacks. A subclass must
define the start and stop methods.'''

    def __init__(self):
        pass

    def start():
        '''Invoked in the context of a thread created by the Ice run time.'''
        pass

    def stop():
        '''Invoked in the context of an Ice run-time thread that is about
to terminate.'''
        pass

class BatchRequest(object):
    '''Base class for batch request interceptor. A subclass must
define the enqueue method.'''
    def __init__(self, size, operation, proxy):
        self._size = size
        self._operation = operation
        self._proxy = proxy

    def getSize():
        return self._size

    def getOperation():
        return self._operation

    def getProxy():
        return self._proxy

    def enqueue():
        '''Call enqueue from the batch request interceptor enqueue
implementation to confirm the batching a this request.'''
        pass

class BatchRequestInterceptor(object):
    '''Base class for batch request interceptor. A subclass must
define the enqueue method.'''

    def __init__(self):
        pass

    def enqueue(request, queueCount, queueSize):
        '''Invoked when a request is batched.'''
        pass

#
# Initialization data.
#
class InitializationData(object):
    '''The attributes of this class are used to initialize a new
communicator instance. The supported attributes are as follows:

properties: An instance of Ice.Properties. You can use the
    Ice.createProperties function to create a new property set.

logger: An instance of Ice.Logger.

threadHook: An object that implements ThreadNotification.
'''
    def __init__(self):
        self.properties = None
        self.logger = None
        self.threadHook = None
        self.batchRequestInterceptor = None

#
# Communicator wrapper.
#
class CommunicatorI(Communicator):
    def __init__(self, impl):
        self._impl = impl
        impl._setWrapper(self)

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

    def addObjectFactory(self, factory, id):
        self._impl.addObjectFactory(factory, id)

    def findObjectFactory(self, id):
        return self._impl.findObjectFactory(id)

    def getImplicitContext(self):
        context = self._impl.getImplicitContext()
        if context == None:
            return None;
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

    def getStats(self):
        raise RuntimeError("operation `getStats' not implemented")

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

    def begin_flushBatchRequests(self, _ex=None, _sent=None):
        return self._impl.begin_flushBatchRequests(_ex, _sent)

    def end_flushBatchRequests(self, r):
        return self._impl.end_flushBatchRequests(r)

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

#
# Ice.initialize()
#
def initialize(args=None, data=None):
    '''Initializes a new communicator. The optional arguments represent
an argument list (such as sys.argv) and an instance of InitializationData.
You can invoke this function as follows:

Ice.initialize()
Ice.initialize(args)
Ice.initialize(data)
Ice.initialize(args, data)

If you supply an argument list, the function removes those arguments from
the list that were recognized by the Ice run time.
'''
    communicator = IcePy.Communicator(args, data)
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

    def refreshPublishedEndpoints(self):
        self._impl.refreshPublishedEndpoints()

    def getEndpoints(self):
        return self._impl.getEndpoints()

    def getPublishedEndpoints(self):
        return self._impl.getPublishedEndpoints()

#
# Logger wrapper.
#
class LoggerI(Logger):
    def __init__(self, impl):
        self._impl = impl

    def _print(self, message):
        return self._impl._print(message)

    def trace(self, category, message):
        return self._impl.trace(category, message)

    def warning(self, message):
        return self._impl.warning(message)

    def error(self, message):
        return self._impl.error(message)

    def getPrefix(self):
        return self._impl.getPrefix()

    def cloneWithPrefix(self, prefix):
        logger = self._impl.cloneWithPrefix(prefix)
        return LoggerI(logger)

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

    def getPropertyAsList(self, key):
        return self._impl.getPropertyAsList(key)

    def getPropertyAsListWithDefault(self, key, value):
        return self._impl.getPropertyAsListWithDefault(key, value)

    def getPropertiesForPrefix(self, prefix):
        return self._impl.getPropertiesForPrefix(prefix)

    def setProperty(self, key, value):
        self._impl.setProperty(key, value)

    def getCommandLineOptions(self):
        return self._impl.getCommandLineOptions()

    def parseCommandLineOptions(self, prefix, options):
        return self._impl.parseCommandLineOptions(prefix, options)

    def parseIceCommandLineOptions(self, options):
        return self._impl.parseIceCommandLineOptions(options)

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
def createProperties(args=None, defaults=None):
    '''Creates a new property set. The optional arguments represent
an argument list (such as sys.argv) and a property set that supplies
default values. You can invoke this function as follows:

Ice.createProperties()
Ice.createProperties(args)
Ice.createProperties(defaults)
Ice.createProperties(args, defaults)

If you supply an argument list, the function removes those arguments
from the list that were recognized by the Ice run time.
'''

    properties = IcePy.createProperties(args, defaults)
    return PropertiesI(properties)

#
# Ice.getProcessLogger()
# Ice.setProcessLogger()
#
def getProcessLogger():
    '''Returns the default logger object.'''
    logger = IcePy.getProcessLogger()
    if isinstance(logger, Logger):
        return logger
    else:
        return LoggerI(logger)

def setProcessLogger(logger):
    '''Sets the default logger object.'''
    IcePy.setProcessLogger(logger)

#
# ImplicitContext wrapper
#
class ImplicitContextI(ImplicitContext):
    def __init__(self, impl):
        self._impl = impl

    def setContext(self, ctx):
        self._impl.setContext(ctx)

    def getContext(self):
        return self._impl.getContext()

    def containsKey(self, key):
        return self._impl.containsKey(key)

    def get(self, key):
        return self._impl.get(key)

    def put(self, key, value):
        return self._impl.put(key, value)

    def remove(self, key):
        return self._impl.remove(key)


#
# Its not possible to block in a python signal handler since this
# blocks the main thread from doing further work. As such we queue the
# signal with a worker thread which then "dispatches" the signal to
# the registered callback object.
#
# Note the interface is the same as the C++ CtrlCHandler
# implementation, however, the implementation is different.
#
class CtrlCHandler(threading.Thread):
    # Class variable referring to the one and only handler for use
    # from the signal handling callback.
    _self = None

    def __init__(self):
        threading.Thread.__init__(self)

        if CtrlCHandler._self != None:
            raise RuntimeError("Only a single instance of a CtrlCHandler can be instantiated.")
        CtrlCHandler._self = self

        # State variables. These are not class static variables.
        self._condVar = threading.Condition()
        self._queue = []
        self._done = False
        self._callback = None

        #
        # Setup and install signal handlers
        #
        if 'SIGHUP' in signal.__dict__:
            signal.signal(signal.SIGHUP, CtrlCHandler.signalHandler)
        if 'SIGBREAK' in signal.__dict__:
            signal.signal(signal.SIGBREAK, CtrlCHandler.signalHandler)
        signal.signal(signal.SIGINT, CtrlCHandler.signalHandler)
        signal.signal(signal.SIGTERM, CtrlCHandler.signalHandler)

        # Start the thread once everything else is done.
        self.start()

    # Dequeue and dispatch signals.
    def run(self):
        while True:
            self._condVar.acquire()
            while len(self._queue) == 0 and not self._done:
                self._condVar.wait()
            if self._done:
                self._condVar.release()
                break
            sig, callback = self._queue.pop()
            self._condVar.release()
            if callback:
                callback(sig)

    # Destroy the object. Wait for the thread to terminate and cleanup
    # the internal state.
    def destroy(self):
        self._condVar.acquire()
        self._done = True
        self._condVar.notify()
        self._condVar.release()

        # Wait for the thread to terminate
        self.join()
        #
        # Cleanup any state set by the CtrlCHandler.
        #
        if 'SIGHUP' in signal.__dict__:
            signal.signal(signal.SIGHUP, signal.SIG_DFL)
        if 'SIGBREAK' in signal.__dict__:
            signal.signal(signal.SIGBREAK, signal.SIG_DFL)
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        CtrlCHandler._self = None

    def setCallback(self, callback):
        self._condVar.acquire()
        self._callback = callback
        self._condVar.release()

    def getCallback(self):
        self._condVar.acquire()
        callback = self._callback
        self._condVar.release()
        return callback

    # Private. Only called by the signal handling mechanism.
    def signalHandler(self, sig, frame):
        self._self._condVar.acquire()
        #
        # The signal AND the current callback are queued together.
        #
        self._self._queue.append([sig, self._self._callback])
        self._self._condVar.notify()
        self._self._condVar.release()
    signalHandler = classmethod(signalHandler)

#
# Application logger.
#
class _ApplicationLoggerI(Logger):
    def __init__(self, prefix):
        if len(prefix) > 0:
            self._prefix = prefix + ": "
        else:
            self._prefix = ""
        self._outputMutex = threading.Lock()

    def _print(self, message):
        s = "[ " + str(datetime.datetime.now()) + " " + self._prefix
        self._outputMutex.acquire()
        sys.stderr.write(message + "\n")
        self._outputMutex.release()

    def trace(self, category, message):
        s = "[ " + str(datetime.datetime.now()) + " " + self._prefix
        if len(category) > 0:
            s += category + ": "
        s += message + " ]"

        s = s.replace("\n", "\n  ")

        self._outputMutex.acquire()
        sys.stderr.write(s + "\n")
        self._outputMutex.release()

    def warning(self, message):
        self._outputMutex.acquire()
        sys.stderr.write(str(datetime.datetime.now()) + " " + self._prefix + "warning: " + message + "\n")
        self._outputMutex.release()

    def error(self, message):
        self._outputMutex.acquire()
        sys.stderr.write(str(datetime.datetime.now()) + " " + self._prefix + "error: " + message + "\n")
        self._outputMutex.release()

#
# Application class.
#
import signal, traceback
class Application(object):
    '''Convenience class that initializes a communicator and reacts
gracefully to signals. An application must define a subclass
of this class and supply an implementation of the run method.
'''

    def __init__(self, signalPolicy=0): # HandleSignals=0
        '''The constructor accepts an optional argument indicating
whether to handle signals. The value should be either
Application.HandleSignals (the default) or
Application.NoSignalHandling.
'''
        if type(self) == Application:
            raise RuntimeError("Ice.Application is an abstract class")
        Application._signalPolicy = signalPolicy

    def main(self, args, configFile=None, initData=None):
        '''The main entry point for the Application class. The arguments
are an argument list (such as sys.argv), the name of an Ice
configuration file (optional), and an instance of
InitializationData (optional). This method does not return
until after the completion of the run method. The return
value is an integer representing the exit status.
'''

        if Application._communicator:
            getProcessLogger().error(args[0] + ": only one instance of the Application class can be used")
            return 1

        #
        # We parse the properties here to extract Ice.ProgramName.
        #
        if not initData:
            initData = InitializationData()
        if configFile:
            try:
                initData.properties = createProperties(None, initData.properties)
                initData.properties.load(configFile)
            except:
                getProcessLogger().error(traceback.format_exc())
                return 1
        initData.properties = createProperties(args, initData.properties)

        #
        #  If the process logger is the default logger, we replace it with a
        #  a logger which is using the program name for the prefix.
        #
        if isinstance(getProcessLogger(), LoggerI):
            setProcessLogger(_ApplicationLoggerI(initData.properties.getProperty("Ice.ProgramName")))

        #
        # Install our handler for the signals we are interested in. We assume main()
        # is called from the main thread.
        #
        Application._ctrlCHandler = CtrlCHandler()

        try:
            Application._interrupted = False
            Application._appName = initData.properties.getPropertyWithDefault("Ice.ProgramName", args[0])
            Application._application = self

            #
            # Used by _destroyOnInterruptCallback and _shutdownOnInterruptCallback.
            #
            Application._nohup = initData.properties.getPropertyAsInt("Ice.Nohup") > 0

            #
            # The default is to destroy when a signal is received.
            #
            if Application._signalPolicy == Application.HandleSignals:
                Application.destroyOnInterrupt()

            status = self.doMain(args, initData)
        except:
            getProcessLogger().error(traceback.format_exc())
            status = 1
        #
        # Set _ctrlCHandler to 0 only once communicator.destroy() has
        # completed.
        #
        Application._ctrlCHandler.destroy()
        Application._ctrlCHandler = None

        return status

    def doMain(self, args, initData):
        try:
            Application._communicator = initialize(args, initData)
            Application._destroyed = False
            status = self.run(args)

        except:
            getProcessLogger().error(traceback.format_exc())
            status = 1

        #
        # Don't want any new interrupt and at this point (post-run),
        # it would not make sense to release a held signal to run
        # shutdown or destroy.
        #
        if Application._signalPolicy == Application.HandleSignals:
            Application.ignoreInterrupt()

        Application._condVar.acquire()
        while Application._callbackInProgress:
            Application._condVar.wait()
        if Application._destroyed:
            Application._communicator = None
        else:
            Application._destroyed = True
            #
            # And _communicator != 0, meaning will be destroyed
            # next, _destroyed = true also ensures that any
            # remaining callback won't do anything
            #
        Application._application = None
        Application._condVar.release()

        if Application._communicator:
            try:
                Application._communicator.destroy()
            except:
                getProcessLogger().error(traceback.format_exc())
                status = 1
            Application._communicator = None
        return status

    def run(self, args):
        '''This method must be overridden in a subclass. The base
class supplies an argument list from which all Ice arguments
have already been removed. The method returns an integer
exit status (0 is success, non-zero is failure).
'''
        raise RuntimeError('run() not implemented')

    def interruptCallback(self, sig):
        '''Subclass hook to intercept an interrupt.'''
        pass

    def appName(self):
        '''Returns the application name (the first element of
the argument list).'''
        return self._appName
    appName = classmethod(appName)

    def communicator(self):
        '''Returns the communicator that was initialized for
the application.'''
        return self._communicator
    communicator = classmethod(communicator)

    def destroyOnInterrupt(self):
        '''Configures the application to destroy its communicator
when interrupted by a signal.'''
        if Application._signalPolicy == Application.HandleSignals:
            self._condVar.acquire()
            if self._ctrlCHandler.getCallback() == self._holdInterruptCallback:
                self._released = True
                self._condVar.notify()
            self._ctrlCHandler.setCallback(self._destroyOnInterruptCallback)
            self._condVar.release()
        else:
            getProcessLogger().error(Application._appName + \
                ": warning: interrupt method called on Application configured to not handle interrupts.")
    destroyOnInterrupt = classmethod(destroyOnInterrupt)

    def shutdownOnInterrupt(self):
        '''Configures the application to shutdown its communicator
when interrupted by a signal.'''
        if Application._signalPolicy == Application.HandleSignals:
            self._condVar.acquire()
            if self._ctrlCHandler.getCallback() == self._holdInterruptCallback:
                self._released = True
                self._condVar.notify()
            self._ctrlCHandler.setCallback(self._shutdownOnInterruptCallback)
            self._condVar.release()
        else:
            getProcessLogger().error(Application._appName + \
                ": warning: interrupt method called on Application configured to not handle interrupts.")
    shutdownOnInterrupt = classmethod(shutdownOnInterrupt)

    def ignoreInterrupt(self):
        '''Configures the application to ignore signals.'''
        if Application._signalPolicy == Application.HandleSignals:
            self._condVar.acquire()
            if self._ctrlCHandler.getCallback() == self._holdInterruptCallback:
                self._released = True
                self._condVar.notify()
            self._ctrlCHandler.setCallback(None)
            self._condVar.release()
        else:
            getProcessLogger().error(Application._appName + \
                ": warning: interrupt method called on Application configured to not handle interrupts.")
    ignoreInterrupt = classmethod(ignoreInterrupt)

    def callbackOnInterrupt(self):
        '''Configures the application to invoke interruptCallback
when interrupted by a signal.'''
        if Application._signalPolicy == Application.HandleSignals:
            self._condVar.acquire()
            if self._ctrlCHandler.getCallback() == self._holdInterruptCallback:
                self._released = True
                self._condVar.notify()
            self._ctrlCHandler.setCallback(self._callbackOnInterruptCallback)
            self._condVar.release()
        else:
            getProcessLogger().error(Application._appName + \
                ": warning: interrupt method called on Application configured to not handle interrupts.")
    callbackOnInterrupt = classmethod(callbackOnInterrupt)

    def holdInterrupt(self):
        '''Configures the application to queue an interrupt for
later processing.'''
        if Application._signalPolicy == Application.HandleSignals:
            self._condVar.acquire()
            if self._ctrlCHandler.getCallback() != self._holdInterruptCallback:
                self._previousCallback = self._ctrlCHandler.getCallback()
                self._released = False
                self._ctrlCHandler.setCallback(self._holdInterruptCallback)
            # else, we were already holding signals
            self._condVar.release()
        else:
            getProcessLogger().error(Application._appName + \
                ": warning: interrupt method called on Application configured to not handle interrupts.")
    holdInterrupt = classmethod(holdInterrupt)

    def releaseInterrupt(self):
        '''Instructs the application to process any queued interrupt.'''
        if Application._signalPolicy == Application.HandleSignals:
            self._condVar.acquire()
            if self._ctrlCHandler.getCallback() == self._holdInterruptCallback:
                #
                # Note that it's very possible no signal is held;
                # in this case the callback is just replaced and
                # setting _released to true and signalling _condVar
                # do no harm.
                #
                self._released = True
                self._ctrlCHandler.setCallback(self._previousCallback)
                self._condVar.notify()
            # Else nothing to release.
            self._condVar.release()
        else:
            getProcessLogger().error(Application._appName + \
                ": warning: interrupt method called on Application configured to not handle interrupts.")
    releaseInterrupt = classmethod(releaseInterrupt)

    def interrupted(self):
        '''Returns True if the application was interrupted by a
signal, or False otherwise.'''
        self._condVar.acquire()
        result = self._interrupted
        self._condVar.release()
        return result
    interrupted = classmethod(interrupted)

    def _holdInterruptCallback(self, sig):
        self._condVar.acquire()
        while not self._released:
            self._condVar.wait()
        if self._destroyed:
            #
            # Being destroyed by main thread
            #
            self._condVar.release()
            return
        callback = self._ctrlCHandler.getCallback()
        self._condVar.release()
        if callback:
            callback(sig)
    _holdInterruptCallback = classmethod(_holdInterruptCallback)

    def _destroyOnInterruptCallback(self, sig):
        self._condVar.acquire()
        if self._destroyed or self._nohup and sig == signal.SIGHUP:
            #
            # Being destroyed by main thread, or nohup.
            #
            self._condVar.release()
            return

        self._callbackInProcess = True
        self._interrupted = True
        self._destroyed = True
        self._condVar.release()

        try:
            self._communicator.destroy()
        except:
            getProcessLogger().error(self._appName + " (while destroying in response to signal " + str(sig) + "):" + \
                                     traceback.format_exc())

        self._condVar.acquire()
        self._callbackInProcess = False
        self._condVar.notify()
        self._condVar.release()
    _destroyOnInterruptCallback = classmethod(_destroyOnInterruptCallback)

    def _shutdownOnInterruptCallback(self, sig):
        self._condVar.acquire()
        if self._destroyed or self._nohup and sig == signal.SIGHUP:
            #
            # Being destroyed by main thread, or nohup.
            #
            self._condVar.release()
            return

        self._callbackInProcess = True
        self._interrupted = True
        self._condVar.release()

        try:
            self._communicator.shutdown()
        except:
            getProcessLogger().error(self._appName + " (while shutting down in response to signal " + str(sig) + \
                                     "):" + traceback.format_exc())

        self._condVar.acquire()
        self._callbackInProcess = False
        self._condVar.notify()
        self._condVar.release()
    _shutdownOnInterruptCallback = classmethod(_shutdownOnInterruptCallback)

    def _callbackOnInterruptCallback(self, sig):
        self._condVar.acquire()
        if self._destroyed:
            #
            # Being destroyed by main thread.
            #
            self._condVar.release()
            return
        # For SIGHUP the user callback is always called. It can decide
        # what to do.

        self._callbackInProcess = True
        self._interrupted = True
        self._condVar.release()

        try:
            self._application.interruptCallback(sig)
        except:
            getProcessLogger().error(self._appName + " (while interrupting in response to signal " + str(sig) + \
                                     "):" + traceback.format_exc())

        self._condVar.acquire()
        self._callbackInProcess = False
        self._condVar.notify()
        self._condVar.release()

    _callbackOnInterruptCallback = classmethod(_callbackOnInterruptCallback)

    HandleSignals = 0
    NoSignalHandling = 1

    _appName = None
    _communicator = None
    _application = None
    _ctrlCHandler = None
    _previousCallback = None
    _interrupted = False
    _released = False
    _destroyed = False
    _callbackInProgress = False
    _condVar = threading.Condition()
    _signalPolicy = HandleSignals

#
# Define Ice::Object and Ice::ObjectPrx.
#
IcePy._t_Object = IcePy.defineClass('::Ice::Object', Object, -1, (), False, False, None, (), ())
IcePy._t_ObjectPrx = IcePy.defineProxy('::Ice::Object', ObjectPrx)
Object._ice_type = IcePy._t_Object

Object._op_ice_isA = IcePy.Operation('ice_isA', OperationMode.Idempotent, OperationMode.Nonmutating, False, None, (), (((), IcePy._t_string, False, 0),), (), ((), IcePy._t_bool, False, 0), ())
Object._op_ice_ping = IcePy.Operation('ice_ping', OperationMode.Idempotent, OperationMode.Nonmutating, False, None, (), (), (), None, ())
Object._op_ice_ids = IcePy.Operation('ice_ids', OperationMode.Idempotent, OperationMode.Nonmutating, False, None, (), (), (), ((), _t_StringSeq, False, 0), ())
Object._op_ice_id = IcePy.Operation('ice_id', OperationMode.Idempotent, OperationMode.Nonmutating, False, None, (), (), (), ((), IcePy._t_string, False, 0), ())

IcePy._t_LocalObject = IcePy.defineClass('::Ice::LocalObject', object, -1, (), False, False, None, (), ())

IcePy._t_UnknownSlicedObject = IcePy.defineClass('::Ice::UnknownSlicedObject', UnknownSlicedObject, -1, (), False, True, None, (), ())
UnknownSlicedObject._ice_type = IcePy._t_UnknownSlicedObject

#
# Annotate some exceptions.
#
def SyscallException__str__(self):
    return "Ice.SyscallException:\n" + os.strerror(self.error)
SyscallException.__str__ = SyscallException__str__
del SyscallException__str__

def SocketException__str__(self):
    return "Ice.SocketException:\n" + os.strerror(self.error)
SocketException.__str__ = SocketException__str__
del SocketException__str__

def ConnectFailedException__str__(self):
    return "Ice.ConnectFailedException:\n" + os.strerror(self.error)
ConnectFailedException.__str__ = ConnectFailedException__str__
del ConnectFailedException__str__

def ConnectionRefusedException__str__(self):
    return "Ice.ConnectionRefusedException:\n" + os.strerror(self.error)
ConnectionRefusedException.__str__ = ConnectionRefusedException__str__
del ConnectionRefusedException__str__

def ConnectionLostException__str__(self):
    if self.error == 0:
        return "Ice.ConnectionLostException:\nrecv() returned zero"
    else:
        return "Ice.ConnectionLostException:\n" + os.strerror(self.error)
ConnectionLostException.__str__ = ConnectionLostException__str__
del ConnectionLostException__str__

#
# Proxy comparison functions.
#
def proxyIdentityEqual(lhs, rhs):
    '''Determines whether the identities of two proxies are equal.'''
    return proxyIdentityCompare(lhs, rhs) == 0

def proxyIdentityCompare(lhs, rhs):
    '''Compares the identities of two proxies.'''
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError('argument is not a proxy')
    if not lhs and not rhs:
        return 0
    elif not lhs and rhs:
        return -1
    elif lhs and not rhs:
        return 1
    else:
        lid = lhs.ice_getIdentity()
        rid = rhs.ice_getIdentity()
        return (lid > rid) - (lid < rid)

def proxyIdentityAndFacetEqual(lhs, rhs):
    '''Determines whether the identities and facets of two
proxies are equal.'''
    return proxyIdentityAndFacetCompare(lhs, rhs) == 0

def proxyIdentityAndFacetCompare(lhs, rhs):
    '''Compares the identities and facets of two proxies.'''
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError('argument is not a proxy')
    if not lhs and not rhs:
        return 0
    elif not lhs and rhs:
        return -1
    elif lhs and not rhs:
        return 1
    elif lhs.ice_getIdentity() != rhs.ice_getIdentity():
        lid = lhs.ice_getIdentity()
        rid = rhs.ice_getIdentity()
        return (lid > rid) - (lid < rid)
    else:
        lf = lhs.ice_getFacet()
        rf = rhs.ice_getFacet()
        return (lf > rf) - (lf < rf)

#
# Used by generated code. Defining these in the Ice module means the generated code
# can avoid the need to qualify the type() and hash() functions with their module
# names. Since the functions are in the __builtin__ module (for Python 2.x) and the
# builtins module (for Python 3.x), it's easier to define them here.
#
def getType(o):
    return type(o)

#
# Used by generated code. Defining this in the Ice module means the generated code
# can avoid the need to qualify the hash() function with its module name. Since
# the function is in the __builtin__ module (for Python 2.x) and the builtins
# module (for Python 3.x), it's easier to define it here.
#
def getHash(o):
    return hash(o)

Protocol_1_0 = ProtocolVersion(1, 0)
Encoding_1_0 = EncodingVersion(1, 0)
Encoding_1_1 = EncodingVersion(1, 1)
