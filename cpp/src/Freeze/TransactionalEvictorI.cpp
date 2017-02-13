// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/StringUtil.h>

#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;

namespace
{

//
// Must be in sync with Parser.cpp
//

const int supports = 0;
const int mandatory = 1;
const int required = 2;
const int never = 3;
}


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

    class DispatchInterceptorAdapter : public Ice::DispatchInterceptor
    {
    public:

        DispatchInterceptorAdapter(const TransactionalEvictorIPtr& evictor) :
            _evictor(evictor)
        {
        }

        virtual DispatchStatus dispatch(Request& request)
        {
            return _evictor->dispatch(request);
        }

    private:

        TransactionalEvictorIPtr _evictor;
    };

    _interceptor = new DispatchInterceptorAdapter(this);

    string propertyPrefix = string("Freeze.Evictor.") + envName + '.' + _filename;

    _rollbackOnUserException = _communicator->getProperties()->
        getPropertyAsIntWithDefault(propertyPrefix + ".RollbackOnUserException", 0) > 0;
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


ObjectPrx
Freeze::TransactionalEvictorI::addFacet(const ObjectPtr& servant, const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    checkServant(servant);
    DeactivateController::Guard deactivateGuard(_deactivateController);

    ObjectStore<TransactionalEvictorElement>* store = findStore(facet, _createDb);
    if(store == 0)
    {
        throw NotFoundException(__FILE__, __LINE__, "addFacet: could not open database for facet '"
                                + facet + "'");
    }

    Ice::Long currentTime = 0;

    if(store->keepStats())
    {
        currentTime = IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds();
    }

    Statistics stats = { currentTime };
    ObjectRecord rec = { servant, stats };

    TransactionIPtr tx = beforeQuery();

    if(store->keepStats())
    {
        updateStats(rec.stats, currentTime);
    }

    if(!store->insert(ident, rec, tx))
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _communicator->identityToString(ident);
        if(!facet.empty())
        {
            ex.id += " -f " + IceUtilInternal::escapeString(facet, "");
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

ObjectPtr
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
            ex.id += " -f " + IceUtilInternal::escapeString(facet, "");
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

DispatchStatus
Freeze::TransactionalEvictorI::dispatch(Request& request)
{
    class CtxHolder
    {
    public:

#ifndef NDEBUG
        CtxHolder(bool ownCtx, const TransactionalEvictorContextPtr& ctx, const SharedDbEnvPtr& dbEnv) :
            _ownCtx(ownCtx),
            _ctx(ctx),
            _dbEnv(dbEnv)
#else
        CtxHolder(bool ownCtx, const TransactionalEvictorContextPtr& ctx) :
            _ownCtx(ownCtx),
            _ctx(ctx)
#endif
        {
        }

        ~CtxHolder() ICE_NOEXCEPT_FALSE
        {
            if(_ownCtx)
            {
                try
                {
                    _ctx->commit();
                }
                catch(...)
                {
                    assert(_dbEnv->getCurrent() == 0);
                    throw;
                }
                assert(_dbEnv->getCurrent() == 0);
            }
        }

    private:
        const bool _ownCtx;
        const TransactionalEvictorContextPtr _ctx;
#ifndef NDEBUG
        const SharedDbEnvPtr& _dbEnv;
#endif
    };


    DeactivateController::Guard deactivateGuard(_deactivateController);

    const Current& current = request.getCurrent();

    ObjectStore<TransactionalEvictorElement>* store = findStore(current.facet, false);
    if(store == 0)
    {
        servantNotFound(__FILE__, __LINE__, current);
    }

    TransactionalEvictorContextPtr ctx = _dbEnv->getCurrent();

    ObjectPtr sample = store->sampleServant();
    ObjectPtr cachedServant = 0;

    TransactionalEvictorContext::ServantHolder servantHolder;

    if(sample == 0)
    {
        if(ctx != 0)
        {
            try
            {
                servantHolder.init(ctx, current, store);
            }
            catch(const DeadlockException& dx)
            {
                assert(dx.tx == ctx->transaction());
                ctx->deadlockException();
                throw TransactionalEvictorDeadlockException(__FILE__, __LINE__, dx.tx);
            }
            sample = servantHolder.servant();
        }
        else
        {
            //
            // find / load read-only servant
            //

            cachedServant = loadCachedServant(current.id, store);

            if(cachedServant == 0)
            {
                servantNotFound(__FILE__, __LINE__, current);
            }
            sample = cachedServant;
        }
    }

    assert(sample != 0);

    int operationAttributes = sample->ice_operationAttributes(current.operation);

    if(operationAttributes < 0)
    {
        throw OperationNotExistException(__FILE__, __LINE__);
    }

    bool readOnly = (operationAttributes & 0x1) == 0;

    int txMode = (operationAttributes & 0x6) >> 1;

    bool ownCtx = false;

    //
    // Establish the proper context
    //
    switch(txMode)
    {
        case never:
        {
            assert(readOnly);
            if(ctx != 0)
            {
                throw DatabaseException(__FILE__, __LINE__, "transaction rejected by 'never' metadata");
            }
            break;
        }
        case supports:
        {
            assert(readOnly);
            break;
        }
        case mandatory:
        {
            if(ctx == 0)
            {
                throw DatabaseException(__FILE__, __LINE__, "operation with a mandatory transaction");
            }
            break;
        }
        case required:
        {
            if(ctx == 0)
            {
                ownCtx = true;
            }
            break;
        }
        default:
        {
            assert(0);
            throw OperationNotExistException(__FILE__, __LINE__);
        }
    }

    if(ctx == 0 && !ownCtx)
    {
        //
        // Read-only dispatch
        //
        assert(readOnly);
        if(cachedServant == 0)
        {
            cachedServant = loadCachedServant(current.id, store);

            if(cachedServant == 0)
            {
                servantNotFound(__FILE__, __LINE__, current);
            }
        }
        return cachedServant->ice_dispatch(request);
    }
    else
    {
        //
        // Create a new transaction; retry on DeadlockException
        //

        bool tryAgain = false;

        do
        {
            TransactionPtr tx;

            try
            {
                if(ownCtx)
                {
                    ctx = _dbEnv->createCurrent();
                }

#ifndef NDEBUG
                CtxHolder ctxHolder(ownCtx, ctx, _dbEnv);
#else
                CtxHolder ctxHolder(ownCtx, ctx);
#endif
                tx = ctx->transaction();

                try
                {
                    TransactionalEvictorContext::ServantHolder sh;
                    if(servantHolder.initialized())
                    {
                        //
                        // Adopt it
                        //
                        sh.adopt(servantHolder);
                    }
                    else
                    {
                        sh.init(ctx, current, store);
                    }

                    if(sh.servant() == 0)
                    {
                        servantNotFound(__FILE__, __LINE__, current);
                    }

                    if(!readOnly)
                    {
                        sh.markReadWrite();
                    }

                    try
                    {
                        DispatchStatus dispatchStatus = sh.servant()->ice_dispatch(request, ctx);
                        if(dispatchStatus == DispatchUserException && _rollbackOnUserException)
                        {
                            ctx->rollback();
                        }
                        if(dispatchStatus == DispatchAsync)
                        {
                            //
                            // May throw DeadlockException or TransactionalEvictorDeadlockException
                            //
                            ctx->checkDeadlockException();

                            if(ctx->clearUserException() && _rollbackOnUserException)
                            {
                                ctx->rollback();
                            }
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
                catch(const DeadlockException& dx)
                {
                    if(dx.tx == tx)
                    {
                        ctx->deadlockException();
                    }
                    throw;
                }
                catch(...)
                {
                    if(ownCtx)
                    {
                        ctx->rollback();
                    }
                    throw;
                }

                //
                // commit occurs here (when ownCtx)
                //
            }
            catch(const DeadlockException& dx)
            {
                if(ownCtx && dx.tx == tx)
                {
                    tryAgain = true;
                }
                else
                {
                    throw TransactionalEvictorDeadlockException(__FILE__, __LINE__, dx.tx);
                }
            }
            catch(const TransactionalEvictorDeadlockException& dx)
            {
                if(ownCtx && dx.tx == tx)
                {
                    tryAgain = true;
                }
                else
                {
                    throw;
                }
            }
        } while(tryAgain);
    }

    //
    // Can't be reached
    //
    assert(0);
    throw OperationNotExistException(__FILE__, __LINE__);
}


void
Freeze::TransactionalEvictorI::deactivate(const string&)
{
    if(_deactivateController.deactivate())
    {
        {
            Lock sync(*this);

            //
            // Set the evictor size to zero, meaning that we will evict
            // everything possible.
            //
            _evictorSize = 0;
            evict();
        }

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
        evict(*_evictorList.rbegin());
    }
}


ObjectPtr
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

ObjectPtr
Freeze::TransactionalEvictorI::evict(const Identity& ident, ObjectStore<TransactionalEvictorElement>* store)
{
    //
    // Important: we can't wait for the DB (even indirectly) with 'this' locked
    //
    TransactionalEvictorElementPtr element = store->getIfPinned(ident, true);

    if(element != 0)
    {
        Lock sync(*this);
        if(!element->_stale)
        {
            evict(element);
            return element->servant();
        }
    }
    return 0;
}

void
Freeze::TransactionalEvictorI::evict(const TransactionalEvictorElementPtr& element)
{
    //
    // Must be called with this locked!
    //
    assert(!element->_stale);
    element->_stale = true;
    element->_store.unpin(element->_cachePosition);

    if(element->_inEvictor)
    {
        element->_inEvictor = false;
        _evictorList.erase(element->_evictPosition);
        _currentEvictorSize--;
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



