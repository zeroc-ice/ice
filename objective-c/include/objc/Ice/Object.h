// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

#import <objc/Ice/Current.h>

//
// Forward declarations.
//
@class ICEObject;
@protocol ICEInputStream;
@protocol ICEOutputStream;

#if defined(__cplusplus)
extern "C"
{
#endif
ICE_API int ICEInternalLookupString(NSString * const arr[], size_t, NSString * __unsafe_unretained);
ICE_API void ICEInternalCheckModeAndSelector(id, ICEOperationMode, SEL, ICECurrent*);
#if defined(__cplusplus)
}
#endif

ICE_API @protocol ICERequest <NSObject>
-(ICECurrent*) getCurrent;
@end

ICE_API @protocol ICEObject <NSObject>
-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)current;
-(void) ice_ping:(ICECurrent*)current;
-(NSString*) ice_id:(ICECurrent*)current;
-(NSArray*) ice_ids:(ICECurrent*)current;
-(void) ice_dispatch:(id<ICERequest>)request;
@end

ICE_API @interface ICEObject : NSObject<ICEObject, NSCopying>
-(BOOL) ice_isA:(NSString*)typeId;
-(void) ice_ping;
-(NSString*) ice_id;
-(NSArray*) ice_ids;
+(NSString*) ice_staticId;
-(void) ice_preMarshal;
-(void) ice_postUnmarshal;
-(id<ICESlicedData>) ice_getSlicedData;
+(NSString*const*) iceStaticIds:(int*)count idIndex:(int*)idx;
-(void) iceWrite:(id<ICEOutputStream>)os;
-(void) iceRead:(id<ICEInputStream>)is;
@end

ICE_API @interface ICEServant : ICEObject
{
    void* iceObject_;
    id iceDelegate_;
}
-(id) initWithDelegate:(id)delegate;
+(id) objectWithDelegate:(id)delegate;
+(void) iceD_ice_isA:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(void) iceD_ice_ping:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(void) iceD_ice_id:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(void) iceD_ice_ids:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(void) iceDispatch:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(void) iceWriteImpl:(id<ICEOutputStream>)os;
-(void) iceReadImpl:(id<ICEInputStream>)is;
-(id) iceTarget;
@end

ICE_API @protocol ICEBlobject<ICEObject>
-(BOOL) ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current;
@end

ICE_API @interface ICEBlobject : ICEServant
@end
