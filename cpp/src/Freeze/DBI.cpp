// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Stream.h> // Not included in Ice/Ice.h
#include <Freeze/DBI.h>
#include <Freeze/EvictorI.h>
#include <Freeze/Initialize.h>
#include <sys/stat.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace Freeze;

#ifdef WIN32
#   define FREEZE_DB_MODE 0
#else
#   define FREEZE_DB_MODE (S_IRUSR | S_IWUSR)
#endif

Freeze::DBEnvironmentI::DBEnvironmentI(const CommunicatorPtr& communicator, const string& name) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _trace(0),
    _dbEnv(0),
    _name(name)
{
    _errorPrefix = "Freeze::DBEnvironment(\"" + _name + "\"): ";

    PropertiesPtr properties = _communicator->getProperties();
    string value;

    value = properties->getProperty("Freeze.Trace.DB");
    if (!value.empty())
    {
	_trace = atoi(value.c_str());
    }

    int ret;

    ret = db_env_create(&_dbEnv, 0);

    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "db_env_create: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    if (_trace >= 1)
    {
	ostringstream s;
	s << "opening database environment \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }

    ret = _dbEnv->open(_dbEnv, _name.c_str(),
		       DB_CREATE |
		       DB_INIT_LOCK |
		       DB_INIT_LOG |
		       DB_INIT_MPOOL |
		       //DB_INIT_TXN |
		       DB_RECOVER |
		       DB_THREAD,
		       FREEZE_DB_MODE);
    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "DB_ENV->open: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }
}

Freeze::DBEnvironmentI::~DBEnvironmentI()
{
    if (_dbEnv)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has not been closed";
	_communicator->getLogger()->warning(s.str());
    }
}

string
Freeze::DBEnvironmentI::getName()
{
    // No mutex lock necessary, _name is immutable
    return _name;
}

DBPtr
Freeze::DBEnvironmentI::openDB(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (!_dbEnv)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    map<string, DBPtr>::iterator p = _dbMap.find(name);
    if (p != _dbMap.end())
    {
	return p->second;
    }

    ::DB* db;
    int ret = db_create(&db, _dbEnv, 0);

    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "db_create: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    return new DBI(_communicator, this, db, name);
}

DBTransactionPtr
Freeze::DBEnvironmentI::startTransaction()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    return new DBTransactionI(_communicator, _dbEnv, _name);
}

void
Freeze::DBEnvironmentI::close()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (!_dbEnv)
    {
	return;
    }

    while(!_dbMap.empty())
    {
	_dbMap.begin()->second->close();
    }

    if (_trace >= 1)
    {
	ostringstream s;
	s << "closing database environment \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }

    int ret = _dbEnv->close(_dbEnv, 0);

    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "DB_ENV->close: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    _dbEnv = 0;
}

void
Freeze::DBEnvironmentI::add(const string& name, const DBPtr& db)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    _dbMap[name] = db;
}

void
Freeze::DBEnvironmentI::remove(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    _dbMap.erase(name);
}

DBEnvironmentPtr
Freeze::initialize(const CommunicatorPtr& communicator, const string& name)
{
    return new DBEnvironmentI(communicator, name);
}

Freeze::DBTransactionI::DBTransactionI(const CommunicatorPtr& communicator, ::DB_ENV* dbEnv, const string& name) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _trace(0),
    _tid(0),
    _name(name)
{
    _errorPrefix = "Freeze::DBTransaction(\"" + _name + "\"): ";

    PropertiesPtr properties = _communicator->getProperties();
    string value;

    value = properties->getProperty("Freeze.Trace.DB");
    if (!value.empty())
    {
	_trace = atoi(value.c_str());
    }

    if (_trace >= 2)
    {
	ostringstream s;
	s << "starting transaction for environment \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = txn_begin(dbEnv, 0, &_tid, 0);
    
    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "txn_begin: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }
}

Freeze::DBTransactionI::~DBTransactionI()
{
    if (_tid)
    {
	ostringstream s;
	s << _errorPrefix << "transaction has not been committed or aborted";
	_communicator->getLogger()->warning(s.str());
    }
}

void
Freeze::DBTransactionI::commit()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_tid)
    {
	ostringstream s;
	s << _errorPrefix << "transaction has already been committed or aborted";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    if (_trace >= 2)
    {
	ostringstream s;
	s << "committing transaction for environment \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = txn_commit(_tid, 0);
    
    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "txn_commit: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    _tid = 0;
}

void
Freeze::DBTransactionI::abort()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_tid)
    {
	ostringstream s;
	s << _errorPrefix << "transaction has already been committed or aborted";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    if (_trace >= 2)
    {
	ostringstream s;
	s << "aborting transaction for environment \"" << _name << "\" due to deadlock";
	_logger->trace("DB", s.str());
    }
    
    int ret = txn_abort(_tid);
    
    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "txn_abort: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    _tid = 0;
}

