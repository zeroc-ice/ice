// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <Freeze/DBI.h>
#include <sys/stat.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

Freeze::DBI::DBI(const CommunicatorPtr& communicator, const PropertiesPtr& properties, const DBEnvIPtr& dbenv,
		 ::DB* db, const string& name) :
    _communicator(communicator),
    _properties(properties),
    _dbenv(dbenv),
    _db(db),
    _name(name)
{
}

Freeze::DBI::~DBI()
{
    if (!_closed)
    {
	_communicator->getLogger()->warning("database has not been closed");
    }
}

void
Freeze::DBI::close()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_dbenv)
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

    _dbenv->remove(_name);
    _dbenv = 0;
}

Freeze::DBEnvI::DBEnvI(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _closed(false),
    _communicator(communicator),
    _properties(properties)
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
    if (!_closed)
    {
	_communicator->getLogger()->warning("database environment object has not been closed");
    }
}

DBPtr
Freeze::DBEnvI::open(const string& name)
{
    JTCSyncT<JTCMutex> sync(*this);

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

    DBPtr dbp = new DBI(_communicator, _properties, this, db, name);
    _dbmap[name] = dbp;
    return dbp;
}

void
Freeze::DBEnvI::close()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_closed)
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

    _closed = true;
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
