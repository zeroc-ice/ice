// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_TRANSACTIONI_H
#define FREEZE_TRANSACTIONI_H

#include <Ice/CommunicatorF.h>
#include <Freeze/Transaction.h>
#include <db_cxx.h>

namespace Freeze
{

class ConnectionI;
typedef IceUtil::Handle<ConnectionI> ConnectionIPtr;

struct SharedMutex;
typedef IceUtil::Handle<SharedMutex> SharedMutexPtr;

class SharedDbEnv;
typedef IceUtil::Handle<SharedDbEnv> SharedDbEnvPtr;

class PostCompletionCallback : public virtual IceUtil::Shared
{
public:

    virtual void postCompletion(bool, bool, const SharedDbEnvPtr&) = 0;
};
typedef IceUtil::Handle<PostCompletionCallback> PostCompletionCallbackPtr;

class TransactionI : public Transaction
{
public:

    virtual void commit();

    virtual void rollback();


    virtual ConnectionPtr getConnection() const;
    
    //
    // Custom refcounting implementation
    //
    virtual void __incRef();
    virtual void __decRef();
    virtual int __getRef() const;

    void rollbackInternal(bool);
    void setPostCompletionCallback(const PostCompletionCallbackPtr&);

    TransactionI(ConnectionI*);
    ~TransactionI();
    
    DbTxn*
    dbTxn() const
    {
        return _txn;
    }

private:

    friend class ConnectionI;
    
    int __getRefNoSync() const;
  
    void postCompletion(bool, bool);

    const Ice::CommunicatorPtr _communicator;
    ConnectionIPtr _connection;
    const Ice::Int _txTrace;
    const Ice::Int _warnRollback;
    DbTxn* _txn;
    PostCompletionCallbackPtr _postCompletionCallback;
    SharedMutexPtr _refCountMutex;
    int _refCount;
};

typedef IceUtil::Handle<TransactionI> TransactionIPtr;

}
#endif
