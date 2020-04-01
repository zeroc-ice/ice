//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROPERTIES_ADMIN_H
#define ICE_PROPERTIES_ADMIN_H

#include <Ice/PropertiesAdmin.h>

namespace Ice
{

/**
 * Base class for the Properties admin facet.
 * \headerfile Ice/Ice.h
 */
class ICE_API NativePropertiesAdmin
{
public:

    virtual ~NativePropertiesAdmin();

    /**
     * Register an update callback that will be invoked when property updates occur.
     * @param cb The callback.
     */
    virtual std::function<void()> addUpdateCallback(std::function<void(const PropertyDict&)> cb) = 0;
};
ICE_DEFINE_PTR(NativePropertiesAdminPtr, NativePropertiesAdmin);

}

#endif
