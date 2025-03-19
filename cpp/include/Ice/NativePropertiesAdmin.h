// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROPERTIES_ADMIN_H
#define ICE_PROPERTIES_ADMIN_H

#include "Ice/PropertyDict.h"

#include <functional>
#include <memory>

namespace Ice
{
    /// Base class for the Properties admin facet.
    /// \headerfile Ice/Ice.h

    // This class must be ICE_API with an out-of-line destructor because we dynamic_pointer_cast to it.
    class ICE_API NativePropertiesAdmin
    {
    public:
        virtual ~NativePropertiesAdmin();

        /// Register an update callback that will be invoked when property updates occur.
        /// @param cb The callback.
        virtual std::function<void()> addUpdateCallback(std::function<void(const PropertyDict&)> cb) = 0;
    };

    /// A shared pointer to a NativePropertiesAdmin.
    using NativePropertiesAdminPtr = std::shared_ptr<NativePropertiesAdmin>;
}

#endif
