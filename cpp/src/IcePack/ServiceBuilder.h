// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_SERVICE_BUILDER_H
#define ICE_PACK_SERVICE_BUILDER_H

#include <IcePack/ComponentBuilder.h>
#include <IcePack/NodeInfo.h>

namespace IcePack
{

class ServerBuilder;

class ServiceBuilder : public ComponentBuilder
{
public:

    enum ServiceKind
    {
	ServiceKindStandard,
	ServiceKindFreeze
    };

    ServiceBuilder(const NodeInfoPtr&, ServerBuilder&, 
		   const std::map<std::string, std::string>&,
		   const std::vector<std::string>&);

    using ComponentBuilder::parse;
    
    void parse(const std::string&);

    ServerBuilder& getServerBuilder() const;

    void setKind(ServiceKind);
    void setEntryPoint(const std::string&);
    void setDBEnv(const std::string&);
    void addDatabaseProperty(const std::string&, const std::string&);

    virtual std::string getDefaultAdapterId(const std::string&);

private:

    NodeInfoPtr _nodeInfo;

    ServerBuilder& _serverBuilder;

    ServiceKind _kind;
    Ice::PropertiesPtr _dbProperties;
};

}

#endif
