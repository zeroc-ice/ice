// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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
    IceUtil::Mutex mutex_;
    std::vector<Ice::PropertiesAdminUpdateCallbackPtr> callbacks_;
}
@end
