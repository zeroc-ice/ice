//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Config.h>
#import <objc/Ice/PropertiesAdmin.h>

ICE_API @protocol ICEPropertiesAdminUpdateCallback <NSObject>
-(void) updated:(ICEMutablePropertyDict*)properties;
@end

ICE_API @protocol ICENativePropertiesAdmin <NSObject>
-(void) addUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)callback;
-(void) removeUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)callback;
@end
