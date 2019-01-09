// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Config.h>

#import <objc/Ice/Communicator.h>
#import <objc/Ice/Properties.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/Connection.h>
#import <objc/Ice/Version.h>

//
// Forward declarations.
//
@protocol ICELogger;

ICE_API @protocol ICEDispatcherCall <NSObject>
-(void) run;
@end

ICE_API @protocol ICEBatchRequest <NSObject>
-(void) enqueue;
-(int) getSize;
-(NSString*) getOperation;
-(id<ICEObjectPrx>) getProxy;
@end

ICE_API @interface ICEInitializationData : NSObject
{
@private
    id<ICEProperties> properties;
    id<ICELogger> logger;
    void(^dispatcher)(id<ICEDispatcherCall>, id<ICEConnection>);
    void(^batchRequestInterceptor)(id<ICEBatchRequest>, int, int);
    NSDictionary* prefixTable_;
}
@property(retain, nonatomic) id<ICEProperties> properties;
@property(retain, nonatomic) id<ICELogger> logger;
@property(copy, nonatomic) void(^dispatcher)(id<ICEDispatcherCall>, id<ICEConnection>);
@property(copy, nonatomic) void(^batchRequestInterceptor)(id<ICEBatchRequest>, int, int);
@property(retain, nonatomic) NSDictionary* prefixTable_;

-(id) init:(id<ICEProperties>)properties logger:(id<ICELogger>)logger
                                     dispatcher:(void(^)(id<ICEDispatcherCall>, id<ICEConnection>))d
                        batchRequestInterceptor:(void(^)(id<ICEBatchRequest>, int, int))i;
+(id) initializationData;
+(id) initializationData:(id<ICEProperties>)properties logger:(id<ICELogger>)logger
                                                   dispatcher:(void(^)(id<ICEDispatcherCall>, id<ICEConnection>))d
                                      batchRequestInterceptor:(void(^)(id<ICEBatchRequest>, int, int))i;
// This class also overrides copyWithZone:, hash, isEqual:, and dealloc.
@end

extern ICEEncodingVersion* ICEEncoding_1_0;
extern ICEEncodingVersion* ICEEncoding_1_1;
extern ICEEncodingVersion* ICECurrentEncoding;

extern ICEProtocolVersion* ICEProtocol_1_0;
extern ICEProtocolVersion* ICECurrentProtocol;
extern ICEEncodingVersion* ICECurrentProtocolEncoding;

ICE_API @interface ICEUtil : NSObject
+(id<ICEProperties>) createProperties;
+(id<ICEProperties>) createProperties:(int*)argc argv:(char*[])argv;
+(id<ICECommunicator>) createCommunicator;
+(id<ICECommunicator>) createCommunicator:(ICEInitializationData*)initData;
+(id<ICECommunicator>) createCommunicator:(int*)argc argv:(char*[])argv;
+(id<ICECommunicator>) createCommunicator:(int*)argc argv:(char*[])argv initData:(ICEInitializationData*)initData;
+(id<ICECommunicator>) createCommunicator:(int*)argc argv:(char*[])argv configFile:(NSString*)configFile;
+(id<ICEInputStream>) createInputStream:(id<ICECommunicator>)communicator data:(NSData*)data;
+(id<ICEInputStream>) createInputStream:(id<ICECommunicator>)c data:(NSData*)data encoding:(ICEEncodingVersion*)e;
+(id<ICEOutputStream>) createOutputStream:(id<ICECommunicator>)communicator;
+(id<ICEOutputStream>) createOutputStream:(id<ICECommunicator>)c encoding:(ICEEncodingVersion*)e;
+(NSString*) generateUUID;
+(NSArray*)argsToStringSeq:(int)argc argv:(char*[])argv;
+(void)stringSeqToArgs:(NSArray*)args argc:(int*)argc argv:(char*[])argv;
+(ICEIdentity*) stringToIdentity:(NSString*)str;
+(NSMutableString*) identityToString:(ICEIdentity*)ident toStringMode:(ICEToStringMode)toStringMode;
+(NSMutableString*) identityToString:(ICEIdentity*)ident;
@end

@interface ICEEncodingVersion(StringConv)
+(ICEEncodingVersion*) encodingVersionWithString:(NSString*)str;
@end

@interface ICEProtocolVersion(StringConv)
+(ICEProtocolVersion*) protocolVersionWithString:(NSString*)str;
@end

extern void ICEregisterIceStringConverter(BOOL);
extern void ICEregisterIceSSL(BOOL);
extern void ICEregisterIceUDP(BOOL);
extern void ICEregisterIceWS(BOOL);
extern void ICEregisterIceDiscovery(BOOL);
extern void ICEregisterIceLocatorDiscovery(BOOL);
#if defined(__APPLE__) && TARGET_OS_IPHONE > 0
extern void ICEregisterIceIAP(BOOL);
#endif
