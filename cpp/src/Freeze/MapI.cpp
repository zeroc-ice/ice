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

#include <Freeze/MapI.h>
#include <Freeze/DBException.h>
#include <Freeze/SharedDb.h>
#include <stdlib.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

namespace
{

inline void 
initializeInDbt(const vector<Ice::Byte>& v, Dbt& dbt)
{
    dbt.set_data(const_cast<Ice::Byte*>(&v[0]));
    dbt.set_size(v.size());
    dbt.set_ulen(0);
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

inline void 
initializeOutDbt(vector<Ice::Byte>& v, Dbt& dbt)
{
    v.resize(v.capacity());
    dbt.set_data(&v[0]);
    dbt.set_size(0);
    dbt.set_ulen(v.size());
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

}


//
// DBMapHelper (from Map.h)
//

Freeze::DBMapHelper*
Freeze::DBMapHelper::create(const Freeze::ConnectionPtr& connection, 
			    const string& dbName, 
			    bool createDb)
{
    Freeze::ConnectionIPtr connectionI = Freeze::ConnectionIPtr::dynamicCast(connection);
    return new DBMapHelperI(connectionI, dbName, createDb);
}

Freeze::DBMapHelper::~DBMapHelper()
{
}


//
// DBIteratorHelper (from Map.h)
//

Freeze::DBIteratorHelper* 
Freeze::DBIteratorHelper::create(const DBMapHelper& m, bool readOnly)
{
    const DBMapHelperI& actualMap = dynamic_cast<const DBMapHelperI&>(m);

    auto_ptr<DBIteratorHelperI> r(new DBIteratorHelperI(actualMap, readOnly));
    if(r->findFirst())
    {
	return r.release();
    }
    else
    {
	return 0;
    }
}


Freeze::DBIteratorHelper::~DBIteratorHelper()
{
}



//
// DBIteratorHelperI
//


Freeze::DBIteratorHelperI::DBIteratorHelperI(const DBMapHelperI& m, bool readOnly) :
    _map(m),
    _dbc(0),
    _tx(0)
{
    if(_map._trace >= 3)
    {
	Trace out(_map._connection->communicator()->getLogger(), "DB");
	out << "opening iterator on database \"" << _map._dbName << "\"";
    }

    DbTxn* txn = _map._connection->dbTxn();
    
    if(txn == 0 && !readOnly)
    {
	//
	// Need to start a transaction
	//
	_tx = new Tx(_map);
	txn = _tx->getTxn();
    }

    try
    {
	_map._db->cursor(txn, &_dbc, 0);
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    _map._iteratorList.push_back(this);
}

Freeze::DBIteratorHelperI::DBIteratorHelperI(const DBIteratorHelperI& it) :
    _map(it._map),
    _dbc(0),
    _tx(0)
{
    if(_map._trace >= 3)
    {
	Trace out(_map._connection->communicator()->getLogger(), "DB");
	out << "duplicating iterator on database \"" << _map._dbName << "\"";
    }

    try
    {
	it._dbc->dup(&_dbc, DB_POSITION);
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
   
    _tx = it._tx;
    _map._iteratorList.push_back(this);
}

Freeze::DBIteratorHelperI::~DBIteratorHelperI()
{
    close();
}

bool 
Freeze::DBIteratorHelperI::findFirst() const
{
    return next();
}

bool 
Freeze::DBIteratorHelperI::find(const Key& key) const
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);

    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    try
    {
	if(_dbc->get(&dbKey, &dbValue, DB_SET) == 0)
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_tx != 0)
	{
	    _tx->dead();
	}

	DBDeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

Freeze::DBIteratorHelper*
Freeze::DBIteratorHelperI::clone() const
{
    return new DBIteratorHelperI(*this);
}
    
void
Freeze::DBIteratorHelperI::get(const Key*& key, const Value*& value) const
{
    key = &_key;
    value = &_value;

    size_t keySize = _key.capacity();
    if(keySize < 1024)
    {
	keySize = 1024;
    }
    _key.resize(keySize);

    Dbt dbKey;
    initializeOutDbt(_key, dbKey);
    
    size_t valueSize = _value.capacity();
    if(valueSize < 1024)
    {
	valueSize = 1024;
    }
    _value.resize(valueSize);
    Dbt dbValue;
    initializeOutDbt(_value, dbValue);

    for(;;)
    {
	try
	{
	    int err = _dbc->get(&dbKey, &dbValue, DB_CURRENT);

	    if(err == 0)
	    {
		_key.resize(dbKey.get_size());
		_value.resize(dbValue.get_size());
		break; // for(;;)
	    }
	    else if(err == DB_KEYEMPTY)
	    {
		throw DBInvalidPositionException(__FILE__, __LINE__);
	    }
	    else 
	    {
		//
		// Bug in Freeze
		//
		assert(0);
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbMemoryException dx)
	{
	    bool resizing = false;
	    if(dbKey.get_size() > dbKey.get_ulen())
	    {
		//
		// Let's resize key
		//
		_key.resize(dbKey.get_size());
		initializeOutDbt(_key, dbKey);
		resizing = true;
	    }
	    
	    if(dbValue.get_size() > dbValue.get_ulen())
	    {
		//
		// Let's resize value
		//
		_value.resize(dbValue.get_size());
		initializeOutDbt(_value, dbValue);
		resizing = true;
	    }
	    
	    if(!resizing)
	    {
		//
		// Real problem
		//
		DBException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(_tx != 0)
	    {
		_tx->dead();
	    }

	    DBDeadlockException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}
    
const Freeze::Key*
Freeze::DBIteratorHelperI::get() const
{
    size_t keySize = _key.capacity();
    if(keySize < 1024)
    {
	keySize = 1024;
    }
    _key.resize(keySize);

    Dbt dbKey;
    initializeOutDbt(_key, dbKey);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    
    for(;;)
    {
	try
	{
	    int err = _dbc->get(&dbKey, &dbValue, DB_CURRENT);

	    if(err == 0)
	    {
		_key.resize(dbKey.get_size());
		return &_key;
	    }
	    else if(err == DB_KEYEMPTY)
	    {
		throw DBInvalidPositionException(__FILE__, __LINE__);
	    }
	    else 
	    {
		//
		// Bug in Freeze
		//
		assert(0);
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbMemoryException dx)
	{
	    if(dbKey.get_size() > dbKey.get_ulen())
	    {
		//
		// Let's resize key
		//
		_key.resize(dbKey.get_size());
		initializeOutDbt(_key, dbKey);
	    }
	    else
	    {
		//
		// Real problem
		//
		DBException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(_tx != 0)
	    {
		_tx->dead();
	    }

	    DBDeadlockException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void 
Freeze::DBIteratorHelperI::set(const Value& value)
{
    //
    // key ignored
    //
    Dbt dbKey;
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    Dbt dbValue;
    initializeInDbt(value, dbValue);

    if(_tx != 0)
    {
	_map.closeAllIteratorsExcept(_tx);
    }

    try
    {
	int err = _dbc->put(&dbKey, &dbValue, DB_CURRENT);
	assert(err == 0);
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_tx != 0)
	{
	    _tx->dead();
	}

	DBDeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

void
Freeze::DBIteratorHelperI::erase()
{
    if(_tx != 0)
    {
	_map.closeAllIteratorsExcept(_tx);
    }

    try
    {
	int err = _dbc->del(0);
	if(err == DB_KEYEMPTY)
	{
	    throw DBInvalidPositionException(__FILE__, __LINE__);
	}
	assert(err == 0);
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_tx != 0)
	{
	    _tx->dead();
	}

	DBDeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

bool
Freeze::DBIteratorHelperI::next() const
{
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbKey;
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    try
    {
	if(_dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_tx != 0)
	{
	    _tx->dead();
	}

	DBDeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

bool
Freeze::DBIteratorHelperI::equals(const DBIteratorHelper& rhs) const
{
    if(this == &rhs)
    {
	return true;
    }
    else
    {
	//
	// Compare keys
	//
	try
	{
	    Key rhsKey = *dynamic_cast<const DBIteratorHelperI&>(rhs).get();
	    return *get() == rhsKey;
	}
	catch(const DBInvalidPositionException&)
	{
	    return false;
	}
    }
}

void
Freeze::DBIteratorHelperI::close()
{
    if(_dbc != 0)
    {
	if(_map._trace >= 3)
	{
	    Trace out(_map._connection->communicator()->getLogger(), "DB");
	    out << "closing iterator on database \"" << _map._dbName << "\"";
	}
	
	try
	{
	    _dbc->close();
	}
	catch(const ::DbDeadlockException& dx)
	{
	    bool raiseException = (_tx == 0);
	    cleanup();
	    if(raiseException)
	    {
		DBDeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	}
	catch(const ::DbException& dx)
	{
	    cleanup();
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	cleanup();
    }
}

void
Freeze::DBIteratorHelperI::cleanup()
{
    _dbc = 0;
    _map._iteratorList.remove(this);
    _tx = 0;
}


//
// DBIteratorHelperI::Tx
//

Freeze::DBIteratorHelperI::Tx::Tx(const DBMapHelperI& m) :
    _map(m),
    _txn(0),
    _dead(false)
{
    if(_map._trace >= 3)
    {
	Trace out(_map._connection->communicator()->getLogger(), "DB");
	out << "starting transaction for database \"" << _map._dbName << "\"";
    }

    try
    {
	_map._connection->dbEnv()->txn_begin(0, &_txn, 0);
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}
	

Freeze::DBIteratorHelperI::Tx::~Tx()
{
    if(_dead)
    {
	if(_map._trace >= 3)
	{
	    Trace out(_map._connection->communicator()->getLogger(), "DB");
	    out << "aborting transaction for database \"" << _map._dbName << "\"";
	}

	try
	{
	    _txn->abort();
	}
	catch(...)
	{
	    //
	    // Ignore exceptions to avoid crash during stack unwinding
	    //
	}
    }
    else
    {
	if(_map._trace >= 3)
	{
	    Trace out(_map._connection->communicator()->getLogger(), "DB");
	    out << "committing transaction for database \"" << _map._dbName.c_str() << "\"";
	}

	try
	{
	    _txn->commit(0);
	}
	catch(const ::DbDeadlockException& dx)
	{
	    DBDeadlockException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void
Freeze::DBIteratorHelperI::Tx::dead()
{
    _dead = true;
}



//
// DBMapHelperI
//


Freeze::DBMapHelperI::DBMapHelperI(const ConnectionIPtr& connection, 
				   const std::string& dbName, 
				   bool createDb) :
    _connection(connection),
    _db(SharedDb::get(connection, dbName, createDb)),
    _dbName(dbName),
    _trace(connection->trace())
{ 
    _connection->registerMap(this);
}

Freeze::DBMapHelperI::~DBMapHelperI()
{
    close();
}

Freeze::DBIteratorHelper*
Freeze::DBMapHelperI::find(const Key& k, bool readOnly) const
{
    for(;;)
    {
	try
	{  
	    auto_ptr<DBIteratorHelperI> r(new DBIteratorHelperI(*this, readOnly));
	    if(r->find(k))
	    {
		return r.release();
	    }
	    else
	    {
		return 0;
	    }
	}
	catch(const DBDeadlockException&)
	{
	    if(_connection->dbTxn() != 0)
	    {
		throw;
	    }
	    else
	    {
		//
		// Ignored, try again
		//
	    }
	}
    }
}

void
Freeze::DBMapHelperI::put(const Key& key, const Value& value)
{
    Dbt dbKey;
    Dbt dbValue;
    initializeInDbt(key, dbKey);
    initializeInDbt(value, dbValue);
 
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
	closeAllIterators();
    }

    for(;;)
    {
	try
	{
	    int err = _db->put(txn, &dbKey, &dbValue, 
			       txn != 0 ? 0 : DB_AUTO_COMMIT);
	    
	    if(err == 0)
	    {
		break;
	    }
	    else
	    {
		//
		// Bug in Freeze
		//
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(txn != 0)
	    {
		DBDeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

size_t
Freeze::DBMapHelperI::erase(const Key& key)
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);

    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
	closeAllIterators();
    }

    for(;;)
    {
	try
	{
	    int err = _db->del(txn, &dbKey, txn != 0 ? 0 : DB_AUTO_COMMIT);

	    if(err == 0)
	    {
		return true;
	    }
	    else if(err == DB_NOTFOUND)
	    {
		return false;
	    }
	    else
	    {
		assert(0);
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(txn != 0)
	    {
		DBDeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

size_t
Freeze::DBMapHelperI::count(const Key& key) const
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    
    for(;;)
    {
	try
	{
	    int err = _db->get(_connection->dbTxn(), &dbKey, &dbValue, 0);
	    
	    if(err == 0)
	    {
		return 1;
	    }
	    else if(err == DB_NOTFOUND)
	    {
		return 0;
	    }
	    else
	    {
		assert(0);
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(_connection->dbTxn() != 0)
	    {
		DBDeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}
    
void
Freeze::DBMapHelperI::clear()
{
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
	closeAllIterators();
    }

    for(;;)
    {
	try
	{
	    u_int32_t count;
	    int err = _db->truncate(txn, &count, txn != 0 ? 0 : DB_AUTO_COMMIT);
	    assert(err == 0);
	    break;
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(txn != 0)
	    {
		DBDeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void
Freeze::DBMapHelperI::destroy()
{
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
	closeAllIterators();
    }

    try
    {
	close();
	_connection->dbEnv()->dbremove(txn, _dbName.c_str(), 0, txn != 0 ? 0 : DB_AUTO_COMMIT);
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}


size_t
Freeze::DBMapHelperI::size() const
{
    //
    // TODO: DB_FAST_STAT doesn't seem to do what the documentation says...
    //
    DB_BTREE_STAT* s;
    _db->stat(&s, 0);

    size_t num = s->bt_ndata;
    free(s);
    return num;
}


void
Freeze::DBMapHelperI::closeAllIterators()
{
    while(!_iteratorList.empty())
    {
	(*_iteratorList.begin())->close();
    }
}

void
Freeze::DBMapHelperI::close()
{
    if(_db != 0)
    {
	_connection->unregisterMap(this);
    }
    _db = 0;
}

void
Freeze::DBMapHelperI::closeAllIteratorsExcept(const DBIteratorHelperI::TxPtr& tx) const
{
    assert(tx != 0);

    list<DBIteratorHelperI*>::iterator q = _iteratorList.begin();

    while(q != _iteratorList.end())
    {
	if((*q)->tx().get() == tx.get())
	{
	    ++q;
	}
	else
	{
	    (*q)->close();
	    q = _iteratorList.begin();
	}
    }
}

//
// Print for the various exception types.
//
void
Freeze::DBDeadlockException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase deadlock:\n" << message;
}

void
Freeze::DBNotFoundException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase record not found:\n" << message;
}

void
Freeze::DBException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}


void
Freeze::DBInvalidPositionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvalid position";
}
