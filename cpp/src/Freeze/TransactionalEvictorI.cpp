// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/TransactionalEvictorI.h>
#include <Freeze/Initialize.h>
#include <Freeze/Util.h>
#include <Freeze/TransactionalEvictorContext.h>

#include <IceUtil/IceUtil.h>

#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;


//
// createEvictor functions
// 

Freeze::TransactionalEvictorPtr
Freeze::createTransactionalEvictor(const ObjectAdapterPtr& adapter, 
                                   const string& envName, 
                                   const string& filename,
                                   const FacetTypeMap& facetTypes,
                                   const ServantInitializerPtr& initializer,
                                   const vector<IndexPtr>& indices,
                                   bool createDb)
{
    return new TransactionalEvictorI(adapter, envName, 0, filename, facetTypes, initializer, indices, createDb);
}

TransactionalEvictorPtr
Freeze::createTransactionalEvictor(const ObjectAdapterPtr& adapter, 
                                   const string& envName, 
                                   DbEnv& dbEnv, 
                                   const string& filename,
                                   const FacetTypeMap& facetTypes,
                                   const ServantInitializerPtr& initializer,
                                   const vector<IndexPtr>& indices,
                                   bool createDb)
{
    return new TransactionalEvictorI(adapter, envName, &dbEnv, filename, facetTypes, initializer, indices, createDb);
}


//
// TransactionalEvictorI
//

Freeze::TransactionalEvictorI::TransactionalEvictorI(const ObjectAdapterPtr& adapter, 
                                                     const string& envName, 
                                                     DbEnv* dbEnv, 
                                                     const string& filename,
                                                     const FacetTypeMap& facetTypes,
                                                     const ServantInitializerPtr& initializer,
                                                     const vector<IndexPtr>& indices,
                                                     bool createDb) :
    EvictorI<TransactionalEvictorElement>(adapter, envName, dbEnv, filename, facetTypes, initializer, indices, createDb),
    _currentEvictorSize(0)
{
    
    class DipatchInterceptorAdapter : public Ice::DispatchInterceptor
    {
    public:

        DipatchInterceptorAdapter(const TransactionalEvictorIPtr& evictor) :
            _evictor(evictor)
        {
        }

        virtual Ice::DispatchStatus dispatch(Ice::Request& request)
        {
            return _evictor->dispatch(request);
        }

    private:
        
        TransactionalEvictorIPtr _evictor;
    };

    _interceptor = new DipatchInterceptorAdapter(this);

    string propertyPrefix = string("Freeze.Evictor.") + envName + '.' + _filename; 
    
    _rollbackOnUserException = _communicator->getProperties()->
        getPropertyAsIntWithDefault(propertyPrefix + ".RollbackOnUserException", 0) != 0;
}


TransactionPtr
Freeze::TransactionalEvictorI::getCurrentTransaction() const
{
    DeactivateController::Guard deactivateGuard(_deactivateController);
    
    TransactionalEvictorContextPtr ctx = _dbEnv->getCurrent();
    if(ctx == 0)
    {
        return 0;
    }
    else
    {
        return ctx->transaction();
    }
}

void
Freeze::TransactionalEvictorI::setCurrentTransaction(const TransactionPtr& tx)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);
    _dbEnv->setCurrentTransaction(tx);
}


Ice::ObjectPrx
Freeze::TransactionalEvictorI::addFacet(const ObjectPtr& servant, const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);
   
    Ice::Long currentTime = IceUtil::Time::now().toMilliSeconds();

    Statistics stats = { currentTime };
    ObjectRecord rec;
    rec.servant = servant;
    rec.stats = stats;
   
    ObjectStore<TransactionalEvictorElement>* store = findStore(facet, _createDb);
    
    if(store == 0)
    {
        throw NotFoundException(__FILE__, __LINE__, "addFacet: could not open database for facet '"
                                + facet + "'");
    }
     
    TransactionIPtr tx = beforeQuery();
        
    updateStats(rec.stats, currentTime);

    if(!store->insert(ident, rec, tx))
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _communicator->identityToString(ident);
        if(!facet.empty())
        {
            ex.id += " -f " + IceUtil::escapeString(facet, "");
        }
        throw ex;
    }
    
    ObjectPrx obj = _adapter->createProxy(ident);
    if(!facet.empty())
    {
        obj = obj->ice_facet(facet);
    }
    return obj;
}