Freeze::DBI::DBI(const CommunicatorPtr& communicator, const DBEnvironmentIPtr& dbEnvObj, ::DB* db,
		 const string& name) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _trace(0),
    _dbEnvObj(dbEnvObj),
    _db(db),
    _name(name)
{
    _errorPrefix = "Freeze::DB(\"" + _name + "\"): ";

    PropertiesPtr properties = _communicator->getProperties();
    string value;

    value = properties->getProperty("Freeze.Trace.DB");
    if (!value.empty())
    {
	_trace = atoi(value.c_str());
    }

    if (_trace >= 1)
    {
	ostringstream s;
	s << "opening database \"" << _name << "\" in environment \"" << _dbEnvObj->getName() << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = _db->open(_db, name.c_str(), 0, DB_BTREE, DB_CREATE, FREEZE_DB_MODE);

    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "DB->open: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }
}

Freeze::DBI::~DBI()
{
    if (_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has not been closed";
	_communicator->getLogger()->warning(s.str());
    }
}

string
Freeze::DBI::getName()
{
    // No mutex lock necessary, _name is immutable
    return _name;
}

void
Freeze::DBI::put(const Key& key, const Value& value, bool txn)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    DBT dbKey, dbData;
    memset(&dbKey, 0, sizeof(dbKey));
    memset(&dbData, 0, sizeof(dbData));
    dbKey.data = const_cast<void*>(static_cast<const void*>(key.begin()));
    dbKey.size = key.size();
    dbData.data = const_cast<void*>(static_cast<const void*>(value.begin()));
    dbData.size = value.size();

    while (true)
    {
	DBTransactionPtr txnObj;
	if (txn)
	{
	    txnObj = _dbEnvObj->startTransaction();
	}
	
	if (_trace >= 1)
	{
	    ostringstream s;
	    s << "writing value in database \"" << _name << "\"";
	    _logger->trace("DB", s.str());
	}

	int ret = _db->put(_db, 0, &dbKey, &dbData, 0);

	switch (ret)
	{
	    case 0:
	    {
		if (txnObj)
		{
		    //
		    // Everything ok, commit the transaction
		    //
		    txnObj->commit();
		}

		return; // We're done
	    }
	    
	    case DB_LOCK_DEADLOCK:
	    {
		if (txnObj)
		{
		    //
		    // Deadlock, abort the transaction and retry
		    //
		    txnObj->abort();
		    break; // Repeat
		}
		else
		{
		    ostringstream s;
		    s << _errorPrefix << "DB->put: " << db_strerror(ret);
		    DBException ex;
		    ex.message = s.str();
		    throw ex;
		}
	    }

	    default:
	    {
		if (txnObj)
		{
		    //
		    // Error, run recovery
		    //
		    txnObj->abort();
		}

		ostringstream s;
		s << _errorPrefix << "DB->put: " << db_strerror(ret);
		DBException ex;
		ex.message = s.str();
		throw ex;
	    }
	}
    }
}

Value
Freeze::DBI::get(const Key& key)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    DBT dbKey, dbData;
    memset(&dbKey, 0, sizeof(dbKey));
    memset(&dbData, 0, sizeof(dbData));
    dbKey.data = const_cast<void*>(static_cast<const void*>(key.begin()));
    dbKey.size = key.size();

    if (_trace >= 1)
    {
	ostringstream s;
	s << "reading value from database \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = _db->get(_db, 0, &dbKey, &dbData, 0);

    switch (ret)
    {
	case 0:
	{
	    //
	    // Everything ok
	    //
	    Value value;
	    value.resize(dbData.size);
	    memcpy(value.begin(), dbData.data, dbData.size);
	    return value;
	}

	case DB_NOTFOUND:
	{
	    //
	    // Key does not exist, return a null servant
	    //
	    return Value();
	}
	
	default:
	{
	    ostringstream s;
	    s << _errorPrefix << "DB->get: " << db_strerror(ret);
	    DBException ex;
	    ex.message = s.str();
	    throw ex;
	}
    }
}

void
Freeze::DBI::del(const Key& key)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    DBT dbKey;
    memset(&dbKey, 0, sizeof(dbKey));
    dbKey.data = const_cast<void*>(static_cast<const void*>(key.begin()));
    dbKey.size = key.size();

    if (_trace >= 1)
    {
	ostringstream s;
	s << "deleting value from database \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = _db->del(_db, 0, &dbKey, 0);

    switch (ret)
    {
	case 0:
	{
	    //
	    // Everything ok
	    //
	    break;
	}

	default:
	{
	    ostringstream s;
	    s << _errorPrefix << "DB->del: " << db_strerror(ret);
	    DBException ex;
	    ex.message = s.str();
	    throw ex;
	}
    }
}

