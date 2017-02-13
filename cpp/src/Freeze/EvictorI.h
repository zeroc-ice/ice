// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_EVICTOR_I_H
#define FREEZE_EVICTOR_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <Freeze/ObjectStore.h>
#include <Freeze/EvictorIteratorI.h>
#include <Freeze/SharedDbEnv.h>
#include <Freeze/Index.h>
#include <Freeze/DB.h>
#include <list>
#include <vector>
#include <deque>

class DbTxn;

namespace Freeze
{

class EvictorIBase;

//
// Helper class to prevent deactivation while the Evictor is in use
//
class DeactivateController : private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    //
    // Prevents deactivation; the constructor raises
    // EvictorDeactivatedException if _deactivated or _deactivating is true.
    //
    class Guard
    {
    public:
        Guard(const DeactivateController&);
        ~Guard();

    private:
        DeactivateController& _controller;
    };

    DeactivateController(EvictorIBase*);

    //
    // Used mostly in asserts
    //
    bool deactivated() const;

    //
    // Returns true if this thread is supposed to do the deactivation and
    // call deactivationComplete() once done.
    //
    bool deactivate();

    void deactivationComplete();

private:

    friend class Guard;

    EvictorIBase* _evictor;
    bool _deactivating;
    bool _deactivated;
    int _guardCount;
};


class EvictorIBase : public virtual Evictor, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual TransactionIPtr beforeQuery() = 0;

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();

    virtual Ice::ObjectPrx add(const Ice::ObjectPtr&, const Ice::Identity&);
    virtual Ice::ObjectPtr remove(const Ice::Identity&);

    virtual bool hasObject(const Ice::Identity&);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);

    DeactivateController& deactivateController();
    const Ice::CommunicatorPtr& communicator() const;
    const Ice::EncodingVersion& encoding() const;
    const SharedDbEnvPtr& dbEnv() const;
    const std::string& filename() const;

    bool deadlockWarning() const;
    Ice::Int trace() const;
    Ice::Int txTrace() const;

    void initialize(const Ice::Identity&, const std::string&, const Ice::ObjectPtr&);


    static void updateStats(Statistics&, IceUtil::Int64);

    static std::string defaultDb;
    static std::string indexPrefix;

protected:

    EvictorIBase(const Ice::ObjectAdapterPtr&, const std::string&, DbEnv*, const std::string&,
             const FacetTypeMap&, const ServantInitializerPtr&, bool);


    virtual bool hasAnotherFacet(const Ice::Identity&, const std::string&) = 0;

    virtual Ice::ObjectPtr locateImpl(const Ice::Current&, Ice::LocalObjectPtr&) = 0;

    virtual void evict() = 0;

    std::vector<std::string> allDbs() const;

    size_t _evictorSize;

    FacetTypeMap _facetTypes;

    DeactivateController _deactivateController;

    Ice::ObjectAdapterPtr _adapter;
    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;

    ServantInitializerPtr _initializer;

    SharedDbEnvPtr _dbEnv;

    std::string _filename;
    bool _createDb;

    Ice::Int _trace;
    Ice::Int _txTrace;

    bool _deadlockWarning;

private:

    Ice::ObjectPtr _pingObject;
};

typedef IceUtil::Handle<EvictorIBase> EvictorIBasePtr;


template<class T>
class EvictorI : public EvictorIBase
{
public:

    virtual EvictorIteratorPtr
    getIterator(const std::string& facet, Ice::Int batchSize)
    {
        DeactivateController::Guard deactivateGuard(_deactivateController);

        TransactionIPtr tx = beforeQuery();
        return new EvictorIteratorI(findStore(facet, false), tx, batchSize);
    }

protected:

