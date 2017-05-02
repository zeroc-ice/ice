// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
