// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    void parse(const std::string&);

    void addServer(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&,
		   const std::string&);
    void setBaseDir(const std::string&);

private:
    
    NodeRegistryPtr _nodeRegistry;
};

}

#endif
