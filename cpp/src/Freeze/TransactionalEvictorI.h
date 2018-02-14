// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_TRANSACTIONAL_EVICTOR_I_H
#define FREEZE_TRANSACTIONAL_EVICTOR_I_H

#include <Freeze/EvictorI.h>
#include <Freeze/TransactionalEvictor.h>
#include <Ice/DispatchInterceptor.h>

namespace Freeze
{

class TransactionalEvictorI;

class TransactionalEvictorElement;
typedef IceUtil::Handle<TransactionalEvictorElement> TransactionalEvictorElementPtr;

class TransactionalEvictorElement : public Ice::LocalObject
{
public:

    TransactionalEvictorElement(ObjectRecord&, ObjectStore<TransactionalEvictorElement>&);
    ~TransactionalEvictorElement();

    void init(ObjectStore<TransactionalEvictorElement>::Position);

    const Ice::ObjectPtr& servant() const
    {
        return _servant;
    }

    bool stale() const
    {
        return _stale;
    }

private:   
    
    friend class TransactionalEvictorI;

    const Ice::ObjectPtr _servant;

    //
    // Immutable
    //
    ObjectStore<TransactionalEvictorElement>& _store;

    //
    // Immutable once set
    //
    ObjectStore<TransactionalEvictorElement>::Position _cachePosition;

    //
    // Protected by TransactionalEvictorI
    //
    std::list<TransactionalEvictorElementPtr>::iterator _evictPosition;
    bool _stale;
    bool _inEvictor;
};


class TransactionalEvictorI : public TransactionalEvictor, public EvictorI<TransactionalEvictorElement>
{
public:

    TransactionalEvictorI(const Ice::ObjectAdapterPtr&, const std::string&, DbEnv*, const std::string&, 
                          const FacetTypeMap&, const ServantInitializerPtr&, const std::vector<IndexPtr>&, bool);
 
    virtual ~TransactionalEvictorI();

    virtual TransactionPtr getCurrentTransaction() const;
    virtual void setCurrentTransaction(const TransactionPtr&);

    virtual Ice::ObjectPrx addFacet(const Ice::ObjectPtr&, const Ice::Identity&, const std::string&);
    virtual Ice::ObjectPtr removeFacet(const Ice::Identity&, const std::string&);

    virtual bool hasFacet(const Ice::Identity&, const std::string&);

    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);  

    virtual TransactionIPtr beforeQuery();

    Ice::DispatchStatus dispatch(Ice::Request&);

    Ice::ObjectPtr evict(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*);

protected:
   
    virtual bool hasAnotherFacet(const Ice::Identity&, const std::string&);
    
    virtual Ice::ObjectPtr locateImpl(const Ice::Current&, Ice::LocalObjectPtr&);
   
    virtual void evict();

private:

    Ice::ObjectPtr loadCachedServant(const Ice::Identity&, ObjectStore<TransactionalEvictorElement>*);

    void evict(const TransactionalEvictorElementPtr&);
    void fixEvictPosition(const TransactionalEvictorElementPtr&);

    void servantNotFound(const char*, int, const Ice::Current&);

    //
    // The _evictorList contains a list of all objects we keep,
    // with the most recently used first.
    //
    std::list<TransactionalEvictorElementPtr> _evictorList;
    std::list<TransactionalEvictorElementPtr>::size_type _currentEvictorSize;

    bool _rollbackOnUserException;

    Ice::DispatchInterceptorPtr _interceptor;
};

typedef IceUtil::Handle<TransactionalEvictorI> TransactionalEvictorIPtr;

}

#endif
