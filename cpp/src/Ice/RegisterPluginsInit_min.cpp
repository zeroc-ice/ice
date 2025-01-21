// Copyright (c) ZeroC, Inc.

#include "Ice/CommunicatorF.h"
#include "Ice/Initialize.h"
#include "RegisterPluginsInit.h"

extern "C"
{
    Ice::Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
}

IceInternal::RegisterPluginsInit::RegisterPluginsInit() noexcept
{
    Ice::registerPluginFactory("IceTCP", createIceTCP, true);
    Ice::registerPluginFactory("IceSSL", createIceSSL, true);
}
