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
from .ObjectPrxF import _Ice_ObjectPrx_t
from .ValueF import _Ice_Value_t

#
# Import local definitions that are part of the Ice module public API.
#
from .StringUtil import format_fields
from .EventLoopAdapter import EventLoopAdapter
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
from .AdapterAlreadyActiveException import AdapterAlreadyActiveException
from .AdapterAlreadyActiveException import _Ice_AdapterAlreadyActiveException_t
from .AdapterNotFoundException import AdapterNotFoundException
from .AdapterNotFoundException import _Ice_AdapterNotFoundException_t
from .BTEndpointType import BTEndpointType
from .BTSEndpointType import BTSEndpointType
from .BoolSeq import _Ice_BoolSeq_t
from .ByteSeq import _Ice_ByteSeq_t
from .Context import _Ice_Context_t
from .DoubleSeq import _Ice_DoubleSeq_t
from .EncodingVersion import EncodingVersion, _Ice_EncodingVersion_t
from .FloatSeq import _Ice_FloatSeq_t
from .Identity import Identity
from .Identity import _Ice_Identity_t
from .IdentitySeq import _Ice_IdentitySeq_t
from .IntSeq import _Ice_IntSeq_t
from .InvalidReplicaGroupIdException import InvalidReplicaGroupIdException
from .InvalidReplicaGroupIdException import _Ice_InvalidReplicaGroupIdException_t
from .Locator import Locator
from .Locator import LocatorPrx
from .LocatorF import _Ice_LocatorPrx_t
from .LocatorFinder import LocatorFinder
from .LocatorFinder import LocatorFinderPrx
from .LocatorFinderF import _Ice_LocatorFinderPrx_t
from .LocatorRegistry import LocatorRegistry
from .LocatorRegistry import LocatorRegistryPrx
from .LocatorRegistryF import _Ice_LocatorRegistryPrx_t
from .LogMessage import LogMessage
from .LogMessage import _Ice_LogMessage_t
from .LogMessageSeq import _Ice_LogMessageSeq_t
from .LogMessageType import LogMessageType
from .LogMessageType import _Ice_LogMessageType_t
from .LogMessageTypeSeq import _Ice_LogMessageTypeSeq_t
from .LoggerAdmin import LoggerAdmin
from .LoggerAdmin import LoggerAdminPrx
from .LoggerAdminF import _Ice_LoggerAdminPrx_t
from .LongSeq import _Ice_LongSeq_t
from .ObjectNotFoundException import ObjectNotFoundException
from .ObjectNotFoundException import _Ice_ObjectNotFoundException_t
from .ObjectProxySeq import _Ice_ObjectProxySeq_t
from .ObjectSeq import _Ice_ObjectSeq_t
from .OperationMode import OperationMode
from .OperationMode import _Ice_OperationMode_t
from .Process import Process
from .Process import ProcessPrx
from .ProcessLogger import getProcessLogger, setProcessLogger
from .ProcessF import _Ice_ProcessPrx_t
from .PropertiesAdmin import PropertiesAdmin
from .PropertiesAdmin import PropertiesAdminPrx
from .PropertiesAdminF import _Ice_PropertiesAdminPrx_t
from .PropertyDict import _Ice_PropertyDict_t
from .ProtocolVersion import ProtocolVersion, _Ice_ProtocolVersion_t
from .RemoteLogger import RemoteLogger
from .RemoteLogger import RemoteLoggerPrx
from .RemoteLoggerAlreadyAttachedException import RemoteLoggerAlreadyAttachedException
from .RemoteLoggerAlreadyAttachedException import _Ice_RemoteLoggerAlreadyAttachedException_t
from .RemoteLoggerF import _Ice_RemoteLoggerPrx_t
from .ReplyStatus import ReplyStatus
from .ReplyStatus import _Ice_ReplyStatus_t
from .Router import Router
from .Router import RouterPrx
from .RouterF import _Ice_RouterPrx_t
from .RouterFinder import RouterFinder
from .RouterFinder import RouterFinderPrx
from .RouterFinderF import _Ice_RouterFinderPrx_t
from .SSLEndpointType import SSLEndpointType
from .ServerNotFoundException import ServerNotFoundException
from .ServerNotFoundException import _Ice_ServerNotFoundException_t
from .ShortSeq import _Ice_ShortSeq_t
from .SliceChecksumDict import _Ice_SliceChecksumDict_t
from .StringSeq import _Ice_StringSeq_t
from .TCPEndpointType import TCPEndpointType
from .UDPEndpointType import UDPEndpointType
from .URIEndpointType import URIEndpointType
from .WSEndpointType import WSEndpointType
from .WSSEndpointType import WSSEndpointType
from .iAPEndpointType import iAPEndpointType
from .iAPSEndpointType import iAPSEndpointType

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
Protocol_1_0 = ProtocolVersion(1, 0)
Encoding_1_0 = EncodingVersion(1, 0)
Encoding_1_1 = EncodingVersion(1, 1)

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
    "BTEndpointType",
    "BTSEndpointType",
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
    "RemoteLoggerAlreadyAttachedException",
    "RemoteLoggerPrx",
    "ReplyStatus",
    "RequestFailedException",
    "Router",
    "RouterFinder",
    "RouterFinderPrx",
    "RouterPrx",
    "SSLEndpointType",
    "SecurityException",
    "ServantLocator",
    "ServerNotFoundException",
    "SocketException",
    "SyscallException",
    "TCPEndpointType",
    "TimeoutException",
    "ToStringMode",
    "TwowayOnlyException",
    "UDPEndpointType",
    "URIEndpointType",
    "UnknownException",
    "UnknownLocalException",
    "UnknownSlicedValue",
    "UnknownUserException",
    "UserException",
    "Value",
    "WSEndpointType",
    "WSSEndpointType",
    "_Ice_AdapterAlreadyActiveException_t",
    "_Ice_AdapterNotFoundException_t",
    "_Ice_BoolSeq_t",
    "_Ice_ByteSeq_t",
    "_Ice_Context_t",
    "_Ice_DoubleSeq_t",
    "_Ice_EncodingVersion_t",
    "_Ice_FloatSeq_t",
    "_Ice_IdentitySeq_t",
    "_Ice_Identity_t",
    "_Ice_IntSeq_t",
    "_Ice_InvalidReplicaGroupIdException_t",
    "_Ice_LocatorFinderPrx_t",
    "_Ice_LocatorPrx_t",
    "_Ice_LocatorRegistryPrx_t",
    "_Ice_LogMessageSeq_t",
    "_Ice_LogMessageTypeSeq_t",
    "_Ice_LogMessageType_t",
    "_Ice_LogMessage_t",
    "_Ice_LoggerAdminPrx_t",
    "_Ice_LongSeq_t",
    "_Ice_ObjectNotFoundException_t",
    "_Ice_ObjectProxySeq_t",
    "_Ice_ObjectSeq_t",
    "_Ice_OperationMode_t",
    "_Ice_ProcessPrx_t",
    "_Ice_PropertiesAdminPrx_t",
    "_Ice_PropertyDict_t",
    "_Ice_ProtocolVersion_t",
    "_Ice_RemoteLoggerAlreadyAttachedException_t",
    "_Ice_RemoteLoggerPrx_t",
    "_Ice_ReplyStatus_t",
    "_Ice_RouterFinderPrx_t",
    "_Ice_RouterPrx_t",
    "_Ice_ServerNotFoundException_t",
    "_Ice_ShortSeq_t",
    "_Ice_SliceChecksumDict_t",
    "_Ice_StringSeq_t",
    "createProperties",
    "format_fields",
    "getProcessLogger",
    "getSliceDir",
    "iAPEndpointType",
    "iAPSEndpointType",
    "identityToString",
    "initialize",
    "proxyIdentityCompare",
    "proxyIdentityEqual",
    "setProcessLogger",
    "stringToIdentity",
    "wrap_future",
]
