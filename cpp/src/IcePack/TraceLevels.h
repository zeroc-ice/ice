// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PACK_TRACE_LEVELS_H
#define ICE_PACK_TRACE_LEVELS_H

#include <IceUtil/Shared.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>

namespace IcePack
{

class TraceLevels : public ::IceUtil::Shared
{
public:

    TraceLevels(const ::Ice::PropertiesPtr&, const Ice::LoggerPtr&);
    virtual ~TraceLevels();

    const int server;
    const char* serverCat;

    const int adapter;
    const char* adapterCat;

    const int activator;
    const char* activatorCat;

    const int serverRegistry;
    const char* serverRegistryCat;

    const int adapterRegistry;
    const char* adapterRegistryCat;

    const int objectRegistry;
    const char* objectRegistryCat;

    const int nodeRegistry;
    const char* nodeRegistryCat;

    const Ice::LoggerPtr logger;
};

typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IcePack

#endif
