
import * as Ice_ACM from "./ACM.js";
import * as Ice_Address from "./Address.js";
import * as Ice_ArrayUtil from "./ArrayUtil.js";
import * as Ice_AsyncResult from "./AsyncResult.js";
import * as Ice_AsyncResultBase from "./AsyncResultBase.js";
import * as Ice_AsyncStatus from "./AsyncStatus.js";
import * as Ice_Base64 from "./Base64.js";
import * as Ice_BatchRequestQueue from "./BatchRequestQueue.js";
import * as Ice_Buffer from "./Buffer.js";
import * as Ice_BuiltinSequences from "./BuiltinSequences.js";
import * as Ice_Communicator from "./Communicator.js";
// Must be imported after Communicator.js for the prototype extensions.
import "./CommunicatorExtensions.js";
import * as Ice_CompactIdRegistry from "./CompactIdRegistry.js";
import * as Ice_ConnectRequestHandler from "./ConnectRequestHandler.js";
import * as Ice_Connection from "./Connection.js";
import * as Ice_ConnectionI from "./ConnectionI.js";
import * as Ice_ConnectionRequestHandler from "./ConnectionRequestHandler.js";
import * as Ice_Context from "./Context.js";
import * as Ice_Current from "./Current.js";
import * as Ice_DefaultsAndOverrides from "./DefaultsAndOverrides.js";
import * as Ice_Endpoint from "./Endpoint.js";
import * as Ice_EndpointFactoryManager from "./EndpointFactoryManager.js";
import * as Ice_EndpointI from "./EndpointI.js";
import * as Ice_EndpointSelectionType from "./EndpointSelectionType.js";
import * as Ice_EndpointTypes from "./EndpointTypes.js";
import * as Ice_EnumBase from "./EnumBase.js";
import * as Ice_ExUtil from "./ExUtil.js";
import * as Ice_Exception from "./Exception.js";
import * as Ice_FacetMap from "./FacetMap.js";
import * as Ice_FormatType from "./FormatType.js";
import * as Ice_HashMap from "./HashMap.js";
import * as Ice_HashUtil from "./HashUtil.js";
import * as Ice_IPEndpointI from "./IPEndpointI.js";
import * as Ice_Identity from "./Identity.js";
import * as Ice_IdentityUtil from "./IdentityUtil.js";
import * as Ice_ImplicitContext from "./ImplicitContext.js";
import * as Ice_IncomingAsync from "./IncomingAsync.js";
import * as Ice_Initialize from "./Initialize.js";
import * as Ice_Instance from  "./Instance.js";
// Must be imported after Instance.js for the prototype extensions.
import "./InstanceExtensions.js";
import * as Ice_LocalException from "./LocalException.js";
import * as Ice_Locator from "./Locator.js";
import * as Ice_LocatorInfo from "./LocatorInfo.js";
import * as Ice_LocatorManager from "./LocatorManager.js";
import * as Ice_LocatorTable from "./LocatorTable.js";
import * as Ice_Logger from "./Logger.js";
import * as Ice_Long from "./Long.js";
import * as Ice_MapUtil from "./MapUtil.js";
import * as Ice_Object from "./Object.js";
// Must be imported after Object.js for the prototype extensions.
import "./ObjectExtensions.js";
import * as Ice_ObjectAdapter from "./ObjectAdapter.js";
import * as Ice_ObjectAdapterFactory from "./ObjectAdapterFactory.js";
import * as Ice_ObjectPrx from "./ObjectPrx.js";
// Must be imported after ObjectPrx.js for the prototype extensions.
import "./ObjectPrxExtensions.js";
import * as Ice_OpaqueEndpointI from "./OpaqueEndpoint.js";
import * as Ice_Operation from "./Operation.js";
import * as Ice_OperationMode from "./OperationMode.js";
import * as Ice_OptionalFormat from "./OptionalFormat.js";
import * as Ice_OutgoingAsync from "./OutgoingAsync.js";
import * as Ice_OutgoingConnectionFactory from "./OutgoingConnectionFactory.js";
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
import * as Ice_ProxyFactory from "./ProxyFactory.js";
import * as Ice_Reference from "./Reference.js";
import * as Ice_ReferenceMode from "./ReferenceMode.js";
import * as Ice_RemoteLogger from "./RemoteLogger.js";
import * as Ice_RequestHandlerFactory from "./RequestHandlerFactory.js";
import * as Ice_RetryException from "./RetryException.js";
import * as Ice_RetryQueue from "./RetryQueue.js";
import * as Ice_Router from "./Router.js";
import * as Ice_RouterInfo from "./RouterInfo.js";
import * as Ice_RouterManager from "./RouterManager.js";
import * as Ice_ServantManager from "./ServantManager.js";
import * as Ice_SocketOperation from "./SocketOperation.js";
import * as Ice_Stream from "./Stream.js";
import * as Ice_StreamHelpers from "./StreamHelpers.js";
import * as Ice_StringUtil from "./StringUtil.js";
import * as Ice_Struct from "./Struct.js";
import * as Ice_TcpEndpointFactory from "./TcpEndpointFactory.js";
import * as Ice_TcpEndpointI from "./TcpEndpointI.js";
import * as Ice_TcpTransceiver from "./TcpTransceiver.js";
import * as Ice_Timer from "./Timer.js";
import * as Ice_TimerUtil from "./TimerUtil.js";
import * as Ice_ToStringMode from "./ToStringMode.js";
import * as Ice_TraceLevels from "./TraceLevels.js";
import * as Ice_TraceUtil from "./TraceUtil.js";
import * as Ice_TypeRegistry from "./TypeRegistry.js";
import * as Ice_UUID from "./UUID.js";
import * as Ice_UnknownSlicedValue from "./UnknownSlicedValue.js";
import * as Ice_Value from "./Value.js";
import * as Ice_ValueFactoryManager from "./ValueFactoryManager.js";
import * as Ice_Version from "./Version.js";
import * as Ice_WSEndpoint from "./WSEndpoint.js";
import * as Ice_WSEndpointFactory from "./WSEndpointFactory.js";
import * as Ice_WSTransceiver from "./WSTransceiver.js";

