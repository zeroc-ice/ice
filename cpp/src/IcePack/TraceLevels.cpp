// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Properties.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

TraceLevels::TraceLevels(const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& theLogger) :
    serverMgr(0),
    serverMgrCat("ServerManager"),
    adapterMgr(0),
    adapterMgrCat("AdapterManager"),
    activator(0),
    activatorCat("Activator"),
    logger(theLogger)
{
    const string keyBase = "IcePack.Trace.";
    const_cast<int&>(serverMgr) = properties->getPropertyAsInt(keyBase + serverMgrCat);
    const_cast<int&>(adapterMgr) = properties->getPropertyAsInt(keyBase + adapterMgrCat);
    const_cast<int&>(activator) = properties->getPropertyAsInt(keyBase + activatorCat);
}

TraceLevels::~TraceLevels()
{
}
