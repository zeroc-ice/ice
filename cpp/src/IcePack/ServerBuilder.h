// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_BUILDER_H
#define ICE_PACK_SERVER_BUILDER_H

#include <IcePack/Internal.h>
#include <IcePack/NodeInfo.h>
#include <IcePack/ComponentBuilder.h>

namespace IcePack
{

class ServerBuilder : public ComponentBuilder
{
public:

    enum ServerKind
    {
	ServerKindCppIceBox,
	ServerKindJavaIceBox,
	ServerKindCppServer,
	ServerKindJavaServer
    };

    ServerBuilder(const NodeInfoPtr&, 
		  const std::map<std::string, std::string>&,
		  const std::vector<std::string>&);

    
    using ComponentBuilder::parse;
    void parse(const std::string&);

    virtual void execute();
    virtual void undo();

    void setClassName(const std::string&);
    void setWorkingDirectory(const std::string&);
    void registerServer();
    void registerAdapter(const std::string&, const std::string&, bool, const std::string&);
    void addService(const std::string&, const std::string&, const std::string&);
    void addOption(const std::string&);
    void addJavaOption(const std::string&);
    void addEnvVar(const std::string&);
    void setKind(ServerKind);

    ServerPrx getServer() const;
    ServerAdapterPrx getServerAdapter(const std::string& name) const;

private:

    NodeInfoPtr _nodeInfo;

    ServerDescription _description;
    std::vector<std::string> _serverAdapterIds;
    ServerKind _kind;
    std::string _className;
    std::string _libraryPath;
    std::vector<std::string> _javaOptions;
    std::vector<std::string> _serviceNames;

    ServerPrx _server;
    std::map<std::string, ServerAdapterPrx> _serverAdapters;
};

}

#endif
