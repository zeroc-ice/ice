// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
