// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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

    IteratorHelperI(const MapHelperI& m, bool readOnly);
    
    IteratorHelperI(const IteratorHelperI&);

    virtual 
    ~IteratorHelperI();

    bool
    findFirst() const;

    bool 
    find(const Key& k) const;

    virtual IteratorHelper*
    clone() const;
    
    const Key*
    get() const;

    virtual void
    get(const Key*&, const Value*&) const;
    
    virtual  void 
    set(const Value&);

    virtual void
    erase();

    virtual bool
    next() const;

    virtual bool
    equals(const IteratorHelper&) const;

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
    TxPtr _tx;

    mutable Key _key;
    mutable Value _value;
}; 


class MapHelperI : public MapHelper
{
public:
   
    MapHelperI(const ConnectionIPtr& connection, const std::string& dbName, 
		 bool createDb);

    virtual ~MapHelperI();

    virtual IteratorHelper*
    find(const Key&, bool) const;

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
 
    void
    close();

private:

    virtual void
    closeAllIteratorsExcept(const IteratorHelperI::TxPtr&) const;


    friend class IteratorHelperI;
    friend class IteratorHelperI::Tx;

    const ConnectionIPtr _connection;
    mutable std::list<IteratorHelperI*> _iteratorList;
    SharedDbPtr _db;
    const std::string _dbName;
    Ice::Int _trace;    
};

inline const IteratorHelperI::TxPtr&
IteratorHelperI::tx() const
{
    return _tx;
}

}

#endif
