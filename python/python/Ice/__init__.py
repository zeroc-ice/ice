#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# ruff: noqa: F401, F821

"""
Ice module
"""

import sys
import os
import types
import array
import atexit

#
# RTTI problems can occur in C++ code unless we modify Python's dlopen flags.
# Note that changing these flags might cause problems for other extensions
# loaded by the application (see bug 3660), so we restore the original settings
# after loading IcePy.
#
_dlopenflags = -1
try:
    _dlopenflags = sys.getdlopenflags()
    sys.setdlopenflags(os.RTLD_NOW | os.RTLD_GLOBAL)

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
AsyncInvocationContext = IcePy.AsyncInvocationContext
Unset = IcePy.Unset

from .Future import Future, wrap_future
from .InvocationFuture import InvocationFuture

from .Object import Object
from .Blobject import Blobject
from .BlobjectAsync import BlobjectAsync
from .Value import Value



#
# This value is used as the default value for struct types in the constructors
# of user-defined types. It allows us to determine whether the application has
# supplied a value. (See bug 3676)
#
# TODO: can we use None instead?
_struct_marker = object()

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
    # bytes - string/bytes
    # instances - tuple of Ice.Value
    # hasOptionalMembers - boolean
    # isLastSlice - boolean
    pass


#
# Native PropertiesAdmin admin facet.
#
NativePropertiesAdmin = IcePy.NativePropertiesAdmin


class PropertiesAdminUpdateCallback(object):
    """Callback class to get notifications of property updates passed
    through the Properties admin facet"""

    def updated(self, props):
        pass


class UnknownSlicedValue(Value):
    #
    # Members:
    #
    # unknownTypeId - string

    def ice_id(self):
        return self.unknownTypeId


