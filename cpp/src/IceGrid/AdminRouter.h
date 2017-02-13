// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ADMIN_ROUTER_H
#define ICE_GRID_ADMIN_ROUTER_H

#include <Ice/Ice.h>

namespace IceGrid
{

//
// An Admin Router routes requests to an admin object
//
class AdminRouter : public Ice::BlobjectArrayAsync
{
public:
    
    virtual Ice::ObjectPrx getTarget(const Ice::Current&) = 0;

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, 
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);
};

}
#endif
