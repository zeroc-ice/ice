// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RegisterPluginsInit.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Initialize.h>

extern "C"
{

Ice::Plugin* createIceUDP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
Ice::Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
Ice::Plugin* createStringConverter(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);

};

IceInternal::RegisterPluginsInit::RegisterPluginsInit()
{
    Ice::registerPluginFactory("IceUDP", createIceUDP, true);
    Ice::registerPluginFactory("IceTCP", createIceTCP, true);
    Ice::registerPluginFactory("IceStringConverter", createStringConverter, false);
}
