// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
// Automatically link Freeze[D].lib with Visual C++
//
#if !defined(ICE_BUILDING_FREEZE) && defined(FREEZE_API_EXPORTS)
#   define ICE_BUILDING_FREEZE
#endif

#ifdef _MSC_VER
#   if !defined(ICE_BUILDING_FREEZE)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "FreezeD.lib")
#      else
#          pragma comment(lib, "Freeze.lib")
#      endif
#   endif
#endif


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
                           const FacetTypeMap& facetTypes = FacetTypeMap(),
                           const ServantInitializerPtr& initializer = 0,
                           const std::vector<IndexPtr>& indices = std::vector<IndexPtr>(),
                           bool createDb = true);

FREEZE_API TransactionalEvictorPtr 
createTransactionalEvictor(const Ice::ObjectAdapterPtr& adapter,
                           const std::string& envName,
                           DbEnv& dbEnv, 
                           const std::string& filename,
                           const FacetTypeMap& facetTypes = FacetTypeMap(),
                           const ServantInitializerPtr& initializer = 0,
                           const std::vector<IndexPtr>& indices = std::vector<IndexPtr>(),
                           bool createDb = true);

//
// TransactionalEvictorDeadlockException propagates through collocation-optimized calls
// The TransactionalEvictor catches and retries on this exception
//
class FREEZE_API TransactionalEvictorDeadlockException : public Ice::SystemException
{
public:
  
    TransactionalEvictorDeadlockException(const char*, int, const TransactionPtr& = 0);

    virtual ~TransactionalEvictorDeadlockException() throw();

    virtual std::string ice_name() const;
    virtual TransactionalEvictorDeadlockException* ice_clone() const;
    virtual void ice_throw() const;

    TransactionPtr tx;
};


FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
                                          const std::string& envName);

FREEZE_API ConnectionPtr createConnection(const Ice::CommunicatorPtr& communicator,
                                          const std::string& envName, 
                                          DbEnv& dbEnv);

FREEZE_API const std::string& catalogName();
FREEZE_API const std::string& catalogIndexListName();

FREEZE_API DbTxn* getTxn(const TransactionPtr&);


typedef void (*FatalErrorCallback)(const BackgroundSaveEvictorPtr&, const Ice::CommunicatorPtr&);
FREEZE_API FatalErrorCallback registerFatalErrorCallback(FatalErrorCallback);

}

#endif
