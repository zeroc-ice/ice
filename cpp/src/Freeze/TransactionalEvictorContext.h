// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_TRANSACTIONAL_EVICTOR_CONTEXT_H
#define FREEZE_TRANSACTIONAL_EVICTOR_CONTEXT_H

#include <Ice/Ice.h>
#include <Freeze/TransactionalEvictor.h>
#include <Freeze/EvictorStorage.h>
#include <Freeze/EvictorI.h>
#include <Freeze/Initialize.h>
#include <IceUtil/IceUtil.h>

namespace Freeze
{

template<class T> class ObjectStore;

class TransactionalEvictorElement;

class TransactionalEvictorContext : public Ice::DispatchInterceptorAsyncCallback, public PostCompletionCallback,
                                    public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    class ServantHolder
    {
    public:
        
        ServantHolder();
        ~ServantHolder() ICE_NOEXCEPT_FALSE;

        void init(const TransactionalEvictorContextPtr&, const Ice::Current&, ObjectStore<TransactionalEvictorElement>*);

        void adopt(ServantHolder&);

        void markReadWrite();

        bool initialized() const
        {
            return _ownBody && _body.ctx != 0;
        }

        const Ice::ObjectPtr& servant() const
        {
            return _body.rec.servant;
        }

        struct Body
        {
            Body();

            bool matches(const Ice::Identity& ident, ObjectStore<TransactionalEvictorElement>* s) const
            {
                return current->id == ident && store == s;
            }

            bool readOnly;
            bool removed;
            bool ownServant;

            const TransactionalEvictorContextPtr* ctx;
            const Ice::Current* current;
            ObjectStore<TransactionalEvictorElement>* store;
            ObjectRecord rec;
        };

    private:

        Body _body;
        bool _ownBody;
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


    TransactionalEvictorContext(const SharedDbEnvPtr&);
    TransactionalEvictorContext(const TransactionIPtr&);

    virtual ~TransactionalEvictorContext();

    virtual void postCompletion(bool, bool, const SharedDbEnvPtr&);

    virtual bool response(bool);
    virtual bool exception(const std::exception&);
    virtual bool exception();

    Ice::ObjectPtr servantRemoved(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*);

    void deadlockException();

    void checkDeadlockException();

    bool clearUserException();

    void commit();
    void rollback();

    const TransactionIPtr& transaction() const
    {
        return _tx;
    }
    
private:
    
    friend class ServantHolder;

    ServantHolder::Body* findServantHolderBody(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*) const;

    void finalize(bool);

    //
    // Stack of ServantHolder::Body*
    //
    typedef std::deque<ServantHolder::Body*> Stack;
    Stack _stack;
    
    //
    // List of objects to invalidate from the caches upon commit
    //
    std::list<ToInvalidate*> _invalidateList;

    TransactionIPtr _tx;
    IceUtil::ThreadControl _owner;

    IceUtil::UniquePtr<DeadlockException> _deadlockException;
    IceUtil::UniquePtr<TransactionalEvictorDeadlockException> _nestedCallDeadlockException;
  
    //
    // Protected by this
    //
    bool _deadlockExceptionDetected;

    //
    // Not protected (used only by dispatch thread)
    //
    bool _userExceptionDetected;
};

typedef IceUtil::Handle<TransactionalEvictorContext> TransactionalEvictorContextPtr;

}


#endif
