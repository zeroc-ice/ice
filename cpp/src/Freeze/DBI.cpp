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
#include <sstream>

using namespace std;
using namespace Ice;
using namespace Freeze;

#ifdef WIN32
#   define FREEZE_DB_MODE 0
#else
#   define FREEZE_DB_MODE (S_IRUSR | S_IWUSR)
#endif

Freeze::DBI::DBI(const CommunicatorPtr& communicator, const string& name, const DBEnvIPtr& dbenvObj, ::DB_ENV* dbenv,
		 ::DB* db) :
    _communicator(communicator),
    _name(name),
    _dbenvObj(dbenvObj),
    _dbenv(dbenv),
    _db(db)
{
}

Freeze::DBI::~DBI()
{
    if (_db)
    {
	ostringstream s;
	s << "Freeze::DB(\"" << _name << "\"): ";
	s << "\"" << _name << "\" has not been closed";
	_communicator->getLogger()->warning(s.str());
    }
}

void
Freeze::DBI::put(const string& key, const ObjectPtr& servant)
{
    //
    // TODO: Is synchronization necessary here? I really don't
    // understand what the Berekely DB documentation says with "free
    // threaded".
    //
    JTCSyncT<JTCMutex> sync(*this);

    if(!_db)
    {
	ostringstream s;
	s << "Freeze::DB(\"" << _name << "\"): ";
	s << "\"" << _name << "\" has been closed";
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
	    ostringstream s;
	    s << "Freeze::DB(\"" << _name << "\"): ";
	    s << "txn_begin: " << db_strerror(ret);
	    DBException ex;
	    ex.message = s.str();
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
		    ostringstream s;
		    s << "Freeze::DB(\"" << _name << "\"): ";
		    s << "txn_commit: " << db_strerror(ret);
		    DBException ex;
		    ex.message = s.str();
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
		    ostringstream s;
		    s << "Freeze::DB(\"" << _name << "\"): ";
		    s << "txn_abort: " << db_strerror(ret);
		    DBException ex;
		    ex.message = s.str();
		    throw ex;
		}
		break; // Repeat
	    }

	    default:
	    {
		//
		// Error, run recovery
		//
		ostringstream s;
		s << "Freeze::DB(\"" << _name << "\"): ";
		s << "DB->put: " << db_strerror(ret);
		DBException ex;
		ex.message = s.str();
		throw ex;
	    }
	}
    }
}

ObjectPtr
Freeze::DBI::get(const string& key)
{
    //
    // TODO: Is synchronization necessary here? I really don't
    // understand what the Berekely DB documentation says with "free
    // threaded".
    //
    JTCSyncT<JTCMutex> sync(*this);

    if(!_db)
    {
	ostringstream s;
	s << "Freeze::DB(\"" << _name << "\"): ";
	s << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
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
	    ostringstream s;
	    s << "Freeze::DB(\"" << _name << "\"): ";
	    s << "DB->get: " << db_strerror(ret);
	    DBException ex;
	    ex.message = s.str();
	    throw ex;
	}
    }
}

void
Freeze::DBI::del(const string& key)
{
    //
    // TODO: Is synchronization necessary here? I really don't
    // understand what the Berekely DB documentation says with "free
    // threaded".
    //
    JTCSyncT<JTCMutex> sync(*this);

    if(!_db)
    {
	ostringstream s;
	s << "Freeze::DB(\"" << _name << "\"): ";
	s << "\"" << _name << "\" has been closed";
	DBException ex;
	ex.message = s.str();
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
	ostringstream s;
	s << "Freeze::DB(\"" << _name << "\"): ";
	s << "DB->close: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    _dbenvObj->remove(_name);
    _dbenvObj = 0;
    _dbenv = 0;
    _db = 0;
}

Freeze::DBEnvI::DBEnvI(const CommunicatorPtr& communicator, const string& directory) :
    _communicator(communicator),
    _directory(directory),
    _dbenv(0)
{
    int ret;

    ret = db_env_create(&_dbenv, 0);
    if (ret != 0)
    {
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "db_env_create: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    ret = _dbenv->open(_dbenv, _directory.c_str(),
		       DB_CREATE |
		       DB_INIT_LOCK |
		       DB_INIT_LOG |
		       DB_INIT_MPOOL |
		       DB_INIT_TXN |
		       DB_RECOVER |
		       DB_THREAD,
		       FREEZE_DB_MODE);
    if (ret != 0)
    {
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "DB_ENV->open: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }
}

Freeze::DBEnvI::~DBEnvI()
{
    if (_dbenv)
    {
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "\"" << _directory << "\" has not been closed";
	_communicator->getLogger()->warning(s.str());
    }
}

DBPtr
Freeze::DBEnvI::open(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if(!_dbenv)
    {
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "\"" << _directory << "\" has been closed";
	DBException ex;
	ex.message = s.str();
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
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "db_create: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    ret = db->open(db, name.c_str(), 0, DB_BTREE, DB_CREATE | DB_THREAD, FREEZE_DB_MODE);
    if(ret != 0)
    {
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "DB->open: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    DBPtr dbp = new DBI(_communicator, name, this, _dbenv, db);
    _dbmap[name] = dbp;
    return dbp;
}

void
Freeze::DBEnvI::close()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (!_dbenv)
    {
	return;
    }

    while(!_dbmap.empty())
    {
	_dbmap.begin()->second->close();
    }

    int ret = _dbenv->close(_dbenv, 0);
    if(ret != 0)
    {
	ostringstream s;
	s << "Freeze::DBEnv(\"" << _directory << "\"): ";
	s << "DB_ENV->close: " << db_strerror(ret);
	DBException ex;
	ex.message = s.str();
	throw ex;
    }

    _dbenv = 0;
}

void
Freeze::DBEnvI::remove(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    _dbmap.erase(name);
}

DBEnvPtr
Freeze::initialize(const CommunicatorPtr& communicator, const string& directory)
{
    return new DBEnvI(communicator, directory);
}
