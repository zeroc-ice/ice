# Copyright (c) ZeroC, Inc.

# ruff: noqa: F401  # Unused imports
# ruff: noqa: F821  # Undefined names
# ruff: noqa: E402  # Module level import not at top of file
# ruff: noqa: F403  # 'from module import *' used
# ruff: noqa: F405  # Name may be undefined from wildcard import
# ruff: noqa: I001  # Import block is unsorted or unformatted

"""
Ice module
"""

import IcePy
from .ModuleUtil import *

#
# Add some symbols to the Ice module.
#
stringVersion = IcePy.stringVersion
intVersion = IcePy.intVersion
currentProtocol = IcePy.currentProtocol
currentProtocolEncoding = IcePy.currentProtocolEncoding
currentEncoding = IcePy.currentEncoding
stringToProtocolVersion = IcePy.stringToProtocolVersion
protocolVersionToString = IcePy.protocolVersionToString
stringToEncodingVersion = IcePy.stringToEncodingVersion
encodingVersionToString = IcePy.encodingVersionToString
loadSlice = IcePy.loadSlice
AsyncInvocationContext = IcePy.AsyncInvocationContext

#
# Forward declarations.
#
IcePy._t_Value = IcePy.declareValue("::Ice::Object")
IcePy._t_ObjectPrx = IcePy.declareProxy("::Ice::Object")

#
# Import local definitions that are part of the Ice module public API.
#
from .EventLoopAdapter import *
from .Current import *
from .Future import *
from .InvocationFuture import *
from .Value import *
from .Object import Object
from .ObjectPrx import ObjectPrx, checkedCast, checkedCastAsync, uncheckedCast
from .Blobject import Blobject
from .FormatType import *
from .SlicedData import SlicedData
from .SliceInfo import SliceInfo
from .UnknownSlicedValue import *
from .ToStringMode import *
from .Exception import *
from .LocalException import *
from .UserException import *
from .Communicator import *
from .ImplicitContext import *
from .EndpointSelectionType import *
from .ObjectAdapter import *
from .CompressBatch import CompressBatch
from .ServantLocator import *
from .InitializationData import *
from .Properties import Properties
from .Logger import *
from .LocalExceptions import *
from .Proxy import *
from .Current import Current
from .Dispatch import *

#
# Import the generated code for the Ice module.
#
import Ice.BuiltinSequences_ice
import Ice.OperationMode_ice
import Ice.EndpointTypes_ice
import Ice.Identity_ice
import Ice.Locator_ice
import Ice.LocatorRegistry_ice
import Ice.Process_ice
import Ice.PropertiesAdmin_ice
import Ice.RemoteLogger_ice
import Ice.Router_ice
import Ice.Version_ice
import Ice.Metrics_ice

from .Util import *

#
# Add EndpointInfo alias in Ice module.
#
EndpointInfo = IcePy.EndpointInfo
IPEndpointInfo = IcePy.IPEndpointInfo
TCPEndpointInfo = IcePy.TCPEndpointInfo
UDPEndpointInfo = IcePy.UDPEndpointInfo
WSEndpointInfo = IcePy.WSEndpointInfo
OpaqueEndpointInfo = IcePy.OpaqueEndpointInfo
SSLEndpointInfo = IcePy.SSLEndpointInfo

#
# Add ConnectionInfo alias in Ice module.
#
ConnectionInfo = IcePy.ConnectionInfo
IPConnectionInfo = IcePy.IPConnectionInfo
TCPConnectionInfo = IcePy.TCPConnectionInfo
UDPConnectionInfo = IcePy.UDPConnectionInfo
WSConnectionInfo = IcePy.WSConnectionInfo
SSLConnectionInfo = IcePy.SSLConnectionInfo

#
# Protocol and Encoding constants
#
Protocol_1_0 = Ice.ProtocolVersion(1, 0)
Encoding_1_0 = Ice.EncodingVersion(1, 0)
Encoding_1_1 = Ice.EncodingVersion(1, 1)

#
# Native PropertiesAdmin admin facet.
#
NativePropertiesAdmin = IcePy.NativePropertiesAdmin

# __all__ defines the public symbols of the Ice module.
# It controls what is imported when using `from Ice import *`.
# Sphinx uses this list when generating API documentation via the `automodule` directive.
__all__ = [
    "AdapterAlreadyActiveException",
    "AdapterNotFoundException",
    "AlreadyRegisteredException",
    "Blobject",
    "CloseConnectionException",
    "CloseTimeoutException",
    "Communicator",
    "CommunicatorDestroyedException",
    "CompressBatch",
    "ConnectFailedException",
    "ConnectTimeoutException",
    "ConnectionAbortedException",
    "ConnectionClosedException",
    "ConnectionLostException",
    "ConnectionRefusedException",
    "Current",
    "DNSException",
    "DatagramLimitException",
    "DispatchException",
    "EncodingVersion",
    "EndpointSelectionType",
    "EventLoopAdapter",
    "Exception",
    "FacetNotExistException",
    "FeatureNotSupportedException",
    "FixedProxyException",
    "FormatType",
    "Future",
    "FutureBase",
    "Identity",
    "ImplicitContext",
    "InitializationData",
    "InitializationException",
    "InvalidReplicaGroupIdException",
    "InvocationCanceledException",
    "InvocationFuture",
    "InvocationTimeoutException",
    "LocalException",
    "Locator",
    "LocatorFinder",
    "LocatorFinderPrx",
    "LocatorPrx",
    "LocatorRegistry",
    "LocatorRegistryPrx",
    "LogMessage",
    "LogMessageType",
    "Logger",
    "LoggerAdmin",
    "LoggerAdminPrx",
    "MarshalException",
    "NoEndpointException",
    "NotRegisteredException",
    "Object",
    "ObjectAdapter",
    "ObjectAdapterDeactivatedException",
    "ObjectAdapterDestroyedException",
    "ObjectAdapterIdInUseException",
    "ObjectNotExistException",
    "ObjectNotFoundException",
    "ObjectPrx",
    "OperationMode",
    "OperationNotExistException",
    "ParseException",
    "Process",
    "ProcessPrx",
    "Properties",
    "PropertiesAdmin",
    "PropertiesAdminPrx",
    "PropertyException",
    "ProtocolException",
    "ProtocolVersion",
    "RemoteLogger",
    "RemoteLoggerPrx",
    "ReplyStatus",
    "RequestFailedException",
    "Router",
    "RouterPrx",
    "SecurityException",
    "ServantLocator",
    "ServerNotFoundException",
    "SocketException",
    "SyscallException",
    "TimeoutException",
    "ToStringMode",
    "TwowayOnlyException",
    "UnknownException",
    "UnknownLocalException",
    "UnknownSlicedValue",
    "UnknownUserException",
    "UserException",
    "Value",
    "createProperties",
    "getProcessLogger",
    "getSliceDir",
    "identityToString",
    "initialize",
    "proxyIdentityCompare",
    "proxyIdentityEqual",
    "setProcessLogger",
    "stringToIdentity",
    "wrap_future",
]
