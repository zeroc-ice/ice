// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_TRANSACTIONAL_EVICTOR_CONTEXT_I_H
#define FREEZE_TRANSACTIONAL_EVICTOR_CONTEXT_I_H

#include <Ice/Ice.h>
#include <Freeze/TransactionalEvictor.h>
#include <Freeze/EvictorStorage.h>
#include <Freeze/EvictorI.h>
#include <IceUtil/IceUtil.h>

namespace Freeze
{

template<class T> class ObjectStore;

class TransactionalEvictorElement;

class TransactionalEvictorContextI : public TransactionalEvictorContext, public Ice::DispatchInterceptorAsyncCallback,
                                     public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    class ServantHolder
    {
    public:
        
        ServantHolder(const TransactionalEvictorContextIPtr&, const Ice::Current&, ObjectStore<TransactionalEvictorElement>*, bool);
        ~ServantHolder();

        void removed();

        const Ice::ObjectPtr& servant() const
        {
            return _rec.servant;
        }

        bool matches(const Ice::Identity& ident, ObjectStore<TransactionalEvictorElement>* store)
        {
            return _current.id == ident && _store == store;
        }

    private:
        
        bool _readOnly;
        bool _ownServant;
        bool _removed;

        const TransactionalEvictorContextIPtr& _ctx;
        const Ice::Current& _current;
        ObjectStore<TransactionalEvictorElement>* _store;
        ObjectRecord _rec;
    };


    class ToInvalidate
    {
    public:

        ToInvalidate(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*);
        
        static void invalidate(ToInvalidate*);
        static void destroy(ToInvalidate*);

    private:

        const Ice::Identity _ident;
        ObjectStore<TransactionalEvictorElement>* _store;
        EvictorIBasePtr _evictor; // for _guard
        DeactivateController::Guard _guard; // ensures store is not dangling
    };


    TransactionalEvictorContextI(const SharedDbEnvPtr&);
    virtual ~TransactionalEvictorContextI();
    
    virtual void rollbackOnly();
    virtual bool isRollbackOnly() const;
    virtual void complete();

    virtual bool response(bool);
    virtual bool exception(const std::exception&);
    virtual bool exception();

    Ice::ObjectPtr servantRemoved(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*);

    void deadlockException();

    void checkDeadlockException();

    const TransactionIPtr& transaction() const
    {
        return _tx;
    }
    
private:
    
    friend class ServantHolder;

    ServantHolder* findServantHolder(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*) const;

    void finalize();

    //
    // Stack of ServantHolder*
    //
    typedef std::deque<ServantHolder*> Stack;
    Stack _stack;
    
    //
    // List of objects to invalidate from the caches upon commit
    //
    std::list<ToInvalidate*> _invalidateList;

    TransactionIPtr _tx;
    IceUtil::ThreadControl _owner;

    bool _rollbackOnly;

    std::auto_ptr<DeadlockException> _deadlockException;

    SharedDbEnvPtr _dbEnv;

    //
    // Protected by this
    //
    bool _deadlockExceptionDetected;

};

typedef IceUtil::Handle<TransactionalEvictorContextI> TransactionalEvictorContextIPtr;

}


#endif
