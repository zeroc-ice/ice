// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Instance.h>
#include <IceSSL/PluginBaseI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

void IceInternal::incRef(PluginBaseI* p) { p->__incRef(); }
void IceInternal::decRef(PluginBaseI* p) { p->__decRef(); }

//
// Protected Methods
//

IceSSL::PluginBaseI::PluginBaseI(const InstancePtr& instance) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger()),
    _properties(instance->properties())
{
}

IceSSL::PluginBaseI::~PluginBaseI()
{
}
