// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

#ifndef ICE_CPP11_MAPPING
/**
 * An application can be notified when its configuration properties are modified
 * via the Properties admin facet. The application must define a subclass of
 * PropertiesAdminUpdateCallback and register it with the facet. The facet
 * implements the class NativePropertiesAdmin, so the application needs to
 * downcast the facet to this type in order to register the callback.
 *
 * For example:
 *
 * \code
 * Ice::ObjectPtr obj = communicator->findAdminFacet("Properties");
 * assert(obj); // May be null if the facet is not enabled
 * NativePropertiesAdminPtr facet = NativePropertiesAdminPtr::dynamicCast(obj);
 * PropertiesAdminUpdateCallbackPtr myCallback = ...;
 * facet->addUpdateCallback(myCallback);
 * \endcode
 *
 * Ice ignores any exceptions raised by the callback.
 * \headerfile Ice/Ice.h
 */
class ICE_API PropertiesAdminUpdateCallback : public virtual Ice::LocalObject
{
public:

    virtual ~PropertiesAdminUpdateCallback();

    /**
     * Called when the communicator's properties have been updated.
     * @param d A dictionary containing the properties that were added, changed or removed,
     * with a removed property denoted by an entry whose value is an empty string.
     */
    virtual void updated(const PropertyDict& d) = 0;
};
typedef IceUtil::Handle<PropertiesAdminUpdateCallback> PropertiesAdminUpdateCallbackPtr;
#endif

/**
 * Base class for the Properties admin facet.
 * \headerfile Ice/Ice.h
 */
class ICE_API NativePropertiesAdmin
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual ~NativePropertiesAdmin();

#ifdef ICE_CPP11_MAPPING
    /**
     * Register an update callback that will be invoked when property updates occur.
     * @param cb The callback.
     */
    virtual std::function<void()> addUpdateCallback(std::function<void(const PropertyDict&)> cb) = 0;
#else
    /**
     * Register an update callback that will be invoked when property updates occur.
     * @param cb The callback.
     */
    virtual void addUpdateCallback(const PropertiesAdminUpdateCallbackPtr& cb) = 0;
    /**
     * Remove an update callback.
     * @param cb The callback to be removed.
     */
    virtual void removeUpdateCallback(const PropertiesAdminUpdateCallbackPtr& cb) = 0;
#endif
};
ICE_DEFINE_PTR(NativePropertiesAdminPtr, NativePropertiesAdmin);

}

#endif
