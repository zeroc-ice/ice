// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

FREEZE_API EvictorPtr createEvictor(const Ice::ObjectAdapterPtr& adapter,
				    const std::string& envName, 
				    const std::string& filename,
				    const ServantInitializerPtr& initializer = 0,
				    const std::vector<Freeze::IndexPtr>& indices = std::vector<Freeze::IndexPtr>(),
				    bool createDb = true);

FREEZE_API EvictorPtr createEvictor(const Ice::ObjectAdapterPtr& adapter,
				    const std::string& envName,
				    DbEnv& dbEnv, 
				    const std::string& filename,
				    const ServantInitializerPtr& initializer = 0,
				    const std::vector<Freeze::IndexPtr>& indices = std::vector<Freeze::IndexPtr>(),
				    bool createDb = true);


FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
					  const std::string& envName);

FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
					  const std::string& envName, 
					  DbEnv& dbEnv);


typedef void (*FatalErrorCallback)(const EvictorPtr&, const Ice::CommunicatorPtr&);
FREEZE_API FatalErrorCallback registerFatalErrorCallback(FatalErrorCallback);

}

#endif
