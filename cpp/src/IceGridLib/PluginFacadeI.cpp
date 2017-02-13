// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_API_EXPORTS
#   define ICE_GRID_API_EXPORTS
#endif

#include <IceGrid/IceGrid.h>

using namespace IceGrid;

namespace
{

RegistryPluginFacade* pluginFacade = 0;

};

namespace IceGrid
{

ICE_GRID_API void setRegistryPluginFacade(const RegistryPluginFacadePtr&);

};

RegistryPluginFacadePtr
IceGrid::getRegistryPluginFacade()
{
    return pluginFacade;
}

void
IceGrid::setRegistryPluginFacade(const RegistryPluginFacadePtr& facade)
{
    if(pluginFacade)
    {
        pluginFacade->__decRef();
    }
    pluginFacade = facade.get();
    if(pluginFacade)
    {
        pluginFacade->__incRef();
    }
}
