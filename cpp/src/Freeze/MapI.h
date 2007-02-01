// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_MAP_I_H
#define FREEZE_MAP_I_H

#include <Freeze/Map.h>
#include <Freeze/SharedDb.h>

namespace Freeze
{

class MapHelperI;


class IteratorHelperI : public IteratorHelper
{  
public:

    IteratorHelperI(const MapHelperI& m, bool readOnly, 
                    const MapIndexBasePtr& index, bool onlyDups);
    IteratorHelperI(const IteratorHelperI&);

    virtual 
    ~IteratorHelperI();
  
    bool 
    find(const Key& k) const;

    bool 
    lowerBound(const Key& k) const;

    bool 
    upperBound(const Key& k) const;

    virtual IteratorHelper*
    clone() const;
    
    virtual const Key*
    get() const;

    virtual void
    get(const Key*&, const Value*&) const;
    
    virtual  void 
    set(const Value&);

    virtual void
    erase();

    virtual bool
    next() const;

    bool next(bool) const;

    void
    close();

    class Tx : public IceUtil::SimpleShared
    {
    public:

        Tx(const MapHelperI&);
        ~Tx();

        void dead();

        DbTxn* getTxn()
        {
            return _txn;
        }

    private:
        const MapHelperI& _map;
        DbTxn* _txn;
        bool _dead;
    };

    typedef IceUtil::Handle<Tx> TxPtr;

    const TxPtr&
    tx() const;

private:

    void
    cleanup();

    const MapHelperI& _map;
    Dbc* _dbc;
    const bool _indexed;
    const bool _onlyDups;
    TxPtr _tx;

    mutable Key _key;
    mutable Value _value;
}; 


class MapHelperI : public MapHelper
{
public:
   
    MapHelperI(const ConnectionIPtr&, const std::string&, 
               const std::string&, const std::string&,
               const KeyCompareBasePtr&,
               const std::vector<MapIndexBasePtr>&, bool);

    virtual ~MapHelperI();

    virtual IteratorHelper*
    find(const Key&, bool) const;

    virtual IteratorHelper*
    lowerBound(const Key&, bool) const;

    virtual IteratorHelper*
    upperBound(const Key&, bool) const;

    virtual void
    put(const Key&, const Value&);

    virtual size_t
    erase(const Key&);

    virtual size_t
    count(const Key&) const;
    
    virtual void
    clear();

    virtual void
    destroy();

    virtual size_t
    size() const;

    virtual void
    closeAllIterators();

    virtual const MapIndexBasePtr&
    index(const std::string&) const;
 
    void
    close();

    const ConnectionIPtr& connection() const
    {
        return _connection;
    }


    typedef std::map<std::string, MapIndexBasePtr> IndexMap;
    
private:

    virtual void
    closeAllIteratorsExcept(const IteratorHelperI::TxPtr&) const;

    friend class IteratorHelperI;
    friend class IteratorHelperI::Tx;

    const ConnectionIPtr _connection;
    mutable std::list<IteratorHelperI*> _iteratorList;
    SharedDbPtr _db;
    const std::string _dbName;
    IndexMap _indices;

    Ice::Int _trace;    
};


inline const IteratorHelperI::TxPtr&
IteratorHelperI::tx() const
{
    return _tx;
}

}

#endif
