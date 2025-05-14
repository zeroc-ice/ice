// Copyright (c) ZeroC, Inc.

#ifndef ICE_NATIVE_PROPERTIES_ADMIN_H
#define ICE_NATIVE_PROPERTIES_ADMIN_H

#include "Ice/PropertiesAdmin.h"
#include "Ice/PropertyDict.h"

#include <functional>
#include <list>
#include <memory>
#include <mutex>

namespace Ice
{
    /// The default implementation for the PropertiesAdmin facet.
    /// @headerfile Ice/Ice.h
    class ICE_API NativePropertiesAdmin final : public PropertiesAdmin,
                                                public std::enable_shared_from_this<NativePropertiesAdmin>
    {
    public:
        /// Constructs a NativePropertiesAdmin.
        /// @param properties The properties to manage.
        /// @param logger The logger to use for logging.
        NativePropertiesAdmin(PropertiesPtr properties, LoggerPtr logger);

        ~NativePropertiesAdmin() final;

        std::string getProperty(std::string, const Ice::Current&) final;
        Ice::PropertyDict getPropertiesForPrefix(std::string, const Ice::Current&) final;
        void setProperties(Ice::PropertyDict, const Ice::Current&) final;

        /// Registers an update callback that will be invoked when a property update occurs.
        /// @param cb The callback.
        /// @return A function that unregisters the callback.
        std::function<void()> addUpdateCallback(std::function<void(const PropertyDict&)> cb);

    private:
        void removeUpdateCallback(std::list<std::function<void(const PropertyDict&)>>::iterator p);

        const Ice::PropertiesPtr _properties;
        const Ice::LoggerPtr _logger;
        std::recursive_mutex _mutex;

        std::list<std::function<void(const Ice::PropertyDict&)>> _updateCallbacks;
    };

    /// A shared pointer to a NativePropertiesAdmin.
    using NativePropertiesAdminPtr = std::shared_ptr<NativePropertiesAdmin>;
}

#endif
