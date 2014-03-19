// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

//
// GCC on OS X doesn't support __thread; and on HP-UX with aC++, there
// is strange compiler or linker bug when using __thread.
//
#if defined(__HP_aCC) || defined(__APPLE__)
   #define USE_PTHREAD_KEY 1
#endif

namespace
{

#ifdef USE_PTHREAD_KEY
pthread_key_t dbKey;
#else
__thread Database* db = 0;
#endif

}

CurrentDatabase::CurrentDatabase(const Ice::CommunicatorPtr& comm, const string& envName, const string& dbName) :
    _communicator(comm),
    _envName(envName),
    _dbName(dbName)
{
#ifdef USE_PTHREAD_KEY
#ifdef NDEBUG
    pthread_key_create(&dbKey, 0);
#else
    int rs = pthread_key_create(&dbKey, 0);
    assert(rs == 0);
#endif
#endif
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
#ifdef USE_PTHREAD_KEY
    Database* db = static_cast<Database*>(pthread_getspecific(dbKey));
#endif

    if(db == 0)
    {
        Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
        db = new Database(connection, _dbName);

        Mutex::Lock sync(_dbListMutex);
        _dbList.push_back(db);
#ifdef USE_PTHREAD_KEY
#ifdef NDEBUG
    pthread_setspecific(dbKey, db);
#else
    int rs = pthread_setspecific(dbKey, db);
    assert(rs == 0);
#endif
#endif
    }
    return *db;
}
