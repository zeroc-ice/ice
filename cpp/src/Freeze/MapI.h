// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef FREEZE_MAP_I_H
#define FREEZE_MAP_I_H

#include <Freeze/Map.h>
#include <Freeze/SharedDb.h>

namespace Freeze
{

class DBMapHelperI;


class DBIteratorHelperI : public DBIteratorHelper
{  
public:

    DBIteratorHelperI(const DBMapHelperI& m, bool readOnly);
    
    DBIteratorHelperI(const DBIteratorHelperI&);

    virtual 
    ~DBIteratorHelperI();

    bool
    findFirst() const;

    bool 
    find(const Key& k) const;

    virtual DBIteratorHelper*
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
    equals(const DBIteratorHelper&) const;

    void
    close();

    class Tx : public IceUtil::SimpleShared
    {
    public:

	Tx(const DBMapHelperI&);
	~Tx();

	void dead();

	DbTxn* getTxn()
	{
	    return _txn;
	}

    private:
	const DBMapHelperI& _map;
	DbTxn* _txn;
	bool _dead;
    };

    typedef IceUtil::Handle<Tx> TxPtr;

    const TxPtr&
    tx() const;

private:

    void
    cleanup();


    const DBMapHelperI& _map;
    Dbc* _dbc;
    TxPtr _tx;

    mutable Key _key;
    mutable Value _value;
}; 


class DBMapHelperI : public DBMapHelper
{
public:
   
    DBMapHelperI(const ConnectionIPtr& connection, const std::string& dbName, 
		 bool createDb);

    virtual ~DBMapHelperI();

    virtual DBIteratorHelper*
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
    closeAllIteratorsExcept(const DBIteratorHelperI::TxPtr&) const;


    friend class DBIteratorHelperI;
    friend class DBIteratorHelperI::Tx;

    const ConnectionIPtr _connection;
    mutable std::list<DBIteratorHelperI*> _iteratorList;
    SharedDbPtr _db;
    const std::string _dbName;
    int _trace;    
};

inline const DBIteratorHelperI::TxPtr&
DBIteratorHelperI::tx() const
{
    return _tx;
}

}

#endif
