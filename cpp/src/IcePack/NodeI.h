// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_NODE_I_H
#define ICE_PACK_NODE_I_H

#include <IcePack/Internal.h>

namespace IcePack
{

class Activator;
typedef IceUtil::Handle<Activator> ActivatorPtr;

class ServerFactory;
typedef IceUtil::Handle<ServerFactory> ServerFactoryPtr;

class NodeI : public Node
{
public:

    NodeI(const ActivatorPtr&, const std::string&, const ServerFactoryPtr&, 
	  const Ice::CommunicatorPtr&, const Ice::PropertiesPtr&);

    virtual ServerPrx createServer(const std::string&, const ServerDescriptorPtr&, const Ice::Current&) const;
    virtual ServerAdapterPrx createServerAdapter(const ServerPrx&, const std::string&, const Ice::Current&) const;

    virtual std::string createTmpDir(const Ice::Current&) const;
    virtual void destroyTmpDir(const std::string&, const Ice::Current&) const;

    virtual std::string getName(const Ice::Current&) const;
    virtual std::string getHostname(const Ice::Current&) const;
    virtual void shutdown(const Ice::Current&) const;

private:

    const ActivatorPtr _activator;
    const std::string _name;
    const std::string _hostname;
    const ServerFactoryPtr _factory;
    std::string _tmpDir;
};

}

#endif
