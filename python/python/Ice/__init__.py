#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# ruff: noqa: F401, F821, E402

"""
Ice module
"""

import array
import atexit
import IcePy

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
from .FormatType import FormatType
from .ModuleUtil import *

#
# This value is used as the default value for struct types in the constructors
# of user-defined types. It allows us to determine whether the application has
# supplied a value. (See bug 3676)
#
# TODO: can we use None instead?
_struct_marker = object()


#
# Native PropertiesAdmin admin facet.
#
NativePropertiesAdmin = IcePy.NativePropertiesAdmin


from .PropertiesAdminUpdateCallback import PropertiesAdminUpdateCallback
from .Util import *
from .UnknownSlicedValue import UnknownSlicedValue, SlicedData, SliceInfo


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
import Ice.OperationMode_ice
from .Current import Current

from .Communicator import Communicator

from .ImplicitContext import ImplicitContext
import Ice.Endpoint
from .EndpointSelectionType import EndpointSelectionType


import Ice.EndpointTypes_ice
import Ice.Identity_ice
import Ice.Locator_ice
from .ObjectAdapter import ObjectAdapter
from .ValueFactory import ValueFactory, ValueFactoryManager
import Ice.Process_ice
import Ice.PropertiesAdmin_ice
import Ice.RemoteLogger_ice
import Ice.Router_ice
from .ServantLocator import ServantLocator
import Ice.Connection
import Ice.Version_ice
import Ice.Instrumentation
import Ice.Metrics_ice

from .InitializationData import InitializationData

from .Properties import Properties
from .PropertiesI import PropertiesI
from .ObjectAdapterI import ObjectAdapterI
from .ImplicitContextI import ImplicitContextI
from .Logger import Logger
from .LoggerI import LoggerI
from .BatchRequestInterceptor import BatchRequestInterceptor

# Local exceptions
from .LocalExceptions import *

#
# Replace EndpointInfo with our implementation.
#
EndpointInfo = IcePy.EndpointInfo
IPEndpointInfo = IcePy.IPEndpointInfo
TCPEndpointInfo = IcePy.TCPEndpointInfo
UDPEndpointInfo = IcePy.UDPEndpointInfo
WSEndpointInfo = IcePy.WSEndpointInfo
OpaqueEndpointInfo = IcePy.OpaqueEndpointInfo

SSLEndpointInfo = IcePy.SSLEndpointInfo

#
# Replace ConnectionInfo with our implementation.
#
ConnectionInfo = IcePy.ConnectionInfo
IPConnectionInfo = IcePy.IPConnectionInfo
TCPConnectionInfo = IcePy.TCPConnectionInfo
UDPConnectionInfo = IcePy.UDPConnectionInfo
WSConnectionInfo = IcePy.WSConnectionInfo

SSLConnectionInfo = IcePy.SSLConnectionInfo


from .Proxy import *

#
# Define Ice::Value and Ice::ObjectPrx.
#
IcePy._t_Object = IcePy.defineClass("::Ice::Object", Object, (), None, ())
IcePy._t_Value = IcePy.defineValue("::Ice::Object", Value, -1, (), False, None, ())
IcePy._t_ObjectPrx = IcePy.defineProxy("::Ice::Object", ObjectPrx)
Object._ice_type = IcePy._t_Object

Object._op_ice_isA = IcePy.Operation(
    "ice_isA",
    Ice.OperationMode.Idempotent,
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
    Ice.OperationMode.Idempotent,
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
    Ice.OperationMode.Idempotent,
    False,
    None,
    (),
    (),
    (),
    ((), Ice._t_StringSeq, False, 0),
    (),
)
Object._op_ice_id = IcePy.Operation(
    "ice_id",
    Ice.OperationMode.Idempotent,
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


Protocol_1_0 = Ice.ProtocolVersion(1, 0)
Encoding_1_0 = Ice.EncodingVersion(1, 0)
Encoding_1_1 = Ice.EncodingVersion(1, 1)


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
