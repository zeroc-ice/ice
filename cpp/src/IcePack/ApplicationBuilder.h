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
#include <IcePack/Internal.h>

namespace IcePack
{

class ApplicationBuilder : public ComponentBuilder
{
public:

    ApplicationBuilder(const Ice::CommunicatorPtr&, const NodeRegistryPtr&, const std::vector<std::string>&);

    using ComponentBuilder::parse;
    void parse(const std::string&);

    void addServer(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&,
		   const std::string&);
    void setBaseDir(const std::string&);

private:
    
    NodeRegistryPtr _nodeRegistry;
};

}

#endif
