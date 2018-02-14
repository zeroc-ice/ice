// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

//
// TransactionalEvictorDeadlockException
//

Freeze::TransactionalEvictorDeadlockException::TransactionalEvictorDeadlockException(const char* file, int line, const TransactionPtr& transaction) :
    Ice::SystemException(file, line),
    tx(transaction)
{
}

Freeze::TransactionalEvictorDeadlockException::~TransactionalEvictorDeadlockException() throw()
{
}

string
Freeze::TransactionalEvictorDeadlockException::ice_name() const
{
    return "Freeze::TransactionalEvictorDeadlockException";
}

Freeze::TransactionalEvictorDeadlockException*
Freeze::TransactionalEvictorDeadlockException::ice_clone() const
{
    return new TransactionalEvictorDeadlockException(*this);
}

void
Freeze::TransactionalEvictorDeadlockException::ice_throw() const
{
    throw *this;
}

//
// TransactionalEvictorContext
//

Freeze::TransactionalEvictorContext::TransactionalEvictorContext(const SharedDbEnvPtr& dbEnv) :
    _tx((new ConnectionI(dbEnv))->beginTransactionI()),
    _deadlockExceptionDetected(false),
    _userExceptionDetected(false)
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
Freeze::TransactionalEvictorContext::postCompletion(bool committed, bool deadlock, const SharedDbEnvPtr& dbEnv)
{
    dbEnv->setCurrentTransaction(0);

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

Freeze::TransactionalEvictorContext::ServantHolder::Body*
Freeze::TransactionalEvictorContext::findServantHolderBody(const Identity& ident, ObjectStore<TransactionalEvictorElement>* store) const
{
    for(Stack::const_iterator p = _stack.begin(); p != _stack.end(); ++p)
    {
        ServantHolder::Body* b = *p;
        if(b->matches(ident, store))
        {
            return b;
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

    if(_nestedCallDeadlockException.get() != 0)
    {
        _nestedCallDeadlockException->ice_throw();
    }
}

bool
Freeze::TransactionalEvictorContext::clearUserException()
{
    bool result = _userExceptionDetected;
    _userExceptionDetected = false;
    return result;
}

bool 
Freeze::TransactionalEvictorContext::response(bool ok)
{
    if(_owner == IceUtil::ThreadControl())
    {
        if(!ok)
        {
            _userExceptionDetected = true;
        }
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
        _deadlockException.reset(dx->ice_clone());
        return false;
    }

    const TransactionalEvictorDeadlockException* edx = 
        dynamic_cast<const TransactionalEvictorDeadlockException*>(&ex);
    if(edx != 0 && _owner == IceUtil::ThreadControl())
    {
        _nestedCallDeadlockException.reset(edx->ice_clone());
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
        ServantHolder::Body* body = findServantHolderBody(ident, store);
        if(body != 0)
        {
            body->removed = true;
            return body->rec.servant;
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

Freeze::TransactionalEvictorContext::ServantHolder::ServantHolder() :
    _ownBody(true)
{
}


Freeze::TransactionalEvictorContext::ServantHolder::~ServantHolder() ICE_NOEXCEPT_FALSE
{
    if(_ownBody && _body.ownServant)
    {
        const TransactionalEvictorContextPtr& ctx = *(_body.ctx);

        if(ctx->_tx != 0)
        {
            if(!_body.readOnly && !_body.removed)
            {
                if(_body.store->keepStats())
                {
                    EvictorIBase::updateStats(_body.rec.stats, 
                                              IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds());
                }
                _body.store->update(_body.current->id, _body.rec, ctx->_tx);
            }
        
            if(!_body.readOnly || _body.removed)
            {
                ctx->_invalidateList.push_back(new ToInvalidate(_body.current->id, _body.store));
            }
        }
        ctx->_stack.pop_front();
    }
}


void
Freeze::TransactionalEvictorContext::ServantHolder::init(const TransactionalEvictorContextPtr& ctx, 
                                                         const Current& current, 
                                                         ObjectStore<TransactionalEvictorElement>* store)
{
    assert(_ownBody && _body.ctx == 0);
  
    _body.ctx = &ctx;
    _body.current = &current;
    _body.store = store;
    
    ServantHolder::Body* body = ctx->findServantHolderBody(current.id, store);

    if(body != 0)
    {
        if(!body->removed)
        {
            _body.rec = body->rec;
            _body.readOnly = body->readOnly;
        }
    }
    else
    {
        //
        // Let's load this servant
        //
        if(store->load(current.id, ctx->_tx, _body.rec))
        {
            ctx->_stack.push_front(&_body);
            _body.ownServant = true;
        }
    }
}

void
Freeze::TransactionalEvictorContext::ServantHolder::adopt(ServantHolder& other)
{
    assert(_ownBody && _body.ctx == 0);

    _body = other._body;
    other._ownBody = false;
}

void
Freeze::TransactionalEvictorContext::ServantHolder::markReadWrite()
{
    assert(_ownBody);

    if(_body.ownServant)
    {
        _body.readOnly = false;
    }
    else
    {
        if(_body.readOnly)
        {
            throw DatabaseException(__FILE__, __LINE__, "freeze:write operation called from freeze:read operation");
        }
    }
}

Freeze::TransactionalEvictorContext::ServantHolder::Body::Body() :
    readOnly(true),
    removed(false),
    ownServant(false),
    ctx(0),
    current(0),
    store(0)
{
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
