//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROPERTIES_ADMIN_I_H
#define ICE_PROPERTIES_ADMIN_I_H

#include <IceUtil/RecMutex.h>
#include <Ice/Properties.h>
#include <Ice/PropertiesAdmin.h>
#include <Ice/NativePropertiesAdmin.h>
#include <Ice/LoggerF.h>

#ifdef ICE_CPP11_MAPPING
#include <list>
#endif

namespace IceInternal
{

class PropertiesAdminI : public Ice::PropertiesAdmin, public Ice::NativePropertiesAdmin,
#ifdef ICE_CPP11_MAPPING
                         public std::enable_shared_from_this<PropertiesAdminI>,
#endif
                         private IceUtil::RecMutex
{
public:

    PropertiesAdminI(const InstancePtr&);

#ifdef ICE_CPP11_MAPPING
    virtual std::string getProperty(std::string, const Ice::Current&) override;
    virtual Ice::PropertyDict getPropertiesForPrefix(std::string, const Ice::Current&) override;
    virtual void setProperties(::Ice::PropertyDict, const Ice::Current&) override;

    virtual std::function<void()> addUpdateCallback(std::function<void(const Ice::PropertyDict&)>) override;
    void removeUpdateCallback(std::list<std::function<void(const Ice::PropertyDict&)>>::iterator);

#else
    virtual std::string getProperty(const std::string&, const Ice::Current&);
    virtual Ice::PropertyDict getPropertiesForPrefix(const std::string&, const Ice::Current&);
    virtual void setProperties(const Ice::PropertyDict&, const Ice::Current&);

    virtual void addUpdateCallback(const Ice::PropertiesAdminUpdateCallbackPtr&);
    virtual void removeUpdateCallback(const Ice::PropertiesAdminUpdateCallbackPtr&);
#endif

private:

    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;

#ifdef ICE_CPP11_MAPPING
    std::list<std::function<void(const Ice::PropertyDict&)>> _updateCallbacks;
#else
    std::vector<Ice::PropertiesAdminUpdateCallbackPtr> _updateCallbacks;
#endif

};
ICE_DEFINE_PTR(PropertiesAdminIPtr, PropertiesAdminI);

}

#endif
