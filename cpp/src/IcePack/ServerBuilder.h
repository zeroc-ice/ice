// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    ServerBuilder(const NodeInfoPtr&, const std::map<std::string, std::string>&, const std::string&, 
		  const std::vector<std::string>&);

    void parse(const std::string&);

    virtual void execute();
    virtual void undo();

    void setClassName(const std::string&);
    void setWorkingDirectory(const std::string&);
    void registerServer();
    void registerAdapter(const std::string&, const std::string&);
    void addService(const std::string&, const std::string&);
    void addOption(const std::string&);
    void addJavaOption(const std::string&);
    void setKind(ServerKind);
    void setActivationMode(ServerActivation);

    ServerPrx getServer() const;
    ServerAdapterPrx getServerAdapter(const std::string& name) const;

private:

    NodeInfoPtr _nodeInfo;

    ServerDescription _description;
    std::vector<std::string> _serverAdapterNames;
    ServerKind _kind;
    std::string _className;
    std::string _libraryPath;
    std::vector<std::string> _javaOptions;

    ServerPrx _server;
    std::map<std::string, ServerAdapterPrx> _serverAdapters;
};

}

#endif
