// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

    virtual std::string getDefaultAdapterId(const std::string&);

private:

    NodeInfoPtr _nodeInfo;

    ServerBuilder& _serverBuilder;

    ServiceKind _kind;
};

}

#endif
