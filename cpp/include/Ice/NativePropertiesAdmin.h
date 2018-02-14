// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROPERTIES_ADMIN_H
#define ICE_PROPERTIES_ADMIN_H

#include <Ice/PropertiesAdmin.h>

namespace Ice
{

//
// An application can be notified when its configuration properties are modified
// via the Properties admin facet. The application must define a subclass of
// PropertiesAdminUpdateCallback and register it with the facet. The facet
// implements the class NativePropertiesAdmin, so the application needs to
// downcast the facet to this type in order to register the callback.
//
// For example:
//
// Ice::ObjectPtr obj = communicator->findAdminFacet("Properties");
// assert(obj); // May be null if the facet is not enabled
// NativePropertiesAdminPtr facet = NativePropertiesAdminPtr::dynamicCast(obj);
// PropertiesAdminUpdateCallbackPtr myCallback = ...;
// facet->addUpdateCallback(myCallback);
//
// Ice ignores any exceptions raised by the callback.
//

#ifndef ICE_CPP11_MAPPING
class ICE_API PropertiesAdminUpdateCallback : public virtual Ice::LocalObject
{
public:

    virtual ~PropertiesAdminUpdateCallback();

    virtual void updated(const PropertyDict&) = 0;
};
typedef IceUtil::Handle<PropertiesAdminUpdateCallback> PropertiesAdminUpdateCallbackPtr;
#endif

class ICE_API NativePropertiesAdmin
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual ~NativePropertiesAdmin();

#ifdef ICE_CPP11_MAPPING
    virtual std::function<void()> addUpdateCallback(std::function<void(const PropertyDict&)>) = 0;
#else
    virtual void addUpdateCallback(const PropertiesAdminUpdateCallbackPtr&) = 0;
    virtual void removeUpdateCallback(const PropertiesAdminUpdateCallbackPtr&) = 0;
#endif
};
ICE_DEFINE_PTR(NativePropertiesAdminPtr, NativePropertiesAdmin);

}

#endif
