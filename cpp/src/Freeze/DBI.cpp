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

DBPtr
Freeze::DBFactoryI::createDB()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    return 0;
}

void
Freeze::DBFactoryI::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroy = true;
}

Freeze::DBFactoryI::DBFactoryI(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _communicator(communicator),
    _properties(properties),
    _destroy(false)
{
    string directory = _properties->getProperty("Freeze.Directory");

//
// TODO: Should we really try to create the directory? Perhaps we
// better leave this task to the administrator, and simply let DB fail
// in case the directory does not exist.
//
/*
    if(!directory.empty())
    {
	//
	// Check whether the directory exists. If yes, we don't need
	// to create it. Note that we don't further check the type of
	// the file, DB will fail appropriately if it's the wrong
	// time.
	//
	struct stat sb;
	if (stat(directory.c_str(), &sb) == 0)
	{
	    //
	    // Directory does not exist, create it.
	    //
	    if (mkdir(directory.c_str(), S_IRWXU) != 0)
	    {
		throw SystemException(__FILE__, __LINE__);
	    }
	}
    }
*/

    int ret;

    if ((ret = db_env_create(&_dbenv, 0)) != 0)
    {
	DBException ex;
	ex.message = db_strerror(ret);
	throw ex;
    }

    const char* dir = 0;
    if (!directory.empty())
    {
	dir = directory.c_str();
    }

    if ((ret = _dbenv->open(_dbenv, dir,
			    DB_CREATE |
			    DB_INIT_LOCK |
			    DB_INIT_LOG |
			    DB_INIT_MPOOL |
			    DB_INIT_TXN |
			    DB_RECOVER |
			    DB_THREAD,
			    S_IRUSR | S_IWUSR)) != 0)
    {
	DBException ex;
	ex.message = db_strerror(ret);
	throw ex;
    }
}

Freeze::DBFactoryI::~DBFactoryI()
{
    if (!_destroy)
    {
	_communicator->getLogger()->warning("database factory object has not been destroyed");
    }
}

DBFactoryPtr
Freeze::initialize(const CommunicatorPtr& communicator)
{
    return new DBFactoryI(communicator, communicator->getProperties());
}

DBFactoryPtr
Freeze::initializeWithProperties(const CommunicatorPtr& communicator, const PropertiesPtr& properties)
{
    return new DBFactoryI(communicator, properties);
}

