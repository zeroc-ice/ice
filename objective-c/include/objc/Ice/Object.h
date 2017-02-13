// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
-(void) ice_preMarshal;
-(void) ice_postUnmarshal;
-(BOOL) ice_dispatch:(id<ICERequest>)request;
@end

ICE_API @interface ICEObject : NSObject<ICEObject, NSCopying>
-(BOOL) ice_isA:(NSString*)typeId;
-(void) ice_ping;
-(NSString*) ice_id;
-(NSArray*) ice_ids;
-(BOOL) ice_dispatch:(id<ICERequest>)request;
+(NSString*) ice_staticId;
+(NSString*const*) staticIds__:(int*)count idIndex:(int*)idx;
-(void) write__:(id<ICEOutputStream>)os;
-(void) read__:(id<ICEInputStream>)is;
@end

ICE_API @interface ICEServant : ICEObject
{
    void* object__;
    id delegate__;
}
-(id) initWithDelegate:(id)delegate;
+(id) objectWithDelegate:(id)delegate;
+(BOOL) ice_isA___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(BOOL) ice_ping___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(BOOL) ice_id___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(BOOL) ice_ids___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(BOOL) dispatch__:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(void) writeImpl__:(id<ICEOutputStream>)os;
-(void) readImpl__:(id<ICEInputStream>)is;
-(id) target__;
@end

ICE_API @protocol ICEBlobject<ICEObject>
-(BOOL) ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current;
@end

ICE_API @interface ICEBlobject : ICEServant
@end
