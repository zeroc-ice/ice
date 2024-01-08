//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `EndpointInfo.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/Endpoint.h>

@class ICEIAPEndpointInfo;
@protocol ICEIAPEndpointInfo;

@protocol ICEIAPEndpointInfo <ICEEndpointInfo>
@end

@interface ICEIAPEndpointInfo : ICEEndpointInfo
{
    NSString *manufacturer;
    NSString *modelNumber;
    NSString *name;
    NSString *protocol;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *manufacturer;
@property(nonatomic, ICE_STRONG_ATTR) NSString *modelNumber;
@property(nonatomic, ICE_STRONG_ATTR) NSString *name;
@property(nonatomic, ICE_STRONG_ATTR) NSString *protocol;

-(id) init;
-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress manufacturer:(NSString*)manufacturer modelNumber:(NSString*)modelNumber name:(NSString*)name protocol:(NSString*)protocol;
+(id) endpointInfo;
+(id) endpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress manufacturer:(NSString*)manufacturer modelNumber:(NSString*)modelNumber name:(NSString*)name protocol:(NSString*)protocol;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end
