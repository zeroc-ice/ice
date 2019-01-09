// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Config.h>
#import <objc/Ice/PropertiesAdmin.h>

ICE_API @protocol ICEPropertiesAdminUpdateCallback <NSObject>
-(void) updated:(ICEMutablePropertyDict*)properties;
@end

ICE_DEPRECATED_API("Use NSObject instead")
ICE_API @interface ICEPropertiesAdminUpdateCallback : NSObject
@end

ICE_API @protocol ICENativePropertiesAdmin <NSObject>
-(void) addUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)callback;
-(void) removeUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)callback;
@end
