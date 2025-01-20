// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROPERTIES_ADMIN_I_H
#define ICE_PROPERTIES_ADMIN_I_H

#include "Ice/Logger.h"
#include "Ice/NativePropertiesAdmin.h"
#include "Ice/Properties.h"
#include "Ice/PropertiesAdmin.h"

#include <list>
#include <mutex>

namespace IceInternal
{
    class PropertiesAdminI final : public Ice::PropertiesAdmin,
                                   public Ice::NativePropertiesAdmin,
                                   public std::enable_shared_from_this<PropertiesAdminI>
    {
    public:
        PropertiesAdminI(const InstancePtr&);

        std::string getProperty(std::string, const Ice::Current&) final;
        Ice::PropertyDict getPropertiesForPrefix(std::string, const Ice::Current&) final;
        void setProperties(Ice::PropertyDict, const Ice::Current&) final;

        std::function<void()> addUpdateCallback(std::function<void(const Ice::PropertyDict&)>) final;

    private:
        void removeUpdateCallback(std::list<std::function<void(const Ice::PropertyDict&)>>::iterator);

        const Ice::PropertiesPtr _properties;
        const Ice::LoggerPtr _logger;
        std::recursive_mutex _mutex;

        std::list<std::function<void(const Ice::PropertyDict&)>> _updateCallbacks;
    };
    using PropertiesAdminIPtr = std::shared_ptr<PropertiesAdminI>;
}

#endif
