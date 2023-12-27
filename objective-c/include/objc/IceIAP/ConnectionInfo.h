//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `ConnectionInfo.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Proxy.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/Connection.h>

@class ICEIAPConnectionInfo;
@protocol ICEIAPConnectionInfo;

@protocol ICEIAPConnectionInfo <ICEConnectionInfo>
@end

@interface ICEIAPConnectionInfo : ICEConnectionInfo
{
    NSString *name;
    NSString *manufacturer;
    NSString *modelNumber;
    NSString *firmwareRevision;
    NSString *hardwareRevision;
    NSString *protocol;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *name;
@property(nonatomic, ICE_STRONG_ATTR) NSString *manufacturer;
@property(nonatomic, ICE_STRONG_ATTR) NSString *modelNumber;
@property(nonatomic, ICE_STRONG_ATTR) NSString *firmwareRevision;
@property(nonatomic, ICE_STRONG_ATTR) NSString *hardwareRevision;
@property(nonatomic, ICE_STRONG_ATTR) NSString *protocol;

-(id) init;
-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId name:(NSString*)name manufacturer:(NSString*)manufacturer modelNumber:(NSString*)modelNumber firmwareRevision:(NSString*)firmwareRevision hardwareRevision:(NSString*)hardwareRevision protocol:(NSString*)protocol;
+(id) connectionInfo;
+(id) connectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId name:(NSString*)name manufacturer:(NSString*)manufacturer modelNumber:(NSString*)modelNumber firmwareRevision:(NSString*)firmwareRevision hardwareRevision:(NSString*)hardwareRevision protocol:(NSString*)protocol;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end
