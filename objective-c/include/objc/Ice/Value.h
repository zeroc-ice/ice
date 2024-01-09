//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Config.h>

//
// Forward declarations.
//
@protocol ICEInputStream;
@protocol ICEOutputStream;
@protocol ICESlicedData;

ICE_API @interface ICEValue : NSObject<NSCopying>
+(NSString*) ice_staticId;
-(NSString*) ice_id;
-(void) ice_preMarshal;
-(void) ice_postUnmarshal;
-(id<ICESlicedData>) ice_getSlicedData;
-(void) iceWrite:(id<ICEOutputStream>)os;
-(void) iceRead:(id<ICEInputStream>)is;
-(void) iceWriteImpl:(id<ICEOutputStream>)os;
-(void) iceReadImpl:(id<ICEInputStream>)is;
@end
