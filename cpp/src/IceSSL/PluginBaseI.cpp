// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/Communicator.h>
#include <IceSSL/PluginBaseI.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

void IceInternal::incRef(PluginBaseI* p) { p->__incRef(); }
void IceInternal::decRef(PluginBaseI* p) { p->__decRef(); }

//
// Protected Methods
//

IceSSL::PluginBaseI::PluginBaseI(const ProtocolPluginFacadePtr& protocolPluginFacade) :
    _protocolPluginFacade(protocolPluginFacade)
{
    _logger = _protocolPluginFacade->getCommunicator()->getLogger();
    _properties = _protocolPluginFacade->getCommunicator()->getProperties();
    _traceLevels = new TraceLevels(_protocolPluginFacade);
}

IceSSL::PluginBaseI::~PluginBaseI()
{
}

TraceLevelsPtr
IceSSL::PluginBaseI::getTraceLevels() const
{
    return _traceLevels;
}

LoggerPtr
IceSSL::PluginBaseI::getLogger() const
{
    return _logger;
}

PropertiesPtr
IceSSL::PluginBaseI::getProperties() const
{
    return _properties;
}

ProtocolPluginFacadePtr
IceSSL::PluginBaseI::getProtocolPluginFacade() const
{
    return _protocolPluginFacade;
}
