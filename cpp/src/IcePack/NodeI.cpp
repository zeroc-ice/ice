// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <IcePack/NodeI.h>
#include <IcePack/Activator.h>
#include <IcePack/ServerFactory.h>

//
// Just to get the hostname
//
#include <Ice/ProtocolPluginFacade.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace IcePack;

NodeI::NodeI(const ActivatorPtr& activator, 
	     const string& name, 
	     const ServerFactoryPtr& factory, 
	     const Ice::CommunicatorPtr& communicator,
	     const Ice::PropertiesPtr& properties) :
    _activator(activator),
    _name(name),
    _hostname(IceInternal::getProtocolPluginFacade(communicator)->getDefaultHost()),
    _factory(factory)
{
    _tmpDir = properties->getProperty("IcePack.Node.Data");
    _tmpDir = _tmpDir + (_tmpDir[_tmpDir.length() - 1] == '/' ? "" : "/") + "tmp/";
}

ServerPrx
NodeI::createServer(const string& name, const ServerDescriptorPtr& desc, const ::Ice::Current&) const
{
    return _factory->createServer(name, desc);
}

ServerAdapterPrx
NodeI::createServerAdapter(const ServerPrx& server, const string& id, const ::Ice::Current&) const
{
    return _factory->createServerAdapter(id, server);
}

string
NodeI::createTmpDir(const Ice::Current&) const
{
    string dir = _tmpDir + IceUtil::generateUUID();
#ifdef _WIN32
    _mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
    return dir;
}

void
NodeI::destroyTmpDir(const string& path, const Ice::Current&) const
{
    rmdir(path.c_str());
}

std::string
NodeI::getName(const Ice::Current&) const
{
    return _name;
}

std::string
NodeI::getHostname(const Ice::Current&) const
{
    return _hostname;
}

void
NodeI::shutdown(const Ice::Current&) const
{
    _activator->shutdown();
}

