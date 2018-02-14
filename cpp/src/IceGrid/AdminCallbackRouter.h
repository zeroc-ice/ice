// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ADMIN_CALLBACK_ROUTER_H
#define ICE_GRID_ADMIN_CALLBACK_ROUTER_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <map>

namespace IceGrid
{

//
// The AdminCallbackRouter routes callbacks from the servers, nodes etc. to the
// admin clients using the admin-client => registry connection.
//

class AdminCallbackRouter : public Ice::BlobjectArrayAsync
{
public:

    void addMapping(const std::string&, const Ice::ConnectionPtr&);
    void removeMapping(const std::string&);
    
    virtual void invokeResponse(bool, 
                                const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&, 
                                const Ice::AMD_Object_ice_invokePtr&);
    
    virtual void invokeException(const Ice::Exception&, const Ice::AMD_Object_ice_invokePtr&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);
    
private:
    
    IceUtil::Mutex _mutex;
    std::map<std::string, Ice::ConnectionPtr> _categoryToConnection;
};

typedef IceUtil::Handle<AdminCallbackRouter> AdminCallbackRouterPtr;
}
#endif
