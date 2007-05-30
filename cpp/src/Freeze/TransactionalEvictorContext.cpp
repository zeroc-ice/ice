// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/TransactionalEvictorContext.h>
#include <Freeze/TransactionalEvictorI.h>
#include <Freeze/Initialize.h>
#include <Freeze/Util.h>
#include <Freeze/ConnectionI.h>
#include <IceUtil/IceUtil.h>
#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;


Freeze::TransactionalEvictorContext::TransactionalEvictorContext(const SharedDbEnvPtr& dbEnv) :
    _tx((new ConnectionI(dbEnv))->beginTransactionI()),
    _deadlockExceptionDetected(false)
{ 
    _tx->setPostCompletionCallback(this);
}

Freeze::TransactionalEvictorContext::TransactionalEvictorContext(const TransactionIPtr& tx) :
    _tx(tx),
    _deadlockExceptionDetected(false)
{ 
    _tx->setPostCompletionCallback(this);
}


Freeze::TransactionalEvictorContext::~TransactionalEvictorContext()
{
    for_each(_invalidateList.begin(), _invalidateList.end(), ToInvalidate::destroy);
}

void
Freeze::TransactionalEvictorContext::commit()
{
    if(_tx != 0)
    {
        _tx->commit();
    }
}

void
Freeze::TransactionalEvictorContext::rollback()
{
    if(_tx != 0)
    {
        _tx->rollback();
    }
}


void 
Freeze::TransactionalEvictorContext::postCompletion(bool committed, bool deadlock)
{
    try
    {
        if(committed)
        {
            //
            // remove updated & removed objects from cache
            //
            for_each(_invalidateList.begin(), _invalidateList.end(), ToInvalidate::invalidate);
            _invalidateList.clear();
        }
        finalize(deadlock);
    }
    catch(...)
    {
        finalize(deadlock);
        throw;
    }
}

Freeze::TransactionalEvictorContext::ServantHolder*
Freeze::TransactionalEvictorContext::findServantHolder(const Identity& ident, ObjectStore<TransactionalEvictorElement>* store) const
{
    for(Stack::const_iterator p = _stack.begin(); p != _stack.end(); ++p)
    {
        ServantHolder* sh = *p;
        if(sh->matches(ident, store))
        {
            return sh;
        }
    }
    return 0;
}

void
Freeze::TransactionalEvictorContext::finalize(bool deadlock)
{
    Lock sync(*this);
    if(_tx != 0)
    {
        if(deadlock)
        {
            _deadlockExceptionDetected = true;
        }
        _tx = 0;
        notifyAll();
    }
}

void
Freeze::TransactionalEvictorContext::checkDeadlockException()
{
    if(_deadlockException.get() != 0)
    {
        _deadlockException->ice_throw();
    }
}

bool 
Freeze::TransactionalEvictorContext::response(bool)
{
    if(_owner == IceUtil::ThreadControl())
    {
        return true;
    }
    else
    {
        Lock sync(*this);
        while(_deadlockExceptionDetected == false && _tx != 0)
        {
            wait();
        }
        return !_deadlockExceptionDetected;
    }
}

bool 
Freeze::TransactionalEvictorContext::exception(const std::exception& ex)
{
    const DeadlockException* dx = dynamic_cast<const DeadlockException*>(&ex);

    if(dx != 0 && _owner == IceUtil::ThreadControl())
    {
        _deadlockException.reset(dynamic_cast<DeadlockException*>(dx->ice_clone()));
        return false;
    }
    return true;
}

bool 
Freeze::TransactionalEvictorContext::exception()
{
    return true;
}

Ice::ObjectPtr
Freeze::TransactionalEvictorContext::servantRemoved(const Identity& ident, 
                                                     ObjectStore<TransactionalEvictorElement>* store)
{
    if(_tx != 0)
    {
        //
        // Lookup servant holder on stack
        //
        ServantHolder* sh = findServantHolder(ident, store);
        if(sh != 0)
        {
            sh->removed();
            return sh->servant();
        }
        else
        {
            _invalidateList.push_back(new ToInvalidate(ident, store));
            return 0;
        }
    }
    return 0;

}

void 
Freeze::TransactionalEvictorContext::deadlockException()
{
    {
        Lock sync(*this);
        _deadlockExceptionDetected = true;
        notifyAll();
    }
    rollback();
}


//
// ServantHolder
//

Freeze::TransactionalEvictorContext::ServantHolder::ServantHolder(const TransactionalEvictorContextPtr& ctx, 
                                                                   const Current& current, 
                                                                   ObjectStore<TransactionalEvictorElement>* store,
                                                                   bool useNonmutating) :
    _readOnly(false),
    _ownServant(false),
    _removed(false),
    _ctx(ctx),
    _current(current),
    _store(store)
{
    ServantHolder* sh = ctx->findServantHolder(_current.id, _store);

    if(sh != 0)
    {
        if(!sh->_removed)
        {
            _rec = sh->_rec;
        }
    }
    else
    {
        //
        // Let's load this servant
        //
        if(store->load(current.id, ctx->_tx, _rec))
        {
            _ctx->_stack.push_front(this);
            _ownServant = true;
       
            //
            // Compute readonly properly
            //
            _readOnly = (useNonmutating && current.mode == Nonmutating) ||
                (!useNonmutating && (_rec.servant->ice_operationAttributes(current.operation) & 0x1) == 0);
        }
    }
}


Freeze::TransactionalEvictorContext::ServantHolder::~ServantHolder()
{
    if(_ownServant)
    {
        if(_ctx->_tx != 0)
        {
            if(!_readOnly && !_removed)
            {
                EvictorIBase::updateStats(_rec.stats, IceUtil::Time::now().toMilliSeconds());
                _store->update(_current.id, _rec, _ctx->_tx);
            }
        
            if(!_readOnly || _removed)
            {
                _ctx->_invalidateList.push_back(new ToInvalidate(_current.id, _store));
            }
        }
        _ctx->_stack.pop_front();
    }
}

void
Freeze::TransactionalEvictorContext::ServantHolder::removed()
{
    _removed = true;
}

//
// ToInvalidate
//

//
// When constructed in the servant holder destructor, it's protected by the dispatch() 
// deactivate controller guard
//
Freeze::TransactionalEvictorContext::ToInvalidate::ToInvalidate(const Identity& ident, 
                                                                ObjectStore<TransactionalEvictorElement>* store) :
    _ident(ident),
    _store(store),
    _evictor(store->evictor()),
    _guard(_evictor->deactivateController())
{
}

void
Freeze::TransactionalEvictorContext::ToInvalidate::invalidate(ToInvalidate* obj)
{
    dynamic_cast<TransactionalEvictorI*>(obj->_store->evictor())->evict(obj->_ident, obj->_store);
    delete obj;
}

void
Freeze::TransactionalEvictorContext::ToInvalidate::destroy(ToInvalidate* obj)
{
    delete obj;
}