    EvictorI(const Ice::ObjectAdapterPtr& adapter, const std::string& envName, DbEnv* dbEnv,
             const std::string& filename, const FacetTypeMap& facetTypes,
             const ServantInitializerPtr& initializer, const std::vector<IndexPtr>& indices, bool createDb) :
        EvictorIBase(adapter, envName, dbEnv, filename, facetTypes, initializer, createDb)
    {
        std::string propertyPrefix = std::string("Freeze.Evictor.") + envName + '.' + filename;
        bool populateEmptyIndices =
            (_communicator->getProperties()->
             getPropertyAsIntWithDefault(propertyPrefix + ".PopulateEmptyIndices", 0) != 0);

        //
        // Instantiate all Dbs in 2 steps:
        // (1) iterate over the indices and create ObjectStore with indices
        // (2) open ObjectStores without indices
        //
        std::vector<std::string> dbs = allDbs();

        //
        // Add default db in case it's not there
        //
        dbs.push_back(defaultDb);

        for(std::vector<IndexPtr>::const_iterator i = indices.begin(); i != indices.end(); ++i)
        {
            std::string facet = (*i)->facet();

            typename StoreMap::iterator q = _storeMap.find(facet);
            if(q == _storeMap.end())
            {
                //
                // New db
                //
                std::vector<IndexPtr> storeIndices;

                for(std::vector<IndexPtr>::const_iterator r = i; r != indices.end(); ++r)
                {
                    if((*r)->facet() == facet)
                    {
                        storeIndices.push_back(*r);
                    }
                }
                std::string facetType;
                FacetTypeMap::const_iterator ft = facetTypes.find(facet);
                if(ft != facetTypes.end())
                {
                    facetType = ft->second;
                }
                ObjectStore<T>* store = new ObjectStore<T>(facet, facetType,_createDb, this, storeIndices, populateEmptyIndices);
                _storeMap.insert(typename StoreMap::value_type(facet, store));
            }
        }

        for(std::vector<std::string>::iterator p = dbs.begin(); p != dbs.end(); ++p)
        {
            std::string facet = *p;
            if(facet == defaultDb)
            {
                facet = "";
            }
#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
            std::pair<typename StoreMap::iterator, bool> ir =
                _storeMap.insert(typename StoreMap::value_type(facet, static_cast<ObjectStore<T>*>(nullptr)));
#else
            std::pair<typename StoreMap::iterator, bool> ir =
                _storeMap.insert(typename StoreMap::value_type(facet, 0));
#endif

            if(ir.second)
            {
                std::string facetType;
                FacetTypeMap::const_iterator ft = facetTypes.find(facet);
                if(ft != facetTypes.end())
                {
                    facetType = ft->second;
                }

                ir.first->second = new ObjectStore<T>(facet, facetType, _createDb, this);
            }
        }
    }

    ObjectStore<T>*
    findStore(const std::string& facet, bool createIt)
    {
        Lock sync(*this);
        ObjectStore<T>* os = 0;

        typename StoreMap::const_iterator p = _storeMap.find(facet);
        if(p != _storeMap.end())
        {
            os = (*p).second;
        }
        else if(createIt)
        {
            std::string facetType;
            typename FacetTypeMap::const_iterator q = _facetTypes.find(facet);
            if(q != _facetTypes.end())
            {
                facetType = q->second;
            }
            os = new ObjectStore<T>(facet, facetType, true, this);
            _storeMap.insert(typename StoreMap::value_type(facet, os));
        }
        return os;
    }

    void
    closeDbEnv()
    {
        for(typename StoreMap::iterator p = _storeMap.begin(); p != _storeMap.end(); ++p)
        {
            delete (*p).second;
        }

        _dbEnv = 0;
        _initializer = 0;
    }

    typedef std::map<std::string, ObjectStore<T>*> StoreMap;
    StoreMap _storeMap;
};


inline DeactivateController&
EvictorIBase::deactivateController()
{
    return _deactivateController;
}

inline const Ice::CommunicatorPtr&
EvictorIBase::communicator() const
{
    return _communicator;
}

inline const Ice::EncodingVersion&
EvictorIBase::encoding() const
{
    return _encoding;
}

inline const SharedDbEnvPtr&
EvictorIBase::dbEnv() const
{
    return _dbEnv;
}

inline bool
EvictorIBase::deadlockWarning() const
{
    return _deadlockWarning;
}

inline Ice::Int
EvictorIBase::trace() const
{
    return _trace;
}

//
// Helper function
//
inline void
checkIdentity(const Ice::Identity& ident)
{
    if(ident.name.size() == 0)
    {
        throw Ice::IllegalIdentityException(__FILE__, __LINE__, ident);
    }
}

inline void
checkServant(const Ice::ObjectPtr& servant)
{
    if(servant == 0)
    {
        throw Ice::IllegalServantException(__FILE__, __LINE__, "cannot add null servant to Freeze Evictor");
    }
}

}

#endif
