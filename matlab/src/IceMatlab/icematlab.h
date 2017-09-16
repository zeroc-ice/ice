// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#ifdef EXPORT_FCNS
#define EXPORTED_FUNCTION __declspec(dllexport)
#else
#define EXPORTED_FUNCTION __declspec(dllimport)
#endif
#else
#define EXPORTED_FUNCTION
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct mxArray_tag mxArray; // Forward declaration to avoid importing mex.h here

EXPORTED_FUNCTION mxArray* Ice_initialize(mxArray*, void*, void**);
EXPORTED_FUNCTION mxArray* Ice_stringToIdentity(mxArray*);
EXPORTED_FUNCTION mxArray* Ice_identityToString(mxArray*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_stringVersion();
EXPORTED_FUNCTION mxArray* Ice_intVersion(int*);
EXPORTED_FUNCTION mxArray* Ice_currentEncoding();
EXPORTED_FUNCTION mxArray* Ice_currentProtocol();
EXPORTED_FUNCTION mxArray* Ice_currentProtocolEncoding();

EXPORTED_FUNCTION mxArray* Ice_Communicator__release(void*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_destroy(void*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_destroyAsync(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_stringToProxy(void*, const char*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_proxyToString(void*, void*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_propertyToProxy(void*, const char*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_proxyToProperty(void*, void*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_identityToString(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_getProperties(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_getDefaultRouter(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_setDefaultRouter(void*, void*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_getDefaultLocator(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_setDefaultLocator(void*, void*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_flushBatchRequests(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_flushBatchRequestsAsync(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_createOutputStream(void*, mxArray*, void**);

typedef enum 
{
    Normal,
    Nonmutating,
    Idempotent
} Ice_OperationMode;

EXPORTED_FUNCTION mxArray* Ice_ObjectPrx__release(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_equals(void*, void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_createOutputStream(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_invoke(void*, const char*, Ice_OperationMode, void*, mxArray*,
                                                    unsigned char*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_invokeNC(void*, const char*, Ice_OperationMode, void*, unsigned char*,
                                                      void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_invokeAsync(void*, const char*, Ice_OperationMode, void*, mxArray*,
                                                         void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_invokeAsyncNC(void*, const char*, Ice_OperationMode, void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_toString(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getIdentity(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_identity(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getContext(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_context(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getFacet(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_facet(void*, const char*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getAdapterId(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_adapterId(void*, const char*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getEndpoints(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_endpoints(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getLocatorCacheTimeout(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_locatorCacheTimeout(void*, int, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getInvocationTimeout(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_invocationTimeout(void*, int, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getConnectionId(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_connectionId(void*, const char*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isConnectionCached(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_connectionCached(void*, unsigned char, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getEndpointSelection(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_endpointSelection(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getEncodingVersion(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_encodingVersion(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getRouter(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_router(void*, void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getLocator(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_locator(void*, void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isSecure(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_secure(void*, unsigned char, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isPreferSecure(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_preferSecure(void*, unsigned char, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isTwoway(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_twoway(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isOneway(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_oneway(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isBatchOneway(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_batchOneway(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isDatagram(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_datagram(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_isBatchDatagram(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_batchDatagram(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_compress(void*, unsigned char, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_timeout(void*, int, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getConnection(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getConnectionAsync(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getCachedConnection(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_flushBatchRequests(void*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_flushBatchRequestsAsync(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_clone(void*, void**);

EXPORTED_FUNCTION mxArray* Ice_InvocationFuture__release(void*);
EXPORTED_FUNCTION mxArray* Ice_InvocationFuture_id(void*, unsigned long long*);
EXPORTED_FUNCTION mxArray* Ice_InvocationFuture_wait(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_InvocationFuture_stream(void*, unsigned char*, void**);
EXPORTED_FUNCTION mxArray* Ice_InvocationFuture_state(void*);
EXPORTED_FUNCTION mxArray* Ice_InvocationFuture_cancel(void*);
EXPORTED_FUNCTION mxArray* Ice_InvocationFuture_check(void*);

EXPORTED_FUNCTION mxArray* Ice_GetConnectionFuture__release(void*);
EXPORTED_FUNCTION mxArray* Ice_GetConnectionFuture_id(void*, unsigned long long*);
EXPORTED_FUNCTION mxArray* Ice_GetConnectionFuture_wait(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_GetConnectionFuture_fetch(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_GetConnectionFuture_state(void*);
EXPORTED_FUNCTION mxArray* Ice_GetConnectionFuture_cancel(void*);

EXPORTED_FUNCTION mxArray* Ice_SimpleFuture__release(void*);
EXPORTED_FUNCTION mxArray* Ice_SimpleFuture_id(void*, unsigned long long*);
EXPORTED_FUNCTION mxArray* Ice_SimpleFuture_wait(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_SimpleFuture_state(void*);
EXPORTED_FUNCTION mxArray* Ice_SimpleFuture_cancel(void*);
EXPORTED_FUNCTION mxArray* Ice_SimpleFuture_check(void*);

EXPORTED_FUNCTION mxArray* Ice_OutputStream__release(void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBool(void*, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBoolSeq(void*, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeByte(void*, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeByteSeq(void*, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeShort(void*, short);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeShortSeq(void*, const short*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeInt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeIntSeq(void*, const int*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeLong(void*, long long);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeLongSeq(void*, const long long*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeFloat(void*, float);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeFloatSeq(void*, const float*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeDouble(void*, double);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeDoubleSeq(void*, const double*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeString(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeStringSeq(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeSize(void*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeProxy(void*, void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeEnum(void*, int, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_startSize(void*, unsigned int*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_endSize(void*, unsigned int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_pos(void*, unsigned int*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_rewriteByte(void*, unsigned char, unsigned int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_rewriteInt(void*, int, unsigned int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBlob(void*, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_getEncoding(void*);

EXPORTED_FUNCTION mxArray* Ice_InputStream__release(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readBool(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readBoolSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readByte(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readByteSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readShort(void*, short*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readShortSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readInt(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readIntSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readLong(void*, long long*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readLongSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readFloat(void*, float*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readFloatSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readDouble(void*, double*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readDoubleSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readString(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readStringSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_skip(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_skipSize(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_getEncoding(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readSize(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readProxy(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readEnum(void*, int, int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_pos(void*, unsigned int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_setPos(void*, unsigned int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_size(void*, unsigned int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_getBytes(void*, unsigned int, unsigned int);

EXPORTED_FUNCTION mxArray* Ice_createProperties(mxArray*, void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Properties__release(void*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getProperty(void*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getPropertyWithDefault(void*, const char*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getPropertyAsInt(void*, const char*, int*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getPropertyAsIntWithDefault(void*, const char*, int, int*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getPropertyAsList(void*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getPropertyAsListWithDefault(void*, const char*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getPropertiesForPrefix(void*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Properties_setProperty(void*, const char*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Properties_getCommandLineOptions(void*);
EXPORTED_FUNCTION mxArray* Ice_Properties_parseCommandLineOptions(void*, const char*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Properties_parseIceCommandLineOptions(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Properties_load(void*, const char*);
EXPORTED_FUNCTION mxArray* Ice_Properties_clone(void*, void**);

EXPORTED_FUNCTION mxArray* Ice_Connection__release(void*);
EXPORTED_FUNCTION mxArray* Ice_Connection_close(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Connection_closeAsync(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_Connection_createProxy(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_Connection_flushBatchRequests(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Connection_flushBatchRequestsAsync(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_Connection_heartbeat(void*);
EXPORTED_FUNCTION mxArray* Ice_Connection_heartbeatAsync(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Connection_setACM(void*, mxArray*, mxArray*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_Connection_getACM(void*);
EXPORTED_FUNCTION mxArray* Ice_Connection_type(void*);
EXPORTED_FUNCTION mxArray* Ice_Connection_timeout(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_Connection_toString(void*);
EXPORTED_FUNCTION mxArray* Ice_Connection_setBufferSize(void*, int, int);
EXPORTED_FUNCTION mxArray* Ice_Connection_throwException(void*);

#ifdef __cplusplus
}
#endif
