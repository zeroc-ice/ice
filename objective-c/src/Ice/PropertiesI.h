//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Properties.h>

#import <objc/Ice/LocalObject.h>
#import <objc/Ice/NativePropertiesAdmin.h>

#import <ObjectI.h>

#include <Ice/Properties.h>
#include <Ice/NativePropertiesAdmin.h>

@interface ICEProperties : ICELocalObject<ICEProperties>
{
    Ice::Properties* properties_;
}
-(Ice::Properties*)properties;
@end

@interface ICENativePropertiesAdmin : ICEServantWrapper<ICENativePropertiesAdmin>
{
    std::mutex mutex_;
    std::vector<std::pair<id, std::function<void()>>> callbacks_;
}
@end
