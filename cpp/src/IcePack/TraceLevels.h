// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
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

    const int serverMgr;
    const char* serverMgrCat;

    const int adapterMgr;
    const char* adapterMgrCat;

    const int activator;
    const char* activatorCat;

    const Ice::LoggerPtr logger;
};

typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IcePack

#endif