def getSliceDir():
    """Convenience function for locating the directory containing the Slice files."""

    #
    # Get the parent of the directory containing this file (__init__.py).
    #
    pyHome = os.path.join(os.path.dirname(__file__), "..")

    #
    # Detect setup.py installation in site-packages. The slice
    # files live one level above this file.
    #
    dir = os.path.join(pyHome, "slice")
    if os.path.isdir(dir):
        return dir

    #
    # For an installation from a source distribution, a binary tarball, or a
    # Windows installer, the "slice" directory is a sibling of the "python"
    # directory.
    #
    dir = os.path.join(pyHome, "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    #
    # In a source distribution, the "slice" directory is an extra level higher.
    # directory.
    #
    dir = os.path.join(pyHome, "..", "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    if sys.platform[:5] == "linux":
        #
        # Check the default Linux location.
        #
        dir = os.path.join("/", "usr", "share", "ice", "slice")
        if os.path.exists(dir):
            return dir

    elif sys.platform == "darwin":
        #
        # Check the default macOS homebrew location.
        #
        dir = os.path.join("/", "usr", "local", "share", "ice", "slice")
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
    parts = name.split(".")
    curr = ""
    mod = None

    for s in parts:
        curr = curr + s

        if curr in sys.modules:
            mod = sys.modules[curr]
        elif curr in _pendingModules:
            mod = _pendingModules[curr]
        else:
            nmod = types.ModuleType(curr)
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
    class __temp:
        pass

    return __temp


class FormatType(object):
    def __init__(self, val):
        assert val >= 0 and val < 3
        self.value = val


FormatType.DefaultFormat = FormatType(0)
FormatType.CompactFormat = FormatType(1)
FormatType.SlicedFormat = FormatType(2)

#
# Forward declarations.
#
IcePy._t_Object = IcePy.declareClass("::Ice::Object")
IcePy._t_Value = IcePy.declareValue("::Ice::Object")
IcePy._t_ObjectPrx = IcePy.declareProxy("::Ice::Object")
IcePy._t_LocalObject = IcePy.declareValue("::Ice::LocalObject")

#
# Import "local slice" and generated Ice modules.
#
from .EnumBase import EnumBase

from .ToStringMode import ToStringMode
from .Exception import Exception
from .LocalException import LocalException
from .UserException import UserException

import Ice.BuiltinSequences_ice
import Ice.Current
import Ice.Communicator

import Ice.ImplicitContext
import Ice.Endpoint
from .EndpointSelectionType import EndpointSelectionType


import Ice.EndpointTypes_ice
import Ice.Identity_ice
import Ice.Locator_ice
import Ice.Logger
import Ice.ObjectAdapter
import Ice.ValueFactory
import Ice.Process_ice
import Ice.Properties
import Ice.PropertiesAdmin_ice
import Ice.RemoteLogger_ice
import Ice.Router_ice
from .ServantLocator import ServantLocator
import Ice.Connection
import Ice.Version_ice
import Ice.Instrumentation
import Ice.Metrics_ice

from .InitializationData import InitializationData

from .initialize import initialize
from .PropertiesI import PropertiesI
from .ObjectAdapterI import ObjectAdapterI
from .ImplicitContextI import ImplicitContextI
from .LoggerI import LoggerI


# Local exceptions
from .LocalExceptions import InitializationException
from .LocalExceptions import AlreadyRegisteredException
from .LocalExceptions import NotRegisteredException
from .LocalExceptions import TwowayOnlyException
from .LocalExceptions import UnknownException
from .LocalExceptions import UnknownLocalException
from .LocalExceptions import UnknownUserException
from .LocalExceptions import CommunicatorDestroyedException
from .LocalExceptions import ObjectAdapterDeactivatedException
from .LocalExceptions import ObjectAdapterIdInUseException
from .LocalExceptions import NoEndpointException
from .LocalExceptions import ParseException
from .LocalExceptions import IllegalIdentityException
from .LocalExceptions import IllegalServantException
from .LocalExceptions import RequestFailedException
from .LocalExceptions import ObjectNotExistException
from .LocalExceptions import FacetNotExistException
from .LocalExceptions import OperationNotExistException
from .LocalExceptions import SyscallException
from .LocalExceptions import SocketException
from .LocalExceptions import FileException
from .LocalExceptions import ConnectFailedException
from .LocalExceptions import ConnectionRefusedException
from .LocalExceptions import ConnectionLostException
from .LocalExceptions import DNSException
from .LocalExceptions import ConnectionIdleException
from .LocalExceptions import TimeoutException
from .LocalExceptions import ConnectTimeoutException
from .LocalExceptions import CloseTimeoutException
from .LocalExceptions import InvocationTimeoutException
from .LocalExceptions import InvocationCanceledException
from .LocalExceptions import ProtocolException
from .LocalExceptions import CloseConnectionException
from .LocalExceptions import ConnectionManuallyClosedException
from .LocalExceptions import DatagramLimitException
from .LocalExceptions import MarshalException
from .LocalExceptions import FeatureNotSupportedException
from .LocalExceptions import SecurityException
from .LocalExceptions import FixedProxyException

#
# Replace EndpointInfo with our implementation.
#
del EndpointInfo
EndpointInfo = IcePy.EndpointInfo
del IPEndpointInfo
IPEndpointInfo = IcePy.IPEndpointInfo
del TCPEndpointInfo
TCPEndpointInfo = IcePy.TCPEndpointInfo
del UDPEndpointInfo
UDPEndpointInfo = IcePy.UDPEndpointInfo
del WSEndpointInfo
WSEndpointInfo = IcePy.WSEndpointInfo
del OpaqueEndpointInfo
OpaqueEndpointInfo = IcePy.OpaqueEndpointInfo

SSLEndpointInfo = IcePy.SSLEndpointInfo

#
# Replace ConnectionInfo with our implementation.
#
del ConnectionInfo
ConnectionInfo = IcePy.ConnectionInfo
del IPConnectionInfo
IPConnectionInfo = IcePy.IPConnectionInfo
del TCPConnectionInfo
TCPConnectionInfo = IcePy.TCPConnectionInfo
del UDPConnectionInfo
UDPConnectionInfo = IcePy.UDPConnectionInfo
del WSConnectionInfo
WSConnectionInfo = IcePy.WSConnectionInfo

SSLConnectionInfo = IcePy.SSLConnectionInfo


class BatchRequestInterceptor(object):
    """Base class for batch request interceptor. A subclass must
    define the enqueue method."""

    def __init__(self):
        pass

    def enqueue(self, request, queueCount, queueSize):
        """Invoked when a request is batched."""
        pass


#
# Ice.identityToString
#


def identityToString(id, toStringMode=None):
    return IcePy.identityToString(id, toStringMode)


#
# Ice.stringToIdentity
#


def stringToIdentity(str):
    return IcePy.stringToIdentity(str)


#
# Ice.createProperties()
#


def createProperties(args=None, defaults=None):
    """Creates a new property set. The optional arguments represent
    an argument list (such as sys.argv) and a property set that supplies
    default values. You can invoke this function as follows:

    Ice.createProperties()
    Ice.createProperties(args)
    Ice.createProperties(defaults)
    Ice.createProperties(args, defaults)

    If you supply an argument list, the function removes those arguments
    from the list that were recognized by the Ice run time."""

    properties = IcePy.createProperties(args, defaults)
    return PropertiesI(properties)


#
# Define Ice::Value and Ice::ObjectPrx.
#
IcePy._t_Object = IcePy.defineClass("::Ice::Object", Object, (), None, ())
IcePy._t_Value = IcePy.defineValue("::Ice::Object", Value, -1, (), False, None, ())
IcePy._t_ObjectPrx = IcePy.defineProxy("::Ice::Object", ObjectPrx)
Object._ice_type = IcePy._t_Object

Object._op_ice_isA = IcePy.Operation(
    "ice_isA",
    OperationMode.Idempotent,
    False,
    None,
    (),
    (((), IcePy._t_string, False, 0),),
    (),
    ((), IcePy._t_bool, False, 0),
    (),
)
Object._op_ice_ping = IcePy.Operation(
    "ice_ping",
    OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    None,
    (),
)
Object._op_ice_ids = IcePy.Operation(
    "ice_ids",
    OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    ((), _t_StringSeq, False, 0),
    (),
)
Object._op_ice_id = IcePy.Operation(
    "ice_id",
    OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    ((), IcePy._t_string, False, 0),
    (),
)

IcePy._t_LocalObject = IcePy.defineValue(
    "::Ice::LocalObject", object, -1, (), False, None, ()
)

IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)
UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue

#
# Proxy comparison functions.
#


def proxyIdentityEqual(lhs, rhs):
    """Determines whether the identities of two proxies are equal."""
    return proxyIdentityCompare(lhs, rhs) == 0


def proxyIdentityCompare(lhs, rhs):
    """Compares the identities of two proxies."""
    if (lhs and not isinstance(lhs, ObjectPrx)) or (
        rhs and not isinstance(rhs, ObjectPrx)
    ):
        raise ValueError("argument is not a proxy")
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
    """Determines whether the identities and facets of two
    proxies are equal."""
    return proxyIdentityAndFacetCompare(lhs, rhs) == 0


def proxyIdentityAndFacetCompare(lhs, rhs):
    """Compares the identities and facets of two proxies."""
    if (lhs and not isinstance(lhs, ObjectPrx)) or (
        rhs and not isinstance(rhs, ObjectPrx)
    ):
        raise ValueError("argument is not a proxy")
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


BuiltinBool = 0
BuiltinByte = 1
BuiltinShort = 2
BuiltinInt = 3
BuiltinLong = 4
BuiltinFloat = 5
BuiltinDouble = 6

BuiltinTypes = [
    BuiltinBool,
    BuiltinByte,
    BuiltinShort,
    BuiltinInt,
    BuiltinLong,
    BuiltinFloat,
    BuiltinDouble,
]
BuiltinArrayTypes = ["b", "b", "h", "i", "q", "f", "d"]


def createArray(view, t, copy):
    if t not in BuiltinTypes:
        raise ValueError("`{0}' is not an array builtin type".format(t))
    a = array.array(BuiltinArrayTypes[t])
    a.frombytes(view)
    return a


try:
    import numpy

    BuiltinNumpyTypes = [
        numpy.bool_,
        numpy.int8,
        numpy.int16,
        numpy.int32,
        numpy.int64,
        numpy.float32,
        numpy.float64,
    ]

    def createNumPyArray(view, t, copy):
        if t not in BuiltinTypes:
            raise ValueError("`{0}' is not an array builtin type".format(t))
        return numpy.frombuffer(view.tobytes() if copy else view, BuiltinNumpyTypes[t])

except ImportError:
    pass
