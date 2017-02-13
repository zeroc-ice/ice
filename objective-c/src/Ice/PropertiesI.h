// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
