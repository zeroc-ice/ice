// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_API_EXPORTS
#   define ICEGRID_API_EXPORTS
#endif

#include <IceGrid/IceGrid.h>

using namespace IceGrid;

namespace
{

#ifdef ICE_CPP11_MAPPING
std::shared_ptr<RegistryPluginFacade> pluginFacade;
#else
RegistryPluginFacade* pluginFacade = 0;
#endif
}

namespace IceGrid
{

ICEGRID_API void setRegistryPluginFacade(const RegistryPluginFacadePtr&);

};

RegistryPluginFacadePtr
IceGrid::getRegistryPluginFacade()
{
    return pluginFacade;
}

void
IceGrid::setRegistryPluginFacade(const RegistryPluginFacadePtr& facade)
{
#ifdef ICE_CPP11_MAPPING
    pluginFacade = facade;
#else
    if(pluginFacade)
    {
        pluginFacade->iceDecRef();
    }
    pluginFacade = facade.get();
    if(pluginFacade)
    {
        pluginFacade->iceIncRef();
    }
#endif
}
