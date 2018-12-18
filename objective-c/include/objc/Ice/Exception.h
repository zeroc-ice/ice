// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

#import <Foundation/NSException.h>

//
// Forward declarations
//
@protocol ICEOutputStream;
@protocol ICEInputStream;
@protocol ICESlicedData;

@class NSCoder;

ICE_API @interface ICEException : NSException
-(NSString*)ice_name ICE_DEPRECATED_API("ice_name() is deprecated use ice_id() instead.");
-(NSString*)ice_id;
@end

ICE_API @interface ICELocalException : ICEException
{
@protected
    const char* file;
    int line;
}

@property(nonatomic, readonly) NSString* file;
@property(nonatomic, readonly) int line;

-(id)init:(const char*)file line:(int)line;
-(id)init:(const char*)file line:(int)line reason:(NSString*)reason;
+(id)localException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEUserException : ICEException
-(id<ICESlicedData>)ice_getSlicedData;
-(BOOL)iceUsesClasses;
-(void)iceWrite:(id<ICEOutputStream>)stream;
-(void) iceWriteImpl:(id<ICEOutputStream>)os;
-(void)iceRead:(id<ICEInputStream>)stream;
-(void) iceReadImpl:(id<ICEInputStream>)is;
@end
