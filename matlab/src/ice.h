//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_MATLAB_API
#   if defined(ICE_MATLAB_API_EXPORTS)
#       define ICE_MATLAB_API __declspec(dllexport)
#   else
#       define ICE_MATLAB_API __declspec(dllimport)
#   endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct mxArray_tag mxArray; // Forward declaration to avoid importing mex.h here

ICE_MATLAB_API mxArray* Ice_initialize(mxArray*, void*, void**);
ICE_MATLAB_API mxArray* Ice_stringToIdentity(mxArray*);
ICE_MATLAB_API mxArray* Ice_identityToString(mxArray*, mxArray*);
ICE_MATLAB_API mxArray* Ice_stringVersion();
ICE_MATLAB_API mxArray* Ice_intVersion(int*);
ICE_MATLAB_API mxArray* Ice_currentEncoding();
ICE_MATLAB_API mxArray* Ice_currentProtocol();
ICE_MATLAB_API mxArray* Ice_currentProtocolEncoding();

ICE_MATLAB_API mxArray* Ice_Communicator_unref(void*);
ICE_MATLAB_API mxArray* Ice_Communicator_destroy(void*);
ICE_MATLAB_API mxArray* Ice_Communicator_destroyAsync(void*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_stringToProxy(void*, const char*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_proxyToString(void*, void*);
ICE_MATLAB_API mxArray* Ice_Communicator_propertyToProxy(void*, const char*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_proxyToProperty(void*, void*, const char*);
ICE_MATLAB_API mxArray* Ice_Communicator_identityToString(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Communicator_getImplicitContext(void*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_getProperties(void*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_getLogger(void*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_getDefaultRouter(void*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_setDefaultRouter(void*, void*);
ICE_MATLAB_API mxArray* Ice_Communicator_getDefaultLocator(void*, void**);
ICE_MATLAB_API mxArray* Ice_Communicator_setDefaultLocator(void*, void*);
ICE_MATLAB_API mxArray* Ice_Communicator_flushBatchRequests(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Communicator_flushBatchRequestsAsync(void*, mxArray*, void**);

ICE_MATLAB_API mxArray* Ice_ObjectPrx_unref(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_equals(void*, void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_read(void*, mxArray*, mxArray*, int, int, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_write(void*, void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_invoke(void*, const char*, int, mxArray*, unsigned int, mxArray*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_invokeNC(void*, const char*, int, mxArray*, unsigned int);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_invokeAsync(void*, const char*, int, mxArray*, unsigned int,
                                                         mxArray*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_invokeAsyncNC(void*, const char*, int, mxArray*, unsigned int, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_toString(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getIdentity(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_identity(void*, void**, mxArray*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getContext(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_context(void*, void**, mxArray*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getFacet(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_facet(void*, void**, const char*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getAdapterId(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_adapterId(void*, void**, const char*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getNumEndpoints(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getEndpoint(void*, unsigned int, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_createEndpointList(void*, unsigned int, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_setEndpoint(void*, void*, unsigned int, void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_endpoints(void*, void**, void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getLocatorCacheTimeout(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_locatorCacheTimeout(void*, void**, int);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getInvocationTimeout(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_invocationTimeout(void*, void**, int);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getConnectionId(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_connectionId(void*, void**, const char*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isConnectionCached(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_connectionCached(void*, void**, unsigned char);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getEndpointSelection(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_endpointSelection(void*, void**, mxArray*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getEncodingVersion(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_encodingVersion(void*, void**, mxArray*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getRouter(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_router(void*, void**, void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getLocator(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_locator(void*, void**, void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isSecure(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_secure(void*, void**, unsigned char);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isPreferSecure(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_preferSecure(void*, void**, unsigned char);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isTwoway(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_twoway(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isOneway(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_oneway(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isBatchOneway(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_batchOneway(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isDatagram(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_datagram(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isBatchDatagram(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_batchDatagram(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_compress(void*, void**, unsigned char);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getCompress(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_timeout(void*, void**, int);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getTimeout(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_fixed(void*, void**, void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_isFixed(void* self);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getConnection(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getConnectionAsync(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_getCachedConnection(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_flushBatchRequests(void*);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_ice_flushBatchRequestsAsync(void*, void**);
ICE_MATLAB_API mxArray* Ice_ObjectPrx_clone(void*, void**);

ICE_MATLAB_API mxArray* Ice_InvocationFuture_unref(void*);
ICE_MATLAB_API mxArray* Ice_InvocationFuture_wait(void*);
ICE_MATLAB_API mxArray* Ice_InvocationFuture_waitState(void*, mxArray*, double);
ICE_MATLAB_API mxArray* Ice_InvocationFuture_results(void*);
ICE_MATLAB_API mxArray* Ice_InvocationFuture_state(void*);
ICE_MATLAB_API mxArray* Ice_InvocationFuture_cancel(void*);
ICE_MATLAB_API mxArray* Ice_InvocationFuture_check(void*);

ICE_MATLAB_API mxArray* Ice_GetConnectionFuture_unref(void*);
ICE_MATLAB_API mxArray* Ice_GetConnectionFuture_wait(void*);
ICE_MATLAB_API mxArray* Ice_GetConnectionFuture_waitState(void*, mxArray*, double);
ICE_MATLAB_API mxArray* Ice_GetConnectionFuture_fetch(void*, void**);
ICE_MATLAB_API mxArray* Ice_GetConnectionFuture_state(void*);
ICE_MATLAB_API mxArray* Ice_GetConnectionFuture_cancel(void*);

ICE_MATLAB_API mxArray* Ice_SimpleFuture_unref(void*);
ICE_MATLAB_API mxArray* Ice_SimpleFuture_wait(void*);
ICE_MATLAB_API mxArray* Ice_SimpleFuture_waitState(void*, mxArray*, double);
ICE_MATLAB_API mxArray* Ice_SimpleFuture_state(void*);
ICE_MATLAB_API mxArray* Ice_SimpleFuture_cancel(void*);
ICE_MATLAB_API mxArray* Ice_SimpleFuture_check(void*);

ICE_MATLAB_API mxArray* Ice_createProperties(mxArray*, void*, void**);
ICE_MATLAB_API mxArray* Ice_Properties_unref(void*);
ICE_MATLAB_API mxArray* Ice_Properties_getProperty(void*, const char*);
ICE_MATLAB_API mxArray* Ice_Properties_getPropertyWithDefault(void*, const char*, const char*);
ICE_MATLAB_API mxArray* Ice_Properties_getPropertyAsInt(void*, const char*, int*);
ICE_MATLAB_API mxArray* Ice_Properties_getPropertyAsIntWithDefault(void*, const char*, int, int*);
ICE_MATLAB_API mxArray* Ice_Properties_getPropertyAsList(void*, const char*);
ICE_MATLAB_API mxArray* Ice_Properties_getPropertyAsListWithDefault(void*, const char*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Properties_getPropertiesForPrefix(void*, const char*);
ICE_MATLAB_API mxArray* Ice_Properties_setProperty(void*, const char*, const char*);
ICE_MATLAB_API mxArray* Ice_Properties_getCommandLineOptions(void*);
ICE_MATLAB_API mxArray* Ice_Properties_parseCommandLineOptions(void*, const char*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Properties_parseIceCommandLineOptions(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Properties_load(void*, const char*);
ICE_MATLAB_API mxArray* Ice_Properties_clone(void*, void**);

ICE_MATLAB_API mxArray* Ice_Connection_unref(void*);
ICE_MATLAB_API mxArray* Ice_Connection_equals(void*, void*);
ICE_MATLAB_API mxArray* Ice_Connection_close(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Connection_closeAsync(void*, mxArray*, void**);
ICE_MATLAB_API mxArray* Ice_Connection_createProxy(void*, mxArray*, void**);
ICE_MATLAB_API mxArray* Ice_Connection_flushBatchRequests(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Connection_flushBatchRequestsAsync(void*, mxArray*, void**);
ICE_MATLAB_API mxArray* Ice_Connection_getEndpoint(void*, void**);
ICE_MATLAB_API mxArray* Ice_Connection_heartbeat(void*);
ICE_MATLAB_API mxArray* Ice_Connection_heartbeatAsync(void*, void**);
ICE_MATLAB_API mxArray* Ice_Connection_setACM(void*, mxArray*, mxArray*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Connection_getACM(void*);
ICE_MATLAB_API mxArray* Ice_Connection_type(void*);
ICE_MATLAB_API mxArray* Ice_Connection_timeout(void*);
ICE_MATLAB_API mxArray* Ice_Connection_toString(void*);
ICE_MATLAB_API mxArray* Ice_Connection_getInfo(void*);
ICE_MATLAB_API mxArray* Ice_Connection_setBufferSize(void*, int, int);
ICE_MATLAB_API mxArray* Ice_Connection_throwException(void*);

ICE_MATLAB_API mxArray* Ice_Logger_unref(void*);
ICE_MATLAB_API mxArray* Ice_Logger_print(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Logger_trace(void*, mxArray*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Logger_warning(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Logger_error(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_Logger_getPrefix(void*);
ICE_MATLAB_API mxArray* Ice_Logger_cloneWithPrefix(void*, mxArray*, void**);

ICE_MATLAB_API mxArray* Ice_Endpoint_unref(void*);
ICE_MATLAB_API mxArray* Ice_Endpoint_equals(void*, void*);
ICE_MATLAB_API mxArray* Ice_Endpoint_toString(void*);
ICE_MATLAB_API mxArray* Ice_Endpoint_getInfo(void*);

ICE_MATLAB_API mxArray* Ice_ImplicitContext_unref(void*);
ICE_MATLAB_API mxArray* Ice_ImplicitContext_getContext(void*);
ICE_MATLAB_API mxArray* Ice_ImplicitContext_setContext(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_ImplicitContext_containsKey(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_ImplicitContext_get(void*, mxArray*);
ICE_MATLAB_API mxArray* Ice_ImplicitContext_put(void*, mxArray*, mxArray*);
ICE_MATLAB_API mxArray* Ice_ImplicitContext_remove(void*, mxArray*);

#ifdef __cplusplus
}
#endif
