// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

namespace IceInternal
{

class PropertiesAdminI : public Ice::PropertiesAdmin, public Ice::NativePropertiesAdmin, private IceUtil::RecMutex
{
public:

    PropertiesAdminI(const Ice::PropertiesPtr&, const Ice::LoggerPtr&);

    virtual std::string getProperty(const std::string&, const Ice::Current&);
    virtual Ice::PropertyDict getPropertiesForPrefix(const std::string&, const Ice::Current&);
    virtual void setProperties_async(const Ice::AMD_PropertiesAdmin_setPropertiesPtr&, const Ice::PropertyDict&, const Ice::Current&);

    virtual void addUpdateCallback(const Ice::PropertiesAdminUpdateCallbackPtr&);
    virtual void removeUpdateCallback(const Ice::PropertiesAdminUpdateCallbackPtr&);

private:

    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
    std::vector<Ice::PropertiesAdminUpdateCallbackPtr> _updateCallbacks;
};
typedef IceUtil::Handle<PropertiesAdminI> PropertiesAdminIPtr;

}

#endif
