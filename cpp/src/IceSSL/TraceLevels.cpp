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

#include <IceSSL/TraceLevels.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/ProtocolPluginFacade.h>

using namespace std;
using namespace Ice;

void IceInternal::incRef(IceSSL::TraceLevels* p) { p->__incRef(); }
void IceInternal::decRef(IceSSL::TraceLevels* p) { p->__decRef(); }

IceSSL::TraceLevels::TraceLevels(const IceInternal::ProtocolPluginFacadePtr& protocolPluginFacade) :
    network(0),
    security(0),
    securityCat("Security")
{
    const_cast<int&>(network) = protocolPluginFacade->getNetworkTraceLevel();
    networkCat = protocolPluginFacade->getNetworkTraceCategory();

    PropertiesPtr properties = protocolPluginFacade->getCommunicator()->getProperties();
    const_cast<int&>(security) = properties->getPropertyAsInt("IceSSL.Trace.Security");
}

IceSSL::TraceLevels::~TraceLevels()
{
}
