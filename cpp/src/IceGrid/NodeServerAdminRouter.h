// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODE_SERVER_ADMIN_ROUTER_H
#define ICE_GRID_NODE_SERVER_ADMIN_ROUTER_H

#include <IceGrid/NodeI.h>

namespace IceGrid
{

class NodeServerAdminRouter : public Ice::BlobjectArrayAsync
{
public:
    
    NodeServerAdminRouter(const NodeIPtr&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, 
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

private:
    NodeIPtr _node;
};
}
#endif
