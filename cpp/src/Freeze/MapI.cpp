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


#include <Freeze/MapI.h>
#include <Freeze/Exception.h>
#include <Freeze/SharedDb.h>
#include <Freeze/Util.h>
#include <stdlib.h>

using namespace std;
using namespace Ice;
using namespace Freeze;


//
// MapHelper (from Map.h)
//

Freeze::MapHelper*
Freeze::MapHelper::create(const Freeze::ConnectionPtr& connection, 
			  const string& dbName, 
			  bool createDb)
{
    Freeze::ConnectionIPtr connectionI = Freeze::ConnectionIPtr::dynamicCast(connection);
    return new MapHelperI(connectionI, dbName, createDb);
}

Freeze::MapHelper::~MapHelper()
{
}


//
// IteratorHelper (from Map.h)
//

Freeze::IteratorHelper* 
Freeze::IteratorHelper::create(const MapHelper& m, bool readOnly)
{
    const MapHelperI& actualMap = dynamic_cast<const MapHelperI&>(m);

    auto_ptr<IteratorHelperI> r(new IteratorHelperI(actualMap, readOnly));
    if(r->findFirst())
    {
	return r.release();
    }
    else
    {
	return 0;
    }
}


Freeze::IteratorHelper::~IteratorHelper()
{
}



//
// IteratorHelperI
//


Freeze::IteratorHelperI::IteratorHelperI(const MapHelperI& m, bool readOnly) :
    _map(m),
    _dbc(0),
    _tx(0)
{
    if(_map._trace >= 2)
    {
	Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
	out << "opening iterator on Db \"" << _map._dbName << "\"";
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
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    _map._iteratorList.push_back(this);
}

Freeze::IteratorHelperI::IteratorHelperI(const IteratorHelperI& it) :
    _map(it._map),
    _dbc(0),
    _tx(0)
{
    if(_map._trace >= 2)
    {
	Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
	out << "duplicating iterator on Db \"" << _map._dbName << "\"";
    }

    try
    {
	it._dbc->dup(&_dbc, DB_POSITION);
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
   
    _tx = it._tx;
    _map._iteratorList.push_back(this);
}

Freeze::IteratorHelperI::~IteratorHelperI()
{
    close();
}

bool 
Freeze::IteratorHelperI::findFirst() const
{
    return next();
}

bool 
Freeze::IteratorHelperI::find(const Key& key) const
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

	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

Freeze::IteratorHelper*
Freeze::IteratorHelperI::clone() const
{
    return new IteratorHelperI(*this);
}
    
void
Freeze::IteratorHelperI::get(const Key*& key, const Value*& value) const
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
		throw InvalidPositionException(__FILE__, __LINE__);
	    }
	    else 
	    {
		//
		// Bug in Freeze
		//
		assert(0);
		throw DatabaseException(__FILE__, __LINE__);
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
		DatabaseException ex(__FILE__, __LINE__);
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

	    DeadlockException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}
    
const Freeze::Key*
Freeze::IteratorHelperI::get() const
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
		throw InvalidPositionException(__FILE__, __LINE__);
	    }
	    else 
	    {
		//
		// Bug in Freeze
		//
		assert(0);
		throw DatabaseException(__FILE__, __LINE__);
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
		DatabaseException ex(__FILE__, __LINE__);
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

	    DeadlockException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void 
Freeze::IteratorHelperI::set(const Value& value)
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
	int err;
	err = _dbc->put(&dbKey, &dbValue, DB_CURRENT);
	assert(err == 0);
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_tx != 0)
	{
	    _tx->dead();
	}

	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

void
Freeze::IteratorHelperI::erase()
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
	    throw InvalidPositionException(__FILE__, __LINE__);
	}
	assert(err == 0);
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_tx != 0)
	{
	    _tx->dead();
	}

	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

bool
Freeze::IteratorHelperI::next() const
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

	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

bool
Freeze::IteratorHelperI::equals(const IteratorHelper& rhs) const
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
	    Key rhsKey = *dynamic_cast<const IteratorHelperI&>(rhs).get();
	    return *get() == rhsKey;
	}
	catch(const InvalidPositionException&)
	{
	    return false;
	}
    }
}

