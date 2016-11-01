// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_H
#define ICEGRID_H

#include <IceGrid/Admin.h>
#include <IceGrid/Descriptor.h>
#include <IceGrid/Exception.h>
#include <IceGrid/FileParser.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Session.h>
#include <IceGrid/UserAccountMapper.h>
#include <IceGrid/PluginFacade.h>

namespace IceGrid
{

//
// Global function to obtain a RegistryPluginFacade
//
ICEGRID_API RegistryPluginFacadePtr getRegistryPluginFacade();

}

#endif
