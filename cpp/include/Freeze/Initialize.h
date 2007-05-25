// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Freeze/Transaction.h>

//
// Berkeley DB's DbEnv and DbTxn
//
class DbEnv;
class DbTxn;

namespace Freeze
{

typedef std::map<std::string, std::string> FacetTypeMap;

FREEZE_API Freeze::BackgroundSaveEvictorPtr 
createBackgroundSaveEvictor(const Ice::ObjectAdapterPtr& adapter,
                            const std::string& envName, 
                            const std::string& filename,
                            const ServantInitializerPtr& initializer = 0,
                            const std::vector<IndexPtr>& indices = std::vector<IndexPtr>(),
                            bool createDb = true);

FREEZE_API BackgroundSaveEvictorPtr
createBackgroundSaveEvictor(const Ice::ObjectAdapterPtr& adapter,
                            const std::string& envName,
                            DbEnv& dbEnv, 
                            const std::string& filename,
                            const ServantInitializerPtr& initializer = 0,
                            const std::vector<IndexPtr>& indices = std::vector<IndexPtr>(),
                            bool createDb = true);

FREEZE_API TransactionalEvictorPtr 
createTransactionalEvictor(const Ice::ObjectAdapterPtr& adapter,
                           const std::string& envName, 
                           const std::string& filename,
#if defined(_MSC_VER) && (_MSC_VER < 1300)
                           const FacetTypeMap& facetTypes = FacetTypeMap(std::less<std::string>()),
#else
                           const FacetTypeMap& facetTypes = FacetTypeMap(),
#endif
                           const ServantInitializerPtr& initializer = 0,
                           const std::vector<IndexPtr>& indices = std::vector<IndexPtr>(),
                           bool createDb = true);

FREEZE_API TransactionalEvictorPtr 
createTransactionalEvictor(const Ice::ObjectAdapterPtr& adapter,
                           const std::string& envName,
                           DbEnv& dbEnv, 
                           const std::string& filename,
#if defined(_MSC_VER) && (_MSC_VER < 1300)
                           const FacetTypeMap& facetTypes = FacetTypeMap(std::less<std::string>()),
#else
                           const FacetTypeMap& facetTypes = FacetTypeMap(),
#endif                   
                           const ServantInitializerPtr& initializer = 0,
                           const std::vector<IndexPtr>& indices = std::vector<IndexPtr>(),
                           bool createDb = true);


FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
                                          const std::string& envName);

FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
                                          const std::string& envName, 
                                          DbEnv& dbEnv);

FREEZE_API const std::string& catalogName();

FREEZE_API DbTxn* getTxn(const TransactionPtr&);


typedef void (*FatalErrorCallback)(const BackgroundSaveEvictorPtr&, const Ice::CommunicatorPtr&);
FREEZE_API FatalErrorCallback registerFatalErrorCallback(FatalErrorCallback);

}

#endif
