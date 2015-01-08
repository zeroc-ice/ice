// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

//
// Forward declarations
//
@class ICEIdentity;
@class ICEException;

@protocol ICEObjectPrx;
@protocol ICEObjectAdapter;
@protocol ICEAsyncResult;
@protocol ICEEndpoint;

@interface ICEConnectionInfo : NSObject
{
@protected
    BOOL incoming;
    NSString* adapterName;
    NSString* connectionId;
}

@property(nonatomic) BOOL incoming;
@property(nonatomic, retain) NSString* adapterName;
@property(nonatomic, retain) NSString* connectionId;
@end

@interface ICEIPConnectionInfo : ICEConnectionInfo
{
@protected
    NSString* localAddress;
    ICEInt localPort;
    NSString* remoteAddress;
    ICEInt remotePort;
}

@property(nonatomic, retain) NSString* localAddress;
@property(nonatomic) ICEInt localPort;
@property(nonatomic, retain) NSString* remoteAddress;
@property(nonatomic) ICEInt remotePort;
@end

@interface ICETCPConnectionInfo : ICEIPConnectionInfo
@end

@interface ICEUDPConnectionInfo : ICEIPConnectionInfo
{
@private
    NSString* mcastAddress;
    ICEInt mcastPort;
}
@property(nonatomic, retain) NSString* mcastAddress;
@property(nonatomic) ICEInt mcastPort;
@end

@interface ICESSLConnectionInfo : ICEIPConnectionInfo
{
@private
    NSString* cipher;
    NSArray* certs;
}
@property(nonatomic, retain) NSString* cipher;
@property(nonatomic, retain) NSArray* certs;
@end

@protocol ICEConnection <NSObject>
-(void) close:(BOOL)force;
-(id<ICEObjectPrx>) createProxy:(ICEIdentity*)identity;
-(void) setAdapter:(id<ICEObjectAdapter>)adapter;
-(id<ICEObjectAdapter>) getAdapter;
-(void) flushBatchRequests;
-(id<ICEAsyncResult>) begin_flushBatchRequests;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_flushBatchRequests:(id<ICEAsyncResult>)result;
-(NSString*) type;
-(ICEInt) timeout;
-(NSString*) toString;
-(ICEConnectionInfo*) getInfo;
-(id<ICEEndpoint>) getEndpoint;
@end