void
Freeze::IteratorHelperI::close()
{
    if(_dbc != 0)
    {
	if(_map._trace >= 2)
	{
	    Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
	    out << "closing iterator on Db \"" << _map._dbName << "\"";
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
		DeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	}
	catch(const ::DbException& dx)
	{
	    cleanup();
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	cleanup();
    }
}

void
Freeze::IteratorHelperI::cleanup()
{
    _dbc = 0;
    _map._iteratorList.remove(this);
    _tx = 0;
}


//
// IteratorHelperI::Tx
//

Freeze::IteratorHelperI::Tx::Tx(const MapHelperI& m) :
    _map(m),
    _txn(0),
    _dead(false)
{
    if(_map._trace >= 2)
    {
	Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
	out << "starting transaction for Db \"" << _map._dbName << "\"";
    }

    try
    {
	_map._connection->dbEnv()->txn_begin(0, &_txn, 0);
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}
	

Freeze::IteratorHelperI::Tx::~Tx()
{
    if(_dead)
    {
	if(_map._trace >= 2)
	{
	    Trace out(_map._connection->communicator()->getLogger(), "");
	    out << "aborting transaction for Db \"" << _map._dbName << "\"";
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
	if(_map._trace >= 2)
	{
	    Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
	    out << "committing transaction for Db \"" << _map._dbName.c_str() << "\"";
	}

	try
	{
	    _txn->commit(0);
	}
	catch(const ::DbDeadlockException& dx)
	{
	    DeadlockException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void
Freeze::IteratorHelperI::Tx::dead()
{
    _dead = true;
}



//
// MapHelperI
//


Freeze::MapHelperI::MapHelperI(const ConnectionIPtr& connection, 
			       const std::string& dbName, 
			       bool createDb) :
    _connection(connection),
    _db(SharedDb::get(connection, dbName, createDb)),
    _dbName(dbName),
    _trace(connection->trace())
{ 
    _connection->registerMap(this);
}

Freeze::MapHelperI::~MapHelperI()
{
    close();
}

Freeze::IteratorHelper*
Freeze::MapHelperI::find(const Key& k, bool readOnly) const
{
    for(;;)
    {
	try
	{  
	    auto_ptr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly));
	    if(r->find(k))
	    {
		return r.release();
	    }
	    else
	    {
		return 0;
	    }
	}
	catch(const DeadlockException&)
	{
	    if(_connection->dbTxn() != 0)
	    {
		throw;
	    }
	    else
	    {
		if(_connection->deadlockWarning())
		{
		    Warning out(_connection->communicator()->getLogger());
		    out << "Deadlock in Freeze::MapHelperI::find on Map \"" 
			<< _dbName << "\"; retrying ...";
		}

		//
		// Ignored, try again
		//
	    }
	}
    }
}

void
Freeze::MapHelperI::put(const Key& key, const Value& value)
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
		throw DatabaseException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(txn != 0)
	    {
		DeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		if(_connection->deadlockWarning())
		{
		    Warning out(_connection->communicator()->getLogger());
		    out << "Deadlock in Freeze::MapHelperI::put on Map \"" 
			<< _dbName << "\"; retrying ...";
		}

		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

size_t
Freeze::MapHelperI::erase(const Key& key)
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
		throw DatabaseException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(txn != 0)
	    {
		DeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		if(_connection->deadlockWarning())
		{
		    Warning out(_connection->communicator()->getLogger());
		    out << "Deadlock in Freeze::MapHelperI::erase on Map \"" 
			<< _dbName << "\"; retrying ...";
		}

		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

size_t
Freeze::MapHelperI::count(const Key& key) const
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
		throw DatabaseException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(_connection->dbTxn() != 0)
	    {
		DeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		if(_connection->deadlockWarning())
		{
		    Warning out(_connection->communicator()->getLogger());
		    out << "Deadlock in Freeze::MapHelperI::count on Map \"" 
			<< _dbName << "\"; retrying ...";
		}

		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}
    
void
Freeze::MapHelperI::clear()
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
	    int err;
	    err = _db->truncate(txn, &count, txn != 0 ? 0 : DB_AUTO_COMMIT);
	    assert(err == 0);
	    break;
	}
	catch(const ::DbDeadlockException& dx)
	{
	    if(txn != 0)
	    {
		DeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    else
	    {
		if(_connection->deadlockWarning())
		{
		    Warning out(_connection->communicator()->getLogger());
		    out << "Deadlock in Freeze::MapHelperI::clear on Map \"" 
			<< _dbName << "\"; retrying ...";
		}

		//
		// Ignored, try again
		//
	    }
	}
	catch(const ::DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void
Freeze::MapHelperI::destroy()
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
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}


size_t
Freeze::MapHelperI::size() const
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
Freeze::MapHelperI::closeAllIterators()
{
    while(!_iteratorList.empty())
    {
	(*_iteratorList.begin())->close();
    }
}

void
Freeze::MapHelperI::close()
{
    if(_db != 0)
    {
	_connection->unregisterMap(this);
    }
    _db = 0;
}

void
Freeze::MapHelperI::closeAllIteratorsExcept(const IteratorHelperI::TxPtr& tx) const
{
    assert(tx != 0);

    list<IteratorHelperI*>::iterator q = _iteratorList.begin();

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
Freeze::DeadlockException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase deadlock:\n" << message;
}

void
Freeze::NotFoundException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase record not found:\n" << message;
}

void
Freeze::DatabaseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}


void
Freeze::InvalidPositionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvalid position";
}
