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

#ifndef FREEZE_INITIALIZE_H
#define FREEZE_INITIALIZE_H

#include <Ice/Ice.h>
#include <Freeze/EvictorF.h>
#include <Freeze/ConnectionF.h>
#include <Freeze/Index.h>

//
// Berkeley DB's DbEnv
//
class DbEnv;

namespace Freeze
{

FREEZE_API EvictorPtr createEvictor(const Ice::CommunicatorPtr& communicator,
				    const std::string& envName, 
				    const std::string& dbName,
				    const std::vector<Freeze::IndexPtr>& indices = std::vector<Freeze::IndexPtr>(),
				    bool createDb = true);

FREEZE_API EvictorPtr createEvictor(const Ice::CommunicatorPtr& communicator,
				    const std::string& envName,
				    DbEnv& dbEnv, 
				    const std::string& dbName, 
				    const std::vector<Freeze::IndexPtr>& indices = std::vector<Freeze::IndexPtr>(),
				    bool createDb = true);


FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
					  const std::string& envName);

FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
					  const std::string& envName, 
					  DbEnv& dbEnv);

}

#endif