void
Freeze::DBI::putServant(const string& identity, const ObjectPtr& servant, bool txn)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    if (!servant)
    {
	return;
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(_communicator);
    IceInternal::Stream stream(instance);
    stream.write(servant);

    DBT dbKey, dbData;
    memset(&dbKey, 0, sizeof(dbKey));
    memset(&dbData, 0, sizeof(dbData));
    dbKey.data = const_cast<void*>(static_cast<const void*>(identity.c_str()));
    dbKey.size = identity.size();
    dbData.data = stream.b.begin();
    dbData.size = stream.b.size();

    while (true)
    {
	DBTransactionPtr txnObj;
	if (txn)
	{
	    txnObj = _dbEnvObj->startTransaction();
	}
	
	if (_trace >= 1)
	{
	    ostringstream s;
	    s << "writing Servant for identity \"" << identity << "\" in database \"" << _name << "\"";
	    _logger->trace("DB", s.str());
	}

	int ret = _db->put(_db, 0, &dbKey, &dbData, 0);

	switch (ret)
	{
	    case 0:
	    {
		if (txnObj)
		{
		    //
		    // Everything ok, commit the transaction
		    //
		    txnObj->commit();
		}

		return; // We're done
	    }
	    
	    case DB_LOCK_DEADLOCK:
	    {
		if (txnObj)
		{
		    //
		    // Deadlock, abort the transaction and retry
		    //
		    txnObj->abort();
		    break; // Repeat
		}
		else
		{
		    ostringstream s;
		    s << _errorPrefix << "DB->put: " << db_strerror(ret);
		    DBException ex;
		    ex.message = s.str();
		    throw ex;
		}
	    }

	    default:
	    {
		if (txnObj)
		{
		    //
		    // Error, run recovery
		    //
		    txnObj->abort();
		}

		ostringstream s;
		s << _errorPrefix << "DB->put: " << db_strerror(ret);
		DBException ex;
		ex.message = s.str();
		throw ex;
	    }
	}
    }
}

ObjectPtr
Freeze::DBI::getServant(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    DBT dbKey, dbData;
    memset(&dbKey, 0, sizeof(dbKey));
    memset(&dbData, 0, sizeof(dbData));
    dbKey.data = const_cast<void*>(static_cast<const void*>(identity.c_str()));
    dbKey.size = identity.size();

    if (_trace >= 1)
    {
	ostringstream s;
	s << "reading Servant for identity \"" << identity << "\" from database \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = _db->get(_db, 0, &dbKey, &dbData, 0);

    switch (ret)
    {
	case 0:
	{
	    //
	    // Everything ok
	    //
	    IceInternal::InstancePtr instance = IceInternal::getInstance(_communicator);
	    IceInternal::Stream stream(instance);
	    stream.b.resize(dbData.size);
	    stream.i = stream.b.begin();
	    memcpy(stream.b.begin(), dbData.data, dbData.size);
	    
	    ObjectPtr servant;
	    stream.read(servant, "::Ice::Object");

	    if (!servant)
	    {
		throw NoServantFactoryException(__FILE__, __LINE__);
	    }

	    return servant;
	}

	case DB_NOTFOUND:
	{
	    //
	    // Identity does not exist, return a null servant
	    //
	    return 0;
	}
	
	default:
	{
	    ostringstream s;
	    s << _errorPrefix << "DB->get: " << db_strerror(ret);
	    DBException ex;
	    ex.message = s.str();
	    throw ex;
	}
    }
}

void
Freeze::DBI::delServant(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    DBT dbKey;
    memset(&dbKey, 0, sizeof(dbKey));
    dbKey.data = const_cast<void*>(static_cast<const void*>(identity.c_str()));
    dbKey.size = identity.size();

    if (_trace >= 1)
    {
	ostringstream s;
	s << "deleting Servant for identity \"" << identity << "\" from database \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = _db->del(_db, 0, &dbKey, 0);

    switch (ret)
    {
	case 0:
	{
	    //
	    // Everything ok
	    //
	    break;
	}

	default:
	{
	    ostringstream s;
	    s << _errorPrefix << "DB->del: " << db_strerror(ret);
	    DBException ex;
	    ex.message = s.str();
	    throw ex;
	}
    }
}

void
Freeze::DBI::close()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	return;
    }

    if (_trace >= 1)
    {
	ostringstream s;
	s << "closing database \"" << _name << "\"";
	_logger->trace("DB", s.str());
    }
    
    int ret = _db->close(_db, 0);

    if (ret != 0)
    {
	ostringstream s;
	s << _errorPrefix << "DB->close: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    _dbEnvObj->remove(_name);
    _dbEnvObj = 0;
    _db = 0;
}

EvictorPtr
Freeze::DBI::createEvictor()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_db)
    {
	ostringstream s;
	s << _errorPrefix << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    return new EvictorI(this, _communicator);
}
