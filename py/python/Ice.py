#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

"""
Ice module
"""

import sys, exceptions, string, imp, os, threading, warnings

try:
    import dl
    #
    # This is necessary for proper operation of Ice plug-ins.
    # Without it, RTTI problems can occur.
    #
    sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)
except ImportError:
    # If the dl module is not available and we're running on a linux
    # platform, use the hard coded value of RTLD_NOW|RTLD_GLOBAL.
    if sys.platform.startswith("linux"):
        sys.setdlopenflags(258)
    pass

#
# Import the Python extension.
#
import IcePy

#
# Add some symbols to the Ice module.
#
ObjectPrx = IcePy.ObjectPrx
identityToString = IcePy.identityToString
stringToIdentity = IcePy.stringToIdentity
generateUUID = IcePy.generateUUID
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
# Replace Endpoint with our implementation.
#
del Endpoint
Endpoint =  IcePy.Endpoint


class ThreadNotification(object):
    def __init__(self):
        pass

    #
    # Operation signatures
    #
    # def start():
    # def stop():

#
# Initialization data.
#
class InitializationData(object):
    def __init__(self):
        self.properties = None
        self.logger = None
        #self.stats = None # Stats not currently supported in Python.
        self.threadHook = None

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
        while not self._impl.waitForShutdown(1000):
            pass

    def isShutdown(self):
        self._impl.isShutdown()

    def stringToProxy(self, str):
        return self._impl.stringToProxy(str)

    def proxyToString(self, obj):
        return self._impl.proxyToString(obj)

    def propertyToProxy(self, str):
        return self._impl.propertyToProxy(str)

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

    def setDefaultContext(self, ctx):
        return self._impl.setDefaultContext(ctx)

    def getDefaultContext(self):
        return self._impl.getDefaultContext()

    def getImplicitContext(self):
        return ImplicitContextI(self._impl.getImplicitContext())

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

#
# Ice.initialize()
#
def initialize(args=None, data=None):
    communicator = IcePy.Communicator(args, data)
    return CommunicatorI(communicator)

#
# Ice.initializeWithProperties()
#
def initializeWithProperties(args, properties):
    warnings.warn("initializeWithProperties has been deprecated, use initialize instead", DeprecationWarning, 2)
    data = InitializationData()
    data.properties = properties
    return initialize(args, data)

#
# Ice.initializeWithLogger()
#
def initializeWithLogger(args, logger):
    warnings.warn("initializeWithLogger has been deprecated, use initialize instead", DeprecationWarning, 2)
    data = InitializationData()
    data.logger = logger
    return initialize(args, data)

#
# Ice.initializeWithPropertiesAndLogger()
#
def initializeWithPropertiesAndLogger(args, properties, logger):
    warnings.warn("initializeWithPropertiesAndLogger has been deprecated, use initialize instead",
                  DeprecationWarning, 2)
    data = InitializationData()
    data.properties = properties
    data.logger = logger
    return initialize(args, data)

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

    def createIndirectProxy(self, id):
        return self._impl.createIndirectProxy(id)

    def createReverseProxy(self, id):
        return self._impl.createReverseProxy(id)

    def setLocator(self, loc):
        self._impl.setLocator(loc)

    def refreshPublishedEndpoints(self):
        self._impl.refreshPublishedEndpoints()

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
def createProperties(args=[], defaults=None):
    properties = IcePy.createProperties(args, defaults)
    return PropertiesI(properties)

#
# Ice.getProcessLogger()
# Ice.setProcessLogger()
#
def getProcessLogger():
    logger = IcePy.getProcessLogger()
    if isinstance(logger, Logger):
        return logger
    else:
        return LoggerI(logger)

