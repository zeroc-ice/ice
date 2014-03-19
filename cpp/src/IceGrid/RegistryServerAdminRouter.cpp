// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/RegistryServerAdminRouter.h>
#include <Ice/Ice.h>

using namespace IceGrid;
using namespace Ice;
using namespace std;

namespace
{

class InvokeAMICallback : public AMI_Array_Object_ice_invoke
{
public:

    InvokeAMICallback(const AMD_Object_ice_invokePtr& cb) :
        _cb(cb)
    {
    }

    virtual void ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
    {
        _cb->ice_response(ok, outParams);
    }
    
    virtual void ice_exception(const Ice::Exception&)
    {
        _cb->ice_exception(ObjectNotExistException(__FILE__, __LINE__)); // Server admin object is unreachable
    }
    
private:
    AMD_Object_ice_invokePtr _cb;
};

}

IceGrid::RegistryServerAdminRouter::RegistryServerAdminRouter(const DatabasePtr& database) :
    _database(database)
{
}

void
IceGrid::RegistryServerAdminRouter::ice_invoke_async(const AMD_Object_ice_invokePtr& cb, 
                                                     const pair<const Byte*, const Byte*>& inParams,
                                                     const Current& current)
{
    ObjectPrx target = 0;

    try
    {
        target = _database->getServer(current.id.name)->getAdminProxy();
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const DeploymentException&)
    {
    }

    if(target == 0)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }
    
    //
    // Set the facet
    //
    target = target->ice_facet(current.facet);

    //
    // Call with AMI
    //
    target->ice_invoke_async(new InvokeAMICallback(cb), current.operation, current.mode, inParams, current.ctx);
}
