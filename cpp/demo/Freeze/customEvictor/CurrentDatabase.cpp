// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <CurrentDatabase.h>

using namespace std;
using namespace IceUtil;

//
// This implementation is very simple but not restartable, i.e., you
// can only create and destroy one CurrentDatabase per process run.
//

#ifdef _MSC_VER
   #define __thread __declspec(thread)
#endif

namespace
{

//
// We keep a db on each thread.
//
__thread Database* db = 0;

}

CurrentDatabase::CurrentDatabase(const Ice::CommunicatorPtr& comm, const string& envName, const string& dbName) :
    _communicator(comm),
    _envName(envName),
    _dbName(dbName)
{
}

CurrentDatabase::~CurrentDatabase()
{
    for(list<Database*>::iterator p = _dbList.begin(); p != _dbList.end(); ++p)
    {
        delete *p;
    }
}

Database&
CurrentDatabase::get()
{
    if(db == 0)
    {
        Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
        db = new Database(connection, _dbName);

        Mutex::Lock sync(_dbListMutex);
        _dbList.push_back(db);
    }
    return *db;
}
