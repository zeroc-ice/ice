// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROPERTIES_ADMIN_I_H
#define ICE_PROPERTIES_ADMIN_I_H

#include <IceUtil/RecMutex.h>
#include <Ice/Properties.h>
#include <Ice/PropertiesAdmin.h>
#include <Ice/NativePropertiesAdmin.h>
#include <Ice/LoggerF.h>

namespace Ice
{

class ICE_API PropertiesAdminI : public PropertiesAdmin, public NativePropertiesAdmin, private IceUtil::RecMutex
{
public:

    PropertiesAdminI(const std::string&, const PropertiesPtr&, const LoggerPtr&);

    virtual std::string getProperty(const std::string&, const Current&);
    virtual PropertyDict getPropertiesForPrefix(const std::string&, const Current&);
    virtual void setProperties_async(const AMD_PropertiesAdmin_setPropertiesPtr&, const PropertyDict&, const Current&);

    virtual void addUpdateCallback(const PropertiesAdminUpdateCallbackPtr&);
    virtual void removeUpdateCallback(const PropertiesAdminUpdateCallbackPtr&);

private:

    const std::string _name;
    const PropertiesPtr _properties;
    const LoggerPtr _logger;
    std::vector<PropertiesAdminUpdateCallbackPtr> _updateCallbacks;
};
typedef IceUtil::Handle<PropertiesAdminI> PropertiesAdminIPtr;

}

#endif
