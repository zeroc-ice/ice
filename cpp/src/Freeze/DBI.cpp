// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/DBI.h>
#include <Ice/Stream.h>
#include <sys/stat.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

Freeze::DBI::DBI(const CommunicatorPtr& communicator, const PropertiesPtr& properties, const DBEnvIPtr& dbenvObj,
		 ::DB_ENV* dbenv, ::DB* db, const string& name) :
    _communicator(communicator),
    _properties(properties),
    _dbenvObj(dbenvObj),
    _dbenv(dbenv),
    _db(db),
    _name(name)
{
}

Freeze::DBI::~DBI()
{
    if (_db)
    {
	_communicator->getLogger()->warning("database has not been closed");
    }
}

void
Freeze::DBI::put(const std::string& key, const ::Ice::ObjectPtr& servant)
{
    //
    // TODO: Is synchronization necessary here? I really don't
    // understand what the Berekely DB documentation says with "free
    // threaded".
    //
    JTCSyncT<JTCMutex> sync(*this);

    if(!_db)
    {
	DBException ex;
	ex.message = "Freeze::DB::open: database has been closed";
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
    DB_TXN *tid;
    int ret;

    memset(&dbKey, 0, sizeof(dbKey));
    memset(&dbData, 0, sizeof(dbData));
    dbKey.data = const_cast<void*>(static_cast<const void*>(key.c_str()));
    dbKey.size = key.size();
    dbData.data = stream.b.begin();
    dbData.size = stream.b.size();

    while (true)
    {
	ret = txn_begin(_dbenv, 0, &tid, 0);
	if (ret != 0)
	{
	    DBException ex;
	    ex.message = "txn_begin: ";
	    ex.message += db_strerror(ret);
	    throw ex;
	}
	
	ret = _db->put(_db, tid, &dbKey, &dbData, 0);
	switch (ret)
	{
	    case 0:
	    {
		//
		// Everything ok, commit the transaction
		//
		ret = txn_commit(tid, 0);
		if (ret != 0)
		{
		    DBException ex;
		    ex.message = "txn_commit: ";
		    ex.message += db_strerror(ret);
		    throw ex;
		}
		return; // We're done
	    }
	    
	    case DB_LOCK_DEADLOCK:
	    {
		//
		// Deadlock, abort the transaction and retry
		//
		ret = txn_abort(tid);
		if (ret != 0)
		{
		    DBException ex;
		    ex.message = "txn_abort: ";
		    ex.message += db_strerror(ret);
		    throw ex;
		}
		break; // Repeat
	    }

	    default:
	    {
		//
		// Error, run recovery
		//
		DBException ex;
		ex.message = "DB->put: ";
		ex.message += db_strerror(ret);
		throw ex;
	    }
	}
    }
}

::Ice::ObjectPtr
Freeze::DBI::get(const std::string& key)
{
    //
    // TODO: Is synchronization necessary here? I really don't
    // understand what the Berekely DB documentation says with "free
    // threaded".
    //
    JTCSyncT<JTCMutex> sync(*this);

    if(!_db)
    {
	DBException ex;
	ex.message = "Freeze::DB::open: database has been closed";
	throw ex;
    }

    //
    // TODO: Do I need transactions for get()?
    //
    DBT dbKey, dbData;
    int ret;

    memset(&dbKey, 0, sizeof(dbKey));
    memset(&dbData, 0, sizeof(dbData));
    dbKey.data = const_cast<void*>(static_cast<const void*>(key.c_str()));
    dbKey.size = key.size();
    dbData.flags = DB_DBT_MALLOC;

    ret = _db->get(_db, 0, &dbKey, &dbData, 0);
    switch (ret)
    {
	case 0:
	{
	    //
	    // Everything ok
	    //
	    ObjectPtr servant;
	    try
	    {
		IceInternal::InstancePtr instance = IceInternal::getInstance(_communicator);
		IceInternal::Stream stream(instance);
		stream.b.resize(dbData.size);
		stream.i = stream.b.begin();
		memcpy(stream.b.begin(), dbData.data, dbData.size);
		stream.read(servant, "::Ice::Object");
	    }
	    catch(...)
	    {
		free(dbData.data);
		throw;
	    }
	    free(dbData.data);

	    if (!servant)
	    {
		throw NoServantFactoryException(__FILE__, __LINE__);
	    }

	    return servant;
	}

	case DB_NOTFOUND:
	{
	    //
	    // Key does not exist, return a null servant
	    //
	    return 0;
	}
	
	default:
	{
	    //
	    // Error, run recovery
	    //
	    DBException ex;
	    ex.message = "DB->get: ";
	    ex.message += db_strerror(ret);
	    throw ex;
	}
    }
}

void
Freeze::DBI::del(const std::string& key)
{
    //
    // TODO: Is synchronization necessary here? I really don't
    // understand what the Berekely DB documentation says with "free
    // threaded".
    //
    JTCSyncT<JTCMutex> sync(*this);

    if(!_db)
    {
	DBException ex;
	ex.message = "Freeze::DB::open: database has been closed";
	throw ex;
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

    int ret = _db->close(_db, 0);
    if(ret != 0)
    {
	DBException ex;
	ex.message = "DB->close: ";
	ex.message += db_strerror(ret);
	throw ex;
    }

    _dbenvObj->remove(_name);
    _dbenvObj = 0;
    _dbenv = 0;
    _db = 0;
}

Freeze::DBEnvI::DBEnvI(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _communicator(communicator),
    _properties(properties),
    _dbenv(0)
{
    int ret;

    ret = db_env_create(&_dbenv, 0);
    if (ret != 0)
    {
	DBException ex;
	ex.message = "db_env_create: ";
	ex.message += db_strerror(ret);
	throw ex;
    }

    _directory = _properties->getProperty("Freeze.Directory");

    const char* dir = 0;
    if (!_directory.empty())
    {
	dir = _directory.c_str();
    }

    ret = _dbenv->open(_dbenv, dir,
		       DB_CREATE |
		       DB_INIT_LOCK |
		       DB_INIT_LOG |
		       DB_INIT_MPOOL |
		       DB_INIT_TXN |
		       DB_RECOVER |
		       DB_THREAD,
		       S_IRUSR | S_IWUSR);
    if (ret != 0)
    {
	DBException ex;
	ex.message = "DB_ENV->open: ";
	ex.message += db_strerror(ret);
	throw ex;
    }
}

Freeze::DBEnvI::~DBEnvI()
{
    if (_dbenv)
    {
	_communicator->getLogger()->warning("database environment object has not been closed");
    }
}

DBPtr
Freeze::DBEnvI::open(const string& name)
{
    JTCSyncT<JTCMutex> sync(*this);

    if(!_dbenv)
    {
	DBException ex;
	ex.message = "Freeze::DBEnv::open: database environment has been closed";
	throw ex;
    }

    map<string, DBPtr>::iterator p = _dbmap.find(name);
    if (p != _dbmap.end())
    {
	return p->second;
    }

    int ret;

    ::DB* db;
    ret = db_create(&db, _dbenv, 0);
    if(ret != 0)
    {
	DBException ex;
	ex.message = "db_create: ";
	ex.message += db_strerror(ret);
	throw ex;
    }

    ret = db->open(db, name.c_str(), 0, DB_BTREE, DB_CREATE | DB_THREAD, S_IRUSR | S_IWUSR);
    if(ret != 0)
    {
	DBException ex;
	ex.message = "DB->open: ";
	ex.message += db_strerror(ret);
	throw ex;
    }

    DBPtr dbp = new DBI(_communicator, _properties, this, _dbenv, db, name);
    _dbmap[name] = dbp;
    return dbp;
}

void
Freeze::DBEnvI::close()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_dbenv)
    {
	return;
    }

    for (map<string, DBPtr>::iterator p = _dbmap.begin(); p != _dbmap.end(); ++p)
    {
	p->second->close();
    }
    _dbmap.clear();

    int ret = _dbenv->close(_dbenv, 0);
    if(ret != 0)
    {
	DBException ex;
	ex.message = "DB_ENV->close: ";
	ex.message += db_strerror(ret);
	throw ex;
    }

    _dbenv = 0;
}

void
Freeze::DBEnvI::remove(const string& name)
{
    JTCSyncT<JTCMutex> sync(*this);

    _dbmap.erase(name);
}

DBEnvPtr
Freeze::initialize(const CommunicatorPtr& communicator)
{
    return new DBEnvI(communicator, communicator->getProperties());
}

DBEnvPtr
Freeze::initializeWithProperties(const CommunicatorPtr& communicator, const PropertiesPtr& properties)
{
    return new DBEnvI(communicator, properties);
}
