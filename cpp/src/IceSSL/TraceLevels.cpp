// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