Ice::ObjectPtr
Freeze::TransactionalEvictorI::removeFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);
   
    ObjectPtr servant = 0;
    bool removed = false;
    ObjectStore<TransactionalEvictorElement>* store = findStore(facet, false);
   
    if(store != 0)
    {
        TransactionalEvictorContextPtr ctx = _dbEnv->getCurrent();
        TransactionIPtr tx = 0;
        if(ctx != 0)
        {
            tx = ctx->transaction();
            if(tx == 0)
            {
                throw DatabaseException(__FILE__, __LINE__, "inactive transaction");
            }
        }
        
        removed = store->remove(ident, tx);
            
        if(removed)
        {
            if(ctx != 0)
            {
                //
                // Remove from cache when transaction commits
                //
                servant = ctx->servantRemoved(ident, store);
            }
            else
            {
                //
                // Remove from cache immediately
                //
                servant = evict(ident, store);
            }
        }
    }
    
    if(!removed)
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _communicator->identityToString(ident);
        if(!facet.empty())
        {
            ex.id += " -f " + IceUtil::escapeString(facet, "");
        }
        throw ex;
    }

    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.Evictor");
        out << "removed object \"" << _communicator->identityToString(ident) << "\"";
        if(!facet.empty())
        {
            out << " with facet \"" << facet << "\"";
        }
        out << " from Db \"" << _filename << "\"";
    }
    return servant;
}

bool
Freeze::TransactionalEvictorI::hasFacet(const Identity& ident, const string& facet)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);
    checkIdentity(ident);
    ObjectStore<TransactionalEvictorElement>* store = findStore(facet, false);

    if(store == 0)
    {
        return false;
    }

    TransactionIPtr tx = beforeQuery();
    
    if(tx == 0)
    {
        TransactionalEvictorElementPtr element = store->getIfPinned(ident);
        if(element != 0)
        {
            return true;
        }
        return store->dbHasObject(ident, 0);
    }
    else
    {
        return store->dbHasObject(ident, tx);
    }
}

bool
Freeze::TransactionalEvictorI::hasAnotherFacet(const Identity& ident, const string& facet)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);

    //
    // If the object exists in another store, throw FacetNotExistException 
    // instead of returning 0 (== ObjectNotExistException)
    // 
    StoreMap storeMapCopy;
    {
        Lock sync(*this);
        storeMapCopy = _storeMap;
    }       

    TransactionIPtr tx = beforeQuery();
        
    for(StoreMap::iterator p = storeMapCopy.begin(); p != storeMapCopy.end(); ++p)
    {
        //
        // Do not check again the given facet
        //
        if((*p).first != facet)
        { 
            ObjectStore<TransactionalEvictorElement>* store = (*p).second;
            
            if(tx == 0 && store->getIfPinned(ident) != 0)
            {
                return true;
            }
            
            if(store->dbHasObject(ident, tx))
            {
                return true;
            }
        }
    }
    
    return false;
}


ObjectPtr
Freeze::TransactionalEvictorI::locateImpl(const Current&, LocalObjectPtr&)
{
    return _interceptor;
}

void
Freeze::TransactionalEvictorI::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    //
    // Nothing to do
    //
}

