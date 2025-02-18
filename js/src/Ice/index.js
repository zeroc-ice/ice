import * as Ice_Address from "./Address.js";
import * as Ice_ArrayUtil from "./ArrayUtil.js";
import * as Ice_AsyncResult from "./AsyncResult.js";
import * as Ice_AsyncStatus from "./AsyncStatus.js";
import * as Ice_Buffer from "./Buffer.js";
import * as Ice_BuiltinSequences from "./BuiltinSequences.js";
import * as Ice_Communicator from "./Communicator.js";
// Must be imported after Communicator.js for the prototype extensions.
import "./CommunicatorExtensions.js";
import * as Ice_Connection from "./Connection.js";
import * as Ice_Context from "./Context.js";
import * as Ice_Current from "./Current.js";
// Must be imported after Current.js for the prototype extensions.
import "./CurrentExtensions.js";
import * as Ice_DefaultsAndOverrides from "./DefaultsAndOverrides.js";
import * as Ice_Endpoint from "./Endpoint.js";
import * as Ice_EndpointSelectionType from "./EndpointSelectionType.js";
import * as Ice_EndpointTypes from "./EndpointTypes.js";
import * as Ice_EnumBase from "./EnumBase.js";
import * as Ice_Exception from "./Exception.js";
import * as Ice_FormatType from "./FormatType.js";
import * as Ice_HashMap from "./HashMap.js";
import * as Ice_HashUtil from "./HashUtil.js";
import * as Ice_Identity from "./Identity.js";
import * as Ice_IncomingRequest from "./IncomingRequest.js";
// Must be imported after Identity.js for the prototype extensions.
import "./IdentityExtensions.js";
import * as Ice_StringToIdentity from "./StringToIdentity.js";
import * as Ice_IdentityToString from "./IdentityToString.js";
import * as Ice_ImplicitContext from "./ImplicitContext.js";
import * as Ice_Initialize from "./Initialize.js";
import * as Ice_InputStream from "./InputStream.js";
import * as Ice_Instance from "./Instance.js";
// Must be imported after Instance.js for the prototype extensions.
import "./InstanceExtensions.js";
import * as Ice_LocalException from "./LocalException.js";
import * as Ice_LocalExceptions from "./LocalExceptions.js";
import * as Ice_Locator from "./Locator.js";
import * as Ice_LocatorInfo from "./LocatorInfo.js";
import * as Ice_Logger from "./Logger.js";
import * as Ice_Long from "./Long.js";
import * as Ice_MapUtil from "./MapUtil.js";
import * as Ice_Object from "./Object.js";
// Must be imported after Object.js for the prototype extensions.
import "./ObjectExtensions.js";
import * as Ice_ObjectAdapter from "./ObjectAdapter.js";
import * as Ice_ObjectPrx from "./ObjectPrx.js";
// Must be imported after ObjectPrx.js for the prototype extensions.
import "./ObjectPrxExtensions.js";
import * as Ice_Operation from "./Operation.js";
import * as Ice_OperationMode from "./OperationMode.js";
import * as Ice_OptionalFormat from "./OptionalFormat.js";
import * as Ice_OutgoingAsync from "./OutgoingAsync.js";
import * as Ice_OutgoingResponse from "./OutgoingResponse.js";
import * as Ice_OutputStream from "./OutputStream.js";
import * as Ice_Process from "./Process.js";
import * as Ice_ProcessLogger from "./ProcessLogger.js";
import * as Ice_Promise from "./Promise.js";
import * as Ice_Properties from "./Properties.js";
import * as Ice_PropertiesAdmin from "./PropertiesAdmin.js";
import * as Ice_Property from "./Property.js";
import * as Ice_PropertyDict from "./PropertyDict.js";
import * as Ice_PropertyNames from "./PropertyNames.js";
import * as Ice_Protocol from "./Protocol.js";
import * as Ice_ProtocolInstance from "./ProtocolInstance.js";
import * as Ice_Reference from "./Reference.js";
import * as Ice_ReferenceMode from "./ReferenceMode.js";
import * as Ice_RemoteLogger from "./RemoteLogger.js";
import * as Ice_ReplyStatus from "./ReplyStatus.js";
import * as Ice_Router from "./Router.js";
import * as Ice_RouterInfo from "./RouterInfo.js";
import * as Ice_StreamHelpers from "./StreamHelpers.js";
import * as Ice_Struct from "./Struct.js";
import * as Ice_Timer from "./Timer.js";
import * as Ice_ToStringMode from "./ToStringMode.js";
import * as Ice_TraceLevels from "./TraceLevels.js";
import * as Ice_TraceUtil from "./TraceUtil.js";
import * as Ice_UnknownSlicedValue from "./UnknownSlicedValue.js";
import * as Ice_UserException from "./UserException.js";
import * as Ice_Value from "./Value.js";
import * as Ice_ValueFactoryManager from "./ValueFactoryManager.js";
import * as Ice_Version from "./Version.js";
// Must be imported after Version.js for the prototype extensions.
import "./VersionExtensions.js";

