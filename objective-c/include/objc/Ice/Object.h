//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
+(NSString*) ice_staticId;
+(NSString*const*) iceStaticIds:(int*)count idIndex:(int*)idx;
@end

ICE_API @interface ICEServant : ICEObject
{
    // A pointer to the Ice::Object that wraps this ObjC object.
    void* iceObject_;
}
+(void) iceD_ice_isA:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(void) iceD_ice_ping:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(void) iceD_ice_id:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
+(void) iceD_ice_ids:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
-(void) iceDispatch:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os;
@end

ICE_API @protocol ICEBlobject<ICEObject>
-(BOOL) ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current;
@end

ICE_API @interface ICEBlobject : ICEServant
@end