def setProcessLogger(logger):
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
        if signal.__dict__.has_key('SIGHUP'):
            signal.signal(signal.SIGHUP, CtrlCHandler.signalHandler)
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
        if signal.__dict__.has_key('SIGHUP'):
            signal.signal(signal.SIGHUP, signal.SIG_DFL)
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        self._self = None

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
# Application class.
#
import signal, traceback
class Application(object):

    def __init__(self):
        if type(self) == Application:
            raise RuntimeError("Ice.Application is an abstract class")

    def main(self, args, configFile=None, initData=None):
        if Application._communicator:
            print args[0] + ": only one instance of the Application class can be used"
            return 1

        #
        # Install our handler for the signals we are interested in. We assume main()
        # is called from the main thread.
        #
        Application._ctrlCHandler = CtrlCHandler()

        try:
            status = 0

            Application._interrupted = False
            Application._appName = args[0]

            if not initData:
                initData = InitializationData()
            if configFile:
                initData.properties = createProperties()
                initData.properties.load(configFile)
            Application._application = self
            Application._communicator = initialize(args, initData)
            Application._destroyed = False

            #
            # Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
            #
            Application._nohup = Application._communicator.getProperties().getPropertyAsInt("Ice.Nohup") > 0

            #
            # The default is to destroy when a signal is received.
            #
            Application.destroyOnInterrupt()

            status = self.run(args)
        except:
            traceback.print_exc()
            status = 1

        #
        # Don't want any new interrupt and at this point (post-run),
        # it would not make sense to release a held signal to run
        # shutdown or destroy.
        #
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
                traceback.print_exc()
                status = 1

            Application._communicator = None

        #
        # Set _ctrlCHandler to 0 only once communicator.destroy() has
        # completed.
        # 
        Application._ctrlCHandler.destroy()
        Application._ctrlCHandler = None

        return status

    def run(self, args):
        raise RuntimeError('run() not implemented')

    def interruptCallback(self, sig):
        pass

    def appName(self):
        return self._appName
    appName = classmethod(appName)

    def communicator(self):
        return self._communicator
    communicator = classmethod(communicator)

    def destroyOnInterrupt(self):
        self._condVar.acquire()
        if self._ctrlCHandler.getCallback() == self.holdInterruptCallback:
            self._released = True
            self._condVar.notify()
        self._ctrlCHandler.setCallback(self.destroyOnInterruptCallback)
        self._condVar.release()
    destroyOnInterrupt = classmethod(destroyOnInterrupt)

    def shutdownOnInterrupt(self):
        self._condVar.acquire()
        if self._ctrlCHandler.getCallback() == self.holdInterruptCallback:
            self._released = True
            self._condVar.notify()
        self._ctrlCHandler.setCallback(self.shutdownOnInterruptCallback)
        self._condVar.release()
    shutdownOnInterrupt = classmethod(shutdownOnInterrupt)

    def ignoreInterrupt(self):
        self._condVar.acquire()
        if self._ctrlCHandler.getCallback() == self.holdInterruptCallback:
            self._released = True
            self._condVar.notify()
        self._ctrlCHandler.setCallback(None)
        self._condVar.release()
    ignoreInterrupt = classmethod(ignoreInterrupt)

    def callbackOnInterrupt(self):
        self._condVar.acquire()
        if self._ctrlCHandler.getCallback() == self.holdInterruptCallback:
            self._released = True
            self._condVar.notify()
        self._ctrlCHandler.setCallback(self.callbackOnInterruptCallback)
        self._condVar.release()
    callbackOnInterrupt = classmethod(callbackOnInterrupt)

    def holdInterrupt(self):
        self._condVar.acquire()
        if self._ctrlCHandler.getCallback() != self.holdInterruptCallback:
            self._previousCallback = self._ctrlCHandler.getCallback()
            self._released = False
            self._ctrlCHandler.setCallback(self.holdInterruptCallback)
        # else, we were already holding signals
        self._condVar.release()
    holdInterrupt = classmethod(holdInterrupt)

    def releaseInterrupt(self):
        self._condVar.acquire()
        if self._ctrlCHandler.getCallback() == self.holdInterruptCallback:
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
    releaseInterrupt = classmethod(releaseInterrupt)

    def interrupted(self):
        self._condVar.acquire()
        result = self._interrupted
        self._condVar.release()
        return result
    interrupted = classmethod(interrupted)

    def holdInterruptCallback(self, sig):
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
    holdInterruptCallback = classmethod(holdInterruptCallback)

    def destroyOnInterruptCallback(self, sig):
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
            print self._appName + " (while destroying in response to signal " + str(sig) + "):"
            traceback.print_exc()

        self._condVar.acquire()
        self._callbackInProcess = False
        self._condVar.notify()
        self._condVar.release()
    destroyOnInterruptCallback = classmethod(destroyOnInterruptCallback)

    def shutdownOnInterruptCallback(self, sig):
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
            print self._appName + " (while shutting down in response to signal " + str(sig) + "):"
            traceback.print_exc()

        self._condVar.acquire()
        self._callbackInProcess = False
        self._condVar.notify()
        self._condVar.release()
    shutdownOnInterruptCallback = classmethod(shutdownOnInterruptCallback)

    def callbackOnInterruptCallback(self, sig):
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
            print self._appName + " (while interrupting in response to signal " + str(sig) + "):"
            traceback.print_exc()

        self._condVar.acquire()
        self._callbackInProcess = False
        self._condVar.notify()
        self._condVar.release()

    callbackOnInterruptCallback = classmethod(callbackOnInterruptCallback)

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

#
# Define Ice::Object and Ice::ObjectPrx.
#
IcePy._t_Object = IcePy.defineClass('::Ice::Object', Object, (), False, None, (), ())
IcePy._t_ObjectPrx = IcePy.defineProxy('::Ice::Object', ObjectPrx)
Object.ice_type = IcePy._t_Object

Object._op_ice_isA = IcePy.Operation('ice_isA', OperationMode.Idempotent, OperationMode.Nonmutating, False, (), (((), IcePy._t_string),), (), IcePy._t_bool, ())
Object._op_ice_ping = IcePy.Operation('ice_ping', OperationMode.Idempotent, OperationMode.Nonmutating, False, (), (), (), None, ())
Object._op_ice_ids = IcePy.Operation('ice_ids', OperationMode.Idempotent, OperationMode.Nonmutating, False, (), (), (), _t_StringSeq, ())
Object._op_ice_id = IcePy.Operation('ice_id', OperationMode.Idempotent, OperationMode.Nonmutating, False, (), (), (), IcePy._t_string, ())

IcePy._t_LocalObject = IcePy.defineClass('::Ice::LocalObject', LocalObject, (), False, None, (), ())
LocalObject.ice_type = IcePy._t_LocalObject

#
# Annotate Ice::Identity.
#
def Identity__str__(self):
    return IcePy.identityToString(self)
Identity.__str__ = Identity__str__
del Identity__str__

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
