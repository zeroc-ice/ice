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
EXPORTED_FUNCTION mxArray* Ice_identityToString(mxArray*);

EXPORTED_FUNCTION mxArray* Ice_Communicator__release(void*);
EXPORTED_FUNCTION mxArray* Ice_Communicator_stringToProxy(void*, const char*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_getProperties(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_createOutputStream(void*, mxArray*, void**);
EXPORTED_FUNCTION mxArray* Ice_Communicator_destroy(void*);

typedef enum 
{
    Normal,
    Nonmutating,
    Idempotent
} Ice_OperationMode;

typedef enum
{
    Random,
    Ordered
} Ice_EndpointSelectionType;

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
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getCommunicator(void*, void**);
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
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_getEndpointSelection(void*, Ice_EndpointSelectionType*);
EXPORTED_FUNCTION mxArray* Ice_ObjectPrx_ice_endpointSelection(void*, Ice_EndpointSelectionType, void**);
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

EXPORTED_FUNCTION mxArray* Ice_SentFuture__release(void*);
EXPORTED_FUNCTION mxArray* Ice_SentFuture_id(void*, unsigned long long*);
EXPORTED_FUNCTION mxArray* Ice_SentFuture_wait(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_SentFuture_state(void*);
EXPORTED_FUNCTION mxArray* Ice_SentFuture_cancel(void*);
EXPORTED_FUNCTION mxArray* Ice_SentFuture_check(void*);

typedef enum 
{
    DefaultFormat,
    CompactFormat,
    SlicedFormat
} Ice_FormatType;

typedef enum
{
    OptionalFormatF1 = 0,
    OptionalFormatF2 = 1,
    OptionalFormatF4 = 2,
    OptionalFormatF8 = 3,
    OptionalFormatSize = 4,
    OptionalFormatVSize = 5,
    OptionalFormatFSize = 6,
    OptionalFormatClass = 7
} Ice_OptionalFormat;

EXPORTED_FUNCTION mxArray* Ice_OutputStream__release(void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBool(void*, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBoolOpt(void*, int, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBoolSeq(void*, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeBoolSeqOpt(void*, int, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeByte(void*, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeByteOpt(void*, int, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeByteSeq(void*, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeByteSeqOpt(void*, int, const unsigned char*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeShort(void*, short);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeShortOpt(void*, int, short);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeShortSeq(void*, const short*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeShortSeqOpt(void*, int, const short*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeInt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeIntOpt(void*, int, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeIntSeq(void*, const int*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeIntSeqOpt(void*, int, const int*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeLong(void*, long long);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeLongOpt(void*, int, long long);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeLongSeq(void*, const long long*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeLongSeqOpt(void*, int, const long long*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeFloat(void*, float);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeFloatOpt(void*, int, float);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeFloatSeq(void*, const float*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeFloatSeqOpt(void*, int, const float*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeDouble(void*, double);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeDoubleOpt(void*, int, double);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeDoubleSeq(void*, const double*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeDoubleSeqOpt(void*, int, const double*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeString(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeStringOpt(void*, int, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeStringSeq(void*, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeStringSeqOpt(void*, int, mxArray*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeSize(void*, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeProxy(void*, void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeProxyOpt(void*, int, void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeEnum(void*, int, int);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_startEncapsulation(void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_startEncapsulationWithFormat(void*, Ice_FormatType);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_endEncapsulation(void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_startSlice(void*, const char*, int, unsigned char);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_endSlice(void*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_writeOptional(void*, int, Ice_OptionalFormat, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_startSize(void*, unsigned int*);
EXPORTED_FUNCTION mxArray* Ice_OutputStream_endSize(void*, unsigned int);

EXPORTED_FUNCTION mxArray* Ice_InputStream__release(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readBool(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readBoolSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readBoolOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readBoolSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readByte(void*, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readByteSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readByteOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readByteSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readShort(void*, short*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readShortSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readShortOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readShortSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readInt(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readIntSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readIntOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readIntSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readLong(void*, long long*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readLongSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readLongOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readLongSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readFloat(void*, float*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readFloatSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readFloatOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readFloatSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readDouble(void*, double*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readDoubleSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readDoubleOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readDoubleSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readString(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readStringSeq(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readStringOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readStringSeqOpt(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_skip(void*, int);
EXPORTED_FUNCTION mxArray* Ice_InputStream_startException(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_endException(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_startEncapsulation(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_endEncapsulation(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_skipEmptyEncapsulation(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_skipEncapsulation(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_getEncoding(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_startSlice(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_endSlice(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_skipSlice(void*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readSize(void*, int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readOptional(void*, int, Ice_OptionalFormat, unsigned char*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readProxy(void*, void**);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readProxyOpt(void*, int, void**);
EXPORTED_FUNCTION mxArray* Ice_InputStream_readEnum(void*, int, int*);
EXPORTED_FUNCTION mxArray* Ice_InputStream_throwException(void*, mxArray*);

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