import * as Ice_SSL_ConnectionInfo from "./SSL/ConnectionInfo.js";
import * as Ice_SSL_EndpointInfo from "./SSL/EndpointInfo.js";

export const Ice = {
    ...Ice_Address,
    ...Ice_ArrayUtil,
    ...Ice_AsyncResult,
    ...Ice_AsyncStatus,
    ...Ice_Buffer,
    ...Ice_Communicator,
    ...Ice_Connection,
    ...Ice_Current,
    ...Ice_DefaultsAndOverrides,
    ...Ice_Endpoint,
    ...Ice_EndpointSelectionType,
    ...Ice_EnumBase,
    ...Ice_Exception,
    ...Ice_FormatType,
    ...Ice_HashMap,
    ...Ice_HashUtil,
    ...Ice_StringToIdentity,
    ...Ice_IdentityToString,
    ...Ice_ImplicitContext,
    ...Ice_Initialize,
    ...Ice_InputStream,
    ...Ice_Instance,
    ...Ice_IncomingRequest,
    ...Ice_LocalException,
    ...Ice_LocalExceptions,
    ...Ice_LocatorInfo,
    ...Ice_Logger,
    ...Ice_Long,
    ...Ice_MapUtil,
    ...Ice_Object,
    ...Ice_ObjectAdapter,
    ...Ice_ObjectPrx,
    ...Ice_Operation,
    ...Ice_OptionalFormat,
    ...Ice_OutgoingAsync,
    ...Ice_OutgoingResponse,
    ...Ice_OutputStream,
    ...Ice_ProcessLogger,
    ...Ice_Promise,
    ...Ice_Properties,
    ...Ice_Property,
    ...Ice_PropertyNames,
    ...Ice_Protocol,
    ...Ice_ProtocolInstance,
    ...Ice_Reference,
    ...Ice_ReferenceMode,
    ...Ice_RouterInfo,
    ...Ice_StreamHelpers,
    ...Ice_Struct,
    ...Ice_Timer,
    ...Ice_ToStringMode,
    ...Ice_TraceLevels,
    ...Ice_TraceUtil,
    ...Ice_UnknownSlicedValue,
    ...Ice_UserException,
    ...Ice_Value,
    ...Ice_ValueFactoryManager,

    ...Ice_BuiltinSequences.Ice,
    ...Ice_Context.Ice,
    ...Ice_Router.Ice,
    ...Ice_Locator.Ice,
    ...Ice_EndpointTypes.Ice,
    ...Ice_Identity.Ice,
    ...Ice_OperationMode.Ice,
    ...Ice_Process.Ice,
    ...Ice_PropertiesAdmin.Ice,
    ...Ice_PropertyDict.Ice,
    ...Ice_RemoteLogger.Ice,
    ...Ice_ReplyStatus.Ice,
    ...Ice_Version.Ice,
};

const SSL = {
    ...Ice_SSL_ConnectionInfo,
    ...Ice_SSL_EndpointInfo,
};

Ice.SSL = SSL;
