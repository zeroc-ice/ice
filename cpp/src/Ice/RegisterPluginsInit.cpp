//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "RegisterPluginsInit.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Initialize.h"

extern "C"
{
    Ice::Plugin* createStringConverter(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceUDP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceWS(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
}

IceInternal::RegisterPluginsInit::RegisterPluginsInit()
{
    Ice::registerPluginFactory("IceTCP", createIceTCP, true);
    Ice::registerPluginFactory("IceSSL", createIceSSL, true);

    //
    // Include the UDP and WS transport plugins with non-static builds.
    //
#if defined(ICE_API_EXPORTS)
    Ice::registerPluginFactory("IceUDP", createIceUDP, true);
    Ice::registerPluginFactory("IceWS", createIceWS, true);
#endif
}
