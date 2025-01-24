// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_API_EXPORTS
#    define ICEGRID_API_EXPORTS
#endif

#include "IceGrid/IceGrid.h"

using namespace IceGrid;

namespace
{
    std::shared_ptr<RegistryPluginFacade> pluginFacade;
}

namespace IceGrid
{
    ICEGRID_API void setRegistryPluginFacade(const RegistryPluginFacadePtr&);
}

RegistryPluginFacadePtr
IceGrid::getRegistryPluginFacade()
{
    return pluginFacade;
}

void
IceGrid::setRegistryPluginFacade(const RegistryPluginFacadePtr& facade)
{
    pluginFacade = facade;
}