Ice::DispatchStatus
Freeze::TransactionalEvictorI::dispatch(Ice::Request& request)
{
    class CtxHolder
    {
    public:

        CtxHolder(const TransactionalEvictorContextPtr& ctx, const SharedDbEnvPtr& dbEnv) :
            _ctx(ctx),
            _dbEnv(dbEnv)
        {
        }
        
        ~CtxHolder()
        {
            try
            {
                _ctx->commit();
            }
            catch(...)
            {
                _dbEnv->setCurrentTransaction(0);
                throw;
            }
            _dbEnv->setCurrentTransaction(0);
        }

    private:
        TransactionalEvictorContextPtr _ctx;
        const SharedDbEnvPtr& _dbEnv;
    };
    

    DeactivateController::Guard deactivateGuard(_deactivateController);

    const Current& current = request.getCurrent();

    ObjectStore<TransactionalEvictorElement>* store = findStore(current.facet, false);
    if(store == 0)
    {
        servantNotFound(__FILE__, __LINE__, current);
    }
    
    //
    // Is there an existing context?
    //
    TransactionalEvictorContextPtr ctx = _dbEnv->getCurrent();
    
    if(ctx != 0)
    {
        try
        {
            //
            // If yes, use this context; there is no retrying
            //
            TransactionalEvictorContext::ServantHolder servantHolder(ctx, current, store, _useNonmutating);

            if(servantHolder.servant() == 0)
            {
                servantNotFound(__FILE__, __LINE__, current);
            }
            
            try
            {
                DispatchStatus dispatchStatus = servantHolder.servant()->ice_dispatch(request, ctx);
                
                if(dispatchStatus == DispatchUserException && _rollbackOnUserException)
                {
                    ctx->rollback();
                }

                if(dispatchStatus == DispatchAsync)
                {
                    //
                    // May throw DeadlockException
                    //
                    ctx->checkDeadlockException();
                }
                return dispatchStatus;
            }
            catch(...)
            {
                //
                // Important: this rollback() ensures that servant holder destructor won't perform
                // any database operation, and hence will not throw.
                //
                ctx->rollback();
                throw;
            }
            //
            // servantHolder destructor runs here and may throw (if tx was not rolled back)
            // 
        }
        catch(const DeadlockException&)
        {
            ctx->deadlockException();
            throw;
        }
        catch(...)
        {
            ctx->rollback();
            throw;
        }
    }
    else
    {
        ObjectPtr servant = 0;

        //
        // Otherwise, first figure out if it's a read or write operation
        //
        bool readOnly = true;
        
        if(_useNonmutating)
        {
            readOnly = (current.mode == Ice::Nonmutating);
        }
        else
        {
            //
            // Is there a sample-servant associated with this store?
            //
            
            ObjectPtr sample = store->sampleServant();
            if(sample != 0)
            {
                readOnly = (sample->ice_operationAttributes(current.operation) & 0x1) == 0;
            }
            else
            {
                //
                // Otherwise find / load read-only servant
                //
                servant = loadCachedServant(current.id, store);
                if(servant == 0)
                {
                    servantNotFound(__FILE__, __LINE__, current);
                }
                else
                {
                    readOnly = (servant->ice_operationAttributes(current.operation) & 0x1) == 0;
                }
            }
        }
        
        //
        // readOnly is now set properly
        //
        if(readOnly)
        {
            if(servant == 0)
            {
                servant = loadCachedServant(current.id, store);
                if(servant == 0)
                {
                    servantNotFound(__FILE__, __LINE__, current);
                }
            }
            // otherwise reuse servant loaded above
            
            //
            // Non-transactional, read-only dispatch
            //
            return servant->ice_dispatch(request);
        }
        else
        {
            //
            // Create a new transaction; retry on DeadlockException
            //
            
            bool tryAgain = false;
            
            do
            {
                try
                {
                    ctx = _dbEnv->createCurrent();
                    CtxHolder ctxHolder(ctx, _dbEnv);
                    
                    try
                    {                   
                        TransactionalEvictorContext::ServantHolder servantHolder(ctx, current, store, _useNonmutating);
                        
                        if(servantHolder.servant() == 0)
                        {
                            servantNotFound(__FILE__, __LINE__, current);
                        }
                               
                        try
                        {
                            DispatchStatus dispatchStatus = servantHolder.servant()->ice_dispatch(request, ctx);
                            if(dispatchStatus == DispatchUserException && _rollbackOnUserException)
                            {
                                ctx->rollback();
                            }
                            if(dispatchStatus == DispatchAsync)
                            {
                                //
                                // May throw DeadlockException
                                //
                                ctx->checkDeadlockException();
                            }

                            return dispatchStatus;
                        }
                        catch(...)
                        {
                            //
                            // Important: this rollback() ensures that servant holder destructor won't perform
                            // any database operation, and hence will not throw.
                            //
                            ctx->rollback();
                            throw;
                        }
                        //
                        // servant holder destructor runs here and may throw (if !rolled back)
                        // 
                    }
                    catch(const DeadlockException&)
                    {
                        ctx->deadlockException();
                        throw;
                    }
                    catch(...)
                    {
                        ctx->rollback();
                        throw;
                    }

                    //
                    // commit occurs here!
                    //
                }
                catch(const DeadlockException&)
                {
                    tryAgain = true;
                }
                
            } while(tryAgain);
        }
    }

    //
    // Can be reached
    //
    assert(0);
    throw OperationNotExistException(__FILE__, __LINE__);
}


