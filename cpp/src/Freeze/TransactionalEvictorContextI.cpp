// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/TransactionalEvictorContextI.h>
#include <Freeze/TransactionalEvictorI.h>
#include <Freeze/Initialize.h>
#include <Freeze/Util.h>
#include <Freeze/ConnectionI.h>
#include <IceUtil/IceUtil.h>
#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;


Freeze::TransactionalEvictorContextI::TransactionalEvictorContextI(const SharedDbEnvPtr& dbEnv) :
    _tx((new ConnectionI(dbEnv))->beginTransactionI()),
    _rollbackOnly(false),
    _dbEnv(dbEnv),
    _deadlockExceptionDetected(false)
{
}

Freeze::TransactionalEvictorContextI::~TransactionalEvictorContextI()
{
    for_each(_invalidateList.begin(), _invalidateList.end(), ToInvalidate::destroy);
}

void 
Freeze::TransactionalEvictorContextI::rollbackOnly()
{
    _rollbackOnly = true;
}

bool 
Freeze::TransactionalEvictorContextI::isRollbackOnly() const
{
    return _rollbackOnly;
}

void 
Freeze::TransactionalEvictorContextI::complete()
{
    try
    {
        if(_rollbackOnly)
        {
            _tx->rollback();
        }
        else
        {
            if(!_stack.empty())
            {
                Warning out(_dbEnv->getCommunicator()->getLogger());
                out << "Committing TransactionalEvictorContext on DbEnv '" <<
                    _dbEnv->getEnvName() << "' with " << _stack.size() << " unsaved objects.";
            }
            
            _tx->commit();

            //
            // Finally, remove updated & removed objects from cache
            //
            
            for_each(_invalidateList.begin(), _invalidateList.end(), ToInvalidate::invalidate);
            _invalidateList.clear();
        } 
    }
    catch(const DeadlockException&)
    {
        deadlockException();
        finalize();
        throw;
    }
    catch(...)
    {
        finalize();
        throw;
    }
    finalize();
}

Freeze::TransactionalEvictorContextI::ServantHolder*
Freeze::TransactionalEvictorContextI::findServantHolder(const Identity& ident, ObjectStore<TransactionalEvictorElement>* store) const
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
Freeze::TransactionalEvictorContextI::finalize()
{
    if(_dbEnv != 0)
    {
        _dbEnv->clearCurrent(this);

        Lock sync(*this);
        _dbEnv = 0;
        notifyAll();
    }
}

void
Freeze::TransactionalEvictorContextI::checkDeadlockException()
{
    if(_deadlockException.get() != 0)
    {
        _deadlockException->ice_throw();
    }
}

bool 
Freeze::TransactionalEvictorContextI::response(bool)
{
    if(_owner == IceUtil::ThreadControl())
    {
        return true;
    }
    else
    {
        Lock sync(*this);
        while(_deadlockExceptionDetected == false && _dbEnv != 0)
        {
            wait();
        }
        return !_deadlockExceptionDetected;
    }
}

bool 
Freeze::TransactionalEvictorContextI::exception(const std::exception& ex)
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
Freeze::TransactionalEvictorContextI::exception()
{
    return true;
}

Ice::ObjectPtr
Freeze::TransactionalEvictorContextI::servantRemoved(const Identity& ident, 
                                                     ObjectStore<TransactionalEvictorElement>* store)
{
    if(!_rollbackOnly)
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
Freeze::TransactionalEvictorContextI::deadlockException()
{
    _rollbackOnly = true;

    Lock sync(*this);
    _deadlockExceptionDetected = true;
    notifyAll();
}


//
// ServantHolder
//

Freeze::TransactionalEvictorContextI::ServantHolder::ServantHolder(const TransactionalEvictorContextIPtr& ctx, 
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


Freeze::TransactionalEvictorContextI::ServantHolder::~ServantHolder()
{
    if(_ownServant)
    {
        if(!_ctx->isRollbackOnly())
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
Freeze::TransactionalEvictorContextI::ServantHolder::removed()
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
Freeze::TransactionalEvictorContextI::ToInvalidate::ToInvalidate(const Identity& ident, 
                                                                 ObjectStore<TransactionalEvictorElement>* store) :
    _ident(ident),
    _store(store),
    _evictor(store->evictor()),
    _guard(_evictor->deactivateController())
{
}

void
Freeze::TransactionalEvictorContextI::ToInvalidate::invalidate(ToInvalidate* obj)
{
    dynamic_cast<TransactionalEvictorI*>(obj->_store->evictor())->evict(obj->_ident, obj->_store);
    delete obj;
}

void
Freeze::TransactionalEvictorContextI::ToInvalidate::destroy(ToInvalidate* obj)
{
    delete obj;
}
