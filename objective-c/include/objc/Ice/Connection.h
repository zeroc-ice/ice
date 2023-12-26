//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Connection.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Proxy.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/ObjectAdapterF.h>
#import <objc/Ice/Identity.h>
#import <objc/Ice/Endpoint.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@class ICEConnectionInfo;
@protocol ICEConnectionInfo;

@protocol ICEConnection;

@class ICEIPConnectionInfo;
@protocol ICEIPConnectionInfo;

@class ICETCPConnectionInfo;
@protocol ICETCPConnectionInfo;

@class ICEUDPConnectionInfo;
@protocol ICEUDPConnectionInfo;

@class ICEWSConnectionInfo;
@protocol ICEWSConnectionInfo;

typedef enum : ICEInt
{
    ICECompressBatchYes,
    ICECompressBatchNo,
    ICECompressBatchBasedOnProxy
} ICECompressBatch;

ICE_API @protocol ICEConnectionInfo <NSObject>
@end

ICE_API @interface ICEConnectionInfo : ICELocalObject
{
    ICEConnectionInfo<ICEConnectionInfo> *underlying;
    BOOL incoming;
    NSString *adapterName;
    NSString *connectionId;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEConnectionInfo<ICEConnectionInfo> *underlying;
@property(nonatomic, assign) BOOL incoming;
@property(nonatomic, ICE_STRONG_ATTR) NSString *adapterName;
@property(nonatomic, ICE_STRONG_ATTR) NSString *connectionId;

-(id) init;
-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId;
+(id) connectionInfo;
+(id) connectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

typedef void (^ICECloseCallback)(id<ICEConnection>);

typedef void (^ICEHeartbeatCallback)(id<ICEConnection>);

typedef enum : ICEInt
{
    ICECloseOff,
    ICECloseOnIdle,
    ICECloseOnInvocation,
    ICECloseOnInvocationAndIdle,
    ICECloseOnIdleForceful
} ICEACMClose;

typedef enum : ICEInt
{
    ICEHeartbeatOff,
    ICEHeartbeatOnDispatch,
    ICEHeartbeatOnIdle,
    ICEHeartbeatAlways
} ICEACMHeartbeat;

ICE_API @interface ICEACM : NSObject <NSCopying>
{
    @private
        ICEInt timeout;
        ICEACMClose close;
        ICEACMHeartbeat heartbeat;
}

@property(nonatomic, assign) ICEInt timeout;
@property(nonatomic, assign) ICEACMClose close;
@property(nonatomic, assign) ICEACMHeartbeat heartbeat;

-(id) init;
-(id) init:(ICEInt)timeout close:(ICEACMClose)close heartbeat:(ICEACMHeartbeat)heartbeat;
+(id) acm;
+(id) aCM ICE_DEPRECATED_API("use acm instead");
+(id) acm:(ICEInt)timeout close:(ICEACMClose)close heartbeat:(ICEACMHeartbeat)heartbeat;
+(id) aCM:(ICEInt)timeout close:(ICEACMClose)close heartbeat:(ICEACMHeartbeat)heartbeat ICE_DEPRECATED_API("use acm instead");
// This class also overrides copyWithZone:
// This class also overrides hash, and isEqual:
@end

typedef enum : ICEInt
{
    ICEConnectionCloseForcefully,
    ICEConnectionCloseGracefully,
    ICEConnectionCloseGracefullyWithWait
} ICEConnectionClose;

ICE_API @protocol ICEConnection <NSObject>
-(void) close:(ICEConnectionClose)mode;
-(id<ICEObjectPrx>) createProxy:(ICEIdentity*)id_;
-(void) setAdapter:(id<ICEObjectAdapter>)adapter;
-(id<ICEObjectAdapter>) getAdapter;
-(id<ICEEndpoint>) getEndpoint;
-(void) flushBatchRequests:(ICECompressBatch)compress;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_flushBatchRequests:(id<ICEAsyncResult>)result;
-(void) setCloseCallback:(ICECloseCallback)callback;
-(void) setHeartbeatCallback:(ICEHeartbeatCallback)callback;
-(void) heartbeat;
-(id<ICEAsyncResult>) begin_heartbeat;
-(id<ICEAsyncResult>) begin_heartbeat:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_heartbeat:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_heartbeat:(id<ICEAsyncResult>)result;
-(void) setACM:(id)timeout close:(id)close heartbeat:(id)heartbeat;
-(ICEACM*) getACM;
-(NSMutableString*) type;
-(ICEInt) timeout;
-(NSMutableString*) toString;
-(ICEConnectionInfo<ICEConnectionInfo>*) getInfo;
-(void) setBufferSize:(ICEInt)rcvSize sndSize:(ICEInt)sndSize;
-(void) throwException;
@end

ICE_API @protocol ICEIPConnectionInfo <ICEConnectionInfo>
@end

ICE_API @interface ICEIPConnectionInfo : ICEConnectionInfo
{
    NSString *localAddress;
    ICEInt localPort;
    NSString *remoteAddress;
    ICEInt remotePort;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *localAddress;
@property(nonatomic, assign) ICEInt localPort;
@property(nonatomic, ICE_STRONG_ATTR) NSString *remoteAddress;
@property(nonatomic, assign) ICEInt remotePort;

-(id) init;
-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId localAddress:(NSString*)localAddress localPort:(ICEInt)localPort remoteAddress:(NSString*)remoteAddress remotePort:(ICEInt)remotePort;
+(id) ipConnectionInfo;
+(id) ipConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId localAddress:(NSString*)localAddress localPort:(ICEInt)localPort remoteAddress:(NSString*)remoteAddress remotePort:(ICEInt)remotePort;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @protocol ICETCPConnectionInfo <ICEIPConnectionInfo>
@end

ICE_API @interface ICETCPConnectionInfo : ICEIPConnectionInfo
{
    ICEInt rcvSize;
    ICEInt sndSize;
}

@property(nonatomic, assign) ICEInt rcvSize;
@property(nonatomic, assign) ICEInt sndSize;

-(id) init;
-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId localAddress:(NSString*)localAddress localPort:(ICEInt)localPort remoteAddress:(NSString*)remoteAddress remotePort:(ICEInt)remotePort rcvSize:(ICEInt)rcvSize sndSize:(ICEInt)sndSize;
+(id) tcpConnectionInfo;
+(id) tcpConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId localAddress:(NSString*)localAddress localPort:(ICEInt)localPort remoteAddress:(NSString*)remoteAddress remotePort:(ICEInt)remotePort rcvSize:(ICEInt)rcvSize sndSize:(ICEInt)sndSize;
// This class also overrides copyWithZone:
@end

ICE_API @protocol ICEUDPConnectionInfo <ICEIPConnectionInfo>
@end

ICE_API @interface ICEUDPConnectionInfo : ICEIPConnectionInfo
{
    NSString *mcastAddress;
    ICEInt mcastPort;
    ICEInt rcvSize;
    ICEInt sndSize;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *mcastAddress;
@property(nonatomic, assign) ICEInt mcastPort;
@property(nonatomic, assign) ICEInt rcvSize;
@property(nonatomic, assign) ICEInt sndSize;

-(id) init;
-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId localAddress:(NSString*)localAddress localPort:(ICEInt)localPort remoteAddress:(NSString*)remoteAddress remotePort:(ICEInt)remotePort mcastAddress:(NSString*)mcastAddress mcastPort:(ICEInt)mcastPort rcvSize:(ICEInt)rcvSize sndSize:(ICEInt)sndSize;
+(id) udpConnectionInfo;
+(id) udpConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId localAddress:(NSString*)localAddress localPort:(ICEInt)localPort remoteAddress:(NSString*)remoteAddress remotePort:(ICEInt)remotePort mcastAddress:(NSString*)mcastAddress mcastPort:(ICEInt)mcastPort rcvSize:(ICEInt)rcvSize sndSize:(ICEInt)sndSize;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

typedef NSDictionary ICEHeaderDict;
typedef NSMutableDictionary ICEMutableHeaderDict;

ICE_API @protocol ICEWSConnectionInfo <ICEConnectionInfo>
@end

ICE_API @interface ICEWSConnectionInfo : ICEConnectionInfo
{
    ICEHeaderDict *headers;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEHeaderDict *headers;

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId headers:(ICEHeaderDict*)headers;
+(id) wsConnectionInfo;
+(id) wsConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId headers:(ICEHeaderDict*)headers;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end