void
Freeze::TransactionalEvictorI::deactivate(const string&)
{
    if(_deactivateController.deactivate())
    {
        //
        // Break cycle
        //
        _interceptor = 0;

        try
        {
            closeDbEnv();
        }
        catch(...)
        {
            _deactivateController.deactivationComplete();
            throw;
        }
        _deactivateController.deactivationComplete();
    }
}

Freeze::TransactionalEvictorI::~TransactionalEvictorI()
{
    //
    // Because of the cycle with _interceptor, this can only run if deactivated()
    //
}

Freeze::TransactionIPtr
Freeze::TransactionalEvictorI::beforeQuery()
{
    TransactionalEvictorContextPtr ctx = _dbEnv->getCurrent();
    TransactionIPtr tx = 0;
    if(ctx != 0)
    {
        tx = ctx->transaction();
        if(tx == 0)
        {
            throw DatabaseException(__FILE__, __LINE__,"inactive transaction");
        }
    }
    
    return tx;
}

void
Freeze::TransactionalEvictorI::evict()
{
    //
    // Must be called with this locked
    //

    while(_currentEvictorSize > _evictorSize)
    {
        //
        // Evict, no matter what!
        //
        evict(*_evictorList.rbegin(), true);
    }
}


Ice::ObjectPtr
Freeze::TransactionalEvictorI::loadCachedServant(const Identity& ident, ObjectStore<TransactionalEvictorElement>* store)
{
    for(;;)
    {
        TransactionalEvictorElementPtr element = store->pin(ident);
        
        if(element == 0)
        {
            return 0;
        }
        
        Lock sync(*this);
        if(element->stale())
        {
            //
            // try again
            //
            continue;
        }
        
        fixEvictPosition(element);
                
        //
        // if _evictorSize is 0, I may evict myself ... no big deal
        //
        evict();
        return element->servant();
    }
}

Ice::ObjectPtr
Freeze::TransactionalEvictorI::evict(const Identity& ident, ObjectStore<TransactionalEvictorElement>* store)
{
    Lock sync(*this);
    TransactionalEvictorElementPtr element = store->unpin(ident);

    if(element != 0)
    {
        evict(element, false);
        return element->servant();
    }
    return 0;
}

void 
Freeze::TransactionalEvictorI::evict(const TransactionalEvictorElementPtr& element, bool unpin)
{
    //
    // Must be called with this locked!
    //
    assert(!element->_stale);
    element->_stale = true;

    if(unpin)
    {
        element->_store.unpin(element->_cachePosition);
    }

    if(element->_inEvictor)
    {
        element->_inEvictor = false;
        _evictorList.erase(element->_evictPosition);
        _currentEvictorSize--;
    }
    else
    {
        //
        // This object was removed before it had time to make it into
        // the evictor
        //
        assert(!unpin);
    }
}

void 
Freeze::TransactionalEvictorI::fixEvictPosition(const TransactionalEvictorElementPtr& element)
{
    //
    // Must be called with this locked!
    //

    assert(!element->_stale);
    
    if(element->_inEvictor)
    {
        _evictorList.erase(element->_evictPosition);
    }
    else
    {
        //
        // New object
        //
        _currentEvictorSize++;
        element->_inEvictor = true;
    }
    _evictorList.push_front(element);
    element->_evictPosition = _evictorList.begin();
}


void
Freeze::TransactionalEvictorI::servantNotFound(const char* file, int line, const Current& current)
{
    if(_trace >= 2)
    {
        Trace out(_communicator->getLogger(), "Freeze.Evictor");
        out << "could not find \"" << _communicator->identityToString(current.id) 
            << "\" with facet \"" <<  current.facet + "\""; 
    }
    
    if(hasAnotherFacet(current.id, current.facet))
    {
        throw FacetNotExistException(file, line, current.id, current.facet, current.operation);
    }
    else
    {
        throw ObjectNotExistException(file, line, current.id, current.facet, current.operation);
    }
}



Freeze::TransactionalEvictorElement::TransactionalEvictorElement(ObjectRecord& r,
                                                                 ObjectStore<TransactionalEvictorElement>& s) :
    _servant(r.servant),
    _store(s),
    _stale(true),
    _inEvictor(false)
{
}


Freeze::TransactionalEvictorElement::~TransactionalEvictorElement()
{
}

void 
Freeze::TransactionalEvictorElement::init(ObjectStore<TransactionalEvictorElement>::Position p)
{
    _stale = false;
    _cachePosition = p;
}



