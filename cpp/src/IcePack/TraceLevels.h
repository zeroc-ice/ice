// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    const int nodeRegistry;
    const char* nodeRegistryCat;

    const Ice::LoggerPtr logger;
};

typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IcePack

#endif
