// **********************************************************************
//
// Copyright (c) 2002
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