export const Ice = {
    ...Ice_ACM,
    ...Ice_Address,
    ...Ice_ArrayUtil,
    ...Ice_AsyncResult,
    ...Ice_AsyncResultBase,
    ...Ice_AsyncStatus,
    ...Ice_Base64,
    ...Ice_BatchRequestQueue,
    ...Ice_Buffer,
    ...Ice_Communicator,
    ...Ice_CompactIdRegistry,
    ...Ice_ConnectRequestHandler,
    ...Ice_Connection,
    ...Ice_ConnectionI,
    ...Ice_ConnectionRequestHandler,
    ...Ice_Current,
    ...Ice_DefaultsAndOverrides,
    ...Ice_Endpoint,
    ...Ice_EndpointFactoryManager,
    ...Ice_EndpointI,
    ...Ice_EndpointSelectionType,
    ...Ice_EnumBase,
    ...Ice_ExUtil,
    ...Ice_Exception,
    ...Ice_FacetMap,
    ...Ice_FormatType,
    ...Ice_HashMap,
    ...Ice_HashUtil,
    ...Ice_IPEndpointI,
    ...Ice_IdentityUtil,
    ...Ice_ImplicitContext,
    ...Ice_IncomingAsync,
    ...Ice_Initialize,
    ...Ice_Instance,
    ...Ice_LocalException,
    ...Ice_LocatorInfo,
    ...Ice_LocatorManager,
    ...Ice_LocatorTable,
    ...Ice_Logger,
    ...Ice_Long,
    ...Ice_MapUtil,
    ...Ice_Object,
    ...Ice_ObjectAdapter,
    ...Ice_ObjectAdapterFactory,
    ...Ice_ObjectPrx,
    ...Ice_OpaqueEndpointI,
    ...Ice_Operation,
    ...Ice_OptionalFormat,
    ...Ice_OutgoingAsync,
    ...Ice_OutgoingConnectionFactory,
    ...Ice_ProcessLogger,
    ...Ice_Promise,
    ...Ice_Properties,
    ...Ice_Property,
    ...Ice_PropertyNames,
    ...Ice_Protocol,
    ...Ice_ProtocolInstance,
    ...Ice_ProxyFactory,
    ...Ice_Reference,
    ...Ice_ReferenceMode,
    ...Ice_RequestHandlerFactory,
    ...Ice_RetryException,
    ...Ice_RetryQueue,
    ...Ice_RouterInfo,
    ...Ice_RouterManager,
    ...Ice_ServantManager,
    ...Ice_SocketOperation,
    ...Ice_Stream,
    ...Ice_StreamHelpers,
    ...Ice_StringUtil,
    ...Ice_Struct,
    ...Ice_TcpEndpointFactory,
    ...Ice_TcpEndpointI,
    ...Ice_TcpTransceiver,
    ...Ice_Timer,
    ...Ice_TimerUtil,
    ...Ice_ToStringMode,
    ...Ice_TraceLevels,
    ...Ice_TraceUtil,
    ...Ice_TypeRegistry,
    ...Ice_UUID,
    ...Ice_UnknownSlicedValue,
    ...Ice_Value,
    ...Ice_ValueFactoryManager,
    ...Ice_WSEndpoint,
    ...Ice_WSEndpointFactory,
    ...Ice_WSTransceiver,

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
    ...Ice_Version.Ice,
};
