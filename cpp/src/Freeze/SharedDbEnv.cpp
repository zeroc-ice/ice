// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/SharedDbEnv.h>
#include <Freeze/Exception.h>
#include <Freeze/Util.h>
#include <Freeze/MapDb.h>
#include <Freeze/TransactionalEvictorContext.h>
#include <Freeze/Catalog.h>
#include <Freeze/CatalogIndexList.h>

#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/MutexPtrTryLock.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/IceUtil.h>

#include <IceUtil/StringConverter.h>

#include <cstdlib>


using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace Freeze;

namespace Freeze
{

class CheckpointThread : public Thread, public Monitor<Mutex>
{
public:

    CheckpointThread(SharedDbEnv&, const Time&, Int, Int);

    virtual void run();

    void terminate();

private:
    SharedDbEnv& _dbEnv;
    bool _done;
    Time _checkpointPeriod;
    Int _kbyte;
    Int _trace;
};

}

namespace
{

struct MapKey
{
    string envName;
    Ice::CommunicatorPtr communicator;
};

inline bool
operator<(const MapKey& lhs, const MapKey& rhs)
{
    return (lhs.communicator < rhs.communicator) ||
        ((lhs.communicator == rhs.communicator) && (lhs.envName < rhs.envName));
}

#if DB_VERSION_MAJOR < 4
#error Freeze requires DB 4.x or greater
#endif

#if DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR < 3
void
dbErrCallback(const char* prefix, char* msg)
#else
void
dbErrCallback(const ::DbEnv* /*ignored*/, const char* prefix, const char* msg)
#endif
{
    const Freeze::SharedDbEnv* env = reinterpret_cast<const Freeze::SharedDbEnv*>(prefix);
    assert(env != 0);

    Ice::Trace out(env->getCommunicator()->getLogger(), "Berkeley DB");
    out << "DbEnv \"" << env->getEnvName() << "\": " << msg;
}

namespace
{

Mutex* mapMutex = 0;
Mutex* refCountMutex = 0;

class Init
{
public:

    Init()
    {
        mapMutex = new IceUtil::Mutex;
        refCountMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete mapMutex;
        mapMutex = 0;

        delete refCountMutex;
        refCountMutex = 0;
    }
};

Init init;

}

typedef map<MapKey, Freeze::SharedDbEnv*> SharedDbEnvMap;
SharedDbEnvMap* sharedDbEnvMap;

}

Freeze::SharedDbEnvPtr
Freeze::SharedDbEnv::get(const CommunicatorPtr& communicator, const string& envName, DbEnv* env)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(mapMutex);

    if(sharedDbEnvMap == 0)
    {
        sharedDbEnvMap = new SharedDbEnvMap;
    }

    MapKey key;
    key.envName = envName;
    key.communicator = communicator;

    {
        SharedDbEnvMap::iterator p = sharedDbEnvMap->find(key);
        if(p != sharedDbEnvMap->end())
        {
            return p->second;
        }
    }

    //
    // MapKey not found, let's create and open a new DbEnv
    //
    IceUtil::UniquePtr<SharedDbEnv> result(new SharedDbEnv(envName, communicator, env));

    //
    // Insert it into the map
    //
    pair<SharedDbEnvMap::iterator, bool> insertResult;
    insertResult = sharedDbEnvMap->insert(SharedDbEnvMap::value_type(key, result.get()));
    assert(insertResult.second);

    return result.release();
}

Freeze::SharedDbEnv::~SharedDbEnv()
{
    try
    {
        cleanup();
    }
    catch(const Ice::Exception& ex)
    {
        Error out(_communicator->getLogger());
        out << "Freeze DbEnv close error:" << ex;
    }
    catch(const std::exception& ex)
    {
        Error out(_communicator->getLogger());
        out << "Freeze DbEnv close error:" << ex.what();
    }
    catch(...)
    {
        Error out(_communicator->getLogger());
        out << "Freeze DbEnv close error: unknown exception";
    }

#ifdef _WIN32
    if(!TlsFree(_tsdKey))
    {
        Error out(_communicator->getLogger());
        out << "Freeze DbEnv close error:" << IceUtilInternal::lastErrorToString();
    }
#else
    int err = pthread_key_delete(_tsdKey);
    if(err != 0)
    {
        Error out(_communicator->getLogger());
        out << "Freeze DbEnv close error:" << IceUtilInternal::errorToString(err);
    }
#endif
}


Freeze::MapDb*
Freeze::SharedDbEnv::getSharedMapDb(const string& dbName,
                                    const string& key,
                                    const string& value,
                                    const KeyCompareBasePtr& keyCompare,
                                    const vector<MapIndexBasePtr>& indices,
                                    bool createDb)
{
    //
    // We don't want to lock to retrieve the catalog or catalog index
    //

    if(dbName == _catalog->dbName())
    {
        _catalog->checkTypes(key, value);
        return _catalog;
    }
    else if(dbName == _catalogIndexList->dbName())
    {
        _catalogIndexList->checkTypes(key, value);
        return _catalogIndexList;
    }

    IceUtil::Mutex::Lock lock(_mutex);

    SharedDbMap::iterator p = _sharedDbMap.find(dbName);
    if(p != _sharedDbMap.end())
    {
        MapDb* db = p->second;
        db->checkTypes(key, value);
        db->connectIndices(indices);
        return db;
    }


    //
    // key not found, let's create and open a new Db
    //

    //
    // Since we're going to put this SharedDb in the map no matter
    // what, we use our own transaction and connection to do so
    //

    ConnectionIPtr insertConnection = new ConnectionI(this);

    IceUtil::UniquePtr<MapDb> result(new MapDb(insertConnection, dbName, key, value,
                                     keyCompare, indices, createDb));

    //
    // Insert it into the map
    //
    pair<SharedDbMap::iterator, bool> insertResult;
    insertResult = _sharedDbMap.insert(SharedDbMap::value_type(dbName, result.get()));
    assert(insertResult.second);

    return result.release();
}



void
Freeze::SharedDbEnv::removeSharedMapDb(const string& dbName)
{
    IceUtil::Mutex::Lock lock(_mutex);

    SharedDbMap::iterator p = _sharedDbMap.find(dbName);
    if(p != _sharedDbMap.end())
    {
        MapDb* db = p->second;
        _sharedDbMap.erase(p);
        delete db;
    }
}


void 
Freeze::SharedDbEnv::__incRef()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(refCountMutex);
    _refCount++;
}

void
Freeze::SharedDbEnv::__decRef()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(refCountMutex);
    if(--_refCount == 0)
    {
        MapKey key;
        key.envName = _envName;
        key.communicator = _communicator;


        IceUtilInternal::MutexPtrTryLock<IceUtil::Mutex> mapLock(mapMutex);
        if(!mapLock.acquired())
        {
            //
            // Reacquire mutex in proper order and check again
            //
            lock.release();
            mapLock.acquire();
            lock.acquire();

            //
            // Now, maybe another thread has deleted 'this'; let's check
            // we're still in the map
            //

            if(sharedDbEnvMap == 0)
            {
                return;
            }

            SharedDbEnvMap::iterator p = sharedDbEnvMap->find(key);

            if(p == sharedDbEnvMap->end() || p->second != this)
            {
                //
                // 'this' has been deleted by another thread
                //
                return;
            }

            if(_refCount > 0)
            {
                return;
            }
        }

        //
        // Remove from map
        //
#ifndef NDEBUG
        size_t one = sharedDbEnvMap->erase(key);
        assert(one == 1);
#else
        sharedDbEnvMap->erase(key);
#endif
        if(sharedDbEnvMap->size() == 0)
        {
            delete sharedDbEnvMap;
            sharedDbEnvMap = 0;
        }

        //
        // Keep lock to prevent somebody else from reopening this DbEnv
        // before it's closed.
        //
        delete this;
    }
}


Freeze::TransactionalEvictorContextPtr
Freeze::SharedDbEnv::createCurrent()
{
    assert(getCurrent() == 0);

    Freeze::TransactionalEvictorContextPtr ctx = new TransactionalEvictorContext(this);
#ifdef _WIN32
    if(TlsSetValue(_tsdKey, ctx.get()) == 0)
    {
        IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
#else
    if(int err = pthread_setspecific(_tsdKey, ctx.get()))
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
    }
#endif

    //
    // Give one refcount to this thread!
    //
    ctx->__incRef();
    return ctx;
}

Freeze::TransactionalEvictorContextPtr
Freeze::SharedDbEnv::getCurrent()
{
#ifdef _WIN32
    void* val = TlsGetValue(_tsdKey);
#else
    void* val = pthread_getspecific(_tsdKey);
#endif

    if(val != 0)
    {
        return static_cast<TransactionalEvictorContext*>(val);
    }
    else
    {
        return 0;
    }
}

void
Freeze::SharedDbEnv::setCurrentTransaction(const Freeze::TransactionPtr& tx)
{
    TransactionIPtr txi;

    if(tx != 0)
    {
        txi = Freeze::TransactionIPtr::dynamicCast(tx);

        //
        // Verify it points to the good DbEnv
        //
        ConnectionIPtr conn = ConnectionIPtr::dynamicCast(tx->getConnection());
        if(!conn || conn->dbEnv() == 0)
        {
            throw DatabaseException(__FILE__, __LINE__, "invalid transaction");
        }

        if(conn->dbEnv().get() != this)
        {
            throw DatabaseException(__FILE__, __LINE__, "the given transaction is bound to environment '" +
                                    conn->dbEnv()->_envName + "'");
        }
    }

    Freeze::TransactionalEvictorContextPtr ctx = getCurrent();

    if(ctx != 0)
    {
        //
        // Release thread's refcount
        //
        ctx->__decRef();
    }

    if(tx != 0)
    {
        if(ctx == 0 || ctx->transaction().get() != txi.get())
        {
            ctx = new TransactionalEvictorContext(txi);

#ifdef _WIN32
            if(TlsSetValue(_tsdKey, ctx.get()) == 0)
            {
                IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
            }
#else
            if(int err = pthread_setspecific(_tsdKey, ctx.get()))
            {
                throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
            }
#endif
            //
            // Give one refcount to this thread
            //
            ctx->__incRef();
        }
    }
    else if(ctx != 0)
    {
#ifdef _WIN32
        if(TlsSetValue(_tsdKey, 0) == 0)
        {
            IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#else
        if(int err = pthread_setspecific(_tsdKey, 0))
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
        }
#endif
    }
}

Freeze::SharedDbEnv::SharedDbEnv(const std::string& envName,
                                 const Ice::CommunicatorPtr& communicator, DbEnv* env) :
    _env(env),
    _envName(envName),
    _communicator(communicator),
    _catalog(0),
    _catalogIndexList(0),
    _refCount(0)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

#ifdef _WIN32
    _tsdKey = TlsAlloc();
    if(_tsdKey == TLS_OUT_OF_INDEXES)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
#else
    int err = pthread_key_create(&_tsdKey, 0);
    if(err != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
    }
#endif

    string propertyPrefix = string("Freeze.DbEnv.") + envName;
    string dbHome = properties->getPropertyWithDefault(propertyPrefix + ".DbHome", envName);

    string encoding = properties->getPropertyWithDefault(propertyPrefix + ".EncodingVersion",
                                                         encodingVersionToString(Ice::currentEncoding));
    _encoding = stringToEncodingVersion(encoding);
    IceInternal::checkSupportedEncoding(_encoding);

    //
    // Normally the file lock is necessary, but for read-only situations (such as when
    // using the FreezeScript utilities) this property allows the file lock to be
    // disabled.
    //
    if(properties->getPropertyAsIntWithDefault(propertyPrefix + ".LockFile", 1) > 0)
    {
        //
        // Use a file lock to prevent multiple processes from opening the same db env. We
        // create the lock file in a sub-directory to ensure db_hotbackup won't try to copy
        // the file when backing up the environment (this would fail on Windows where copying
        // a locked file isn't possible).
        //
        if(!::IceUtilInternal::directoryExists(dbHome + "/__Freeze"))
        {
            ::IceUtilInternal::mkdir(dbHome + "/__Freeze", 0777);
        }
        _fileLock = new ::IceUtilInternal::FileLock(dbHome + "/__Freeze/lock");
    }

    _trace = properties->getPropertyAsInt("Freeze.Trace.DbEnv");

    try
    {
        if(_env == 0)
        {
            _envHolder.reset(new DbEnv(0));
            _env = _envHolder.get();

            if(_trace >= 1)
            {
                Trace out(_communicator->getLogger(), "Freeze.DbEnv");
                out << "opening database environment \"" << envName << "\"";
            }

            _env->set_errpfx(reinterpret_cast<char*>(this));

            _env->set_errcall(dbErrCallback);

#ifdef _WIN32
            //
            // Berkeley DB may use a different C++ runtime
            //
            _env->set_alloc(::malloc, ::realloc, ::free);
#endif

            //
            // Deadlock detection
            //
            _env->set_lk_detect(DB_LOCK_YOUNGEST);

            u_int32_t flags = DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;

            if(properties->getPropertyAsInt(propertyPrefix + ".DbRecoverFatal") > 0)
            {
                flags |= DB_RECOVER_FATAL | DB_CREATE;
            }
            else
            {
                flags |= DB_RECOVER | DB_CREATE;
            }

            if(properties->getPropertyAsIntWithDefault(propertyPrefix + ".DbPrivate", 1) > 0)
            {
                flags |= DB_PRIVATE;
            }

            //
            // Auto delete
            //
            bool autoDelete = (properties->getPropertyAsIntWithDefault(propertyPrefix + ".OldLogsAutoDelete", 1) > 0);

            if(autoDelete)
            {
#if (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR < 7)
                //
                // Old API
                //
                _env->set_flags(DB_LOG_AUTOREMOVE, 1);
#else
                _env->log_set_config(DB_LOG_AUTO_REMOVE, 1);
#endif
            }

            //
            // Threading
            //
            flags |= DB_THREAD;

            //
            // Berkeley DB expects file paths to be UTF8 encoded.
            //
            _env->open(nativeToUTF8(dbHome, getProcessStringConverter()).c_str(), flags, FREEZE_DB_MODE);

            //
            // Default checkpoint period is every 120 seconds
            //
            Int checkpointPeriod = properties->getPropertyAsIntWithDefault(propertyPrefix + ".CheckpointPeriod", 120);
            Int kbyte = properties->getPropertyAsIntWithDefault(propertyPrefix + ".PeriodicCheckpointMinSize", 0);

            if(checkpointPeriod > 0)
            {
                _thread = new CheckpointThread(*this, Time::seconds(checkpointPeriod), kbyte, _trace);
            }
        }

        //
        // Get catalogs
        //
        _catalog = new MapDb(_communicator, _encoding, catalogName(), Catalog::keyTypeId(),
                             Catalog::valueTypeId(), _env);
        _catalogIndexList = new MapDb(_communicator, _encoding, catalogIndexListName(),
                                      CatalogIndexList::keyTypeId(), CatalogIndexList::valueTypeId(), _env);
    }
    catch(const ::DbException& dx)
    {
        cleanup();
        throw DatabaseException(__FILE__, __LINE__, dx.what());
    }
}

void
Freeze::SharedDbEnv::cleanup()
{
    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.DbEnv");
        out << "closing database environment \"" << _envName << "\"";
    }

    //
    // Close & destroy all MapDbs
    //
    for(SharedDbMap::iterator p = _sharedDbMap.begin(); p != _sharedDbMap.end(); ++p)
    {
        try
        {
            delete p->second;
        }
        catch(const DatabaseException& ex)
        {
            Error out(_communicator->getLogger());
            out << "Freeze map: \"" << p->first << "\" close error: " << ex;
        }
        catch(const std::exception& ex)
        {
            Error out(_communicator->getLogger());
            out << "Freeze map: \"" << p->first << "\" close error: " << ex.what();
        }
        catch(...)
        {
            Error out(_communicator->getLogger());
            out << "Freeze map: \"" << p->first << "\" close error: unknown exception.";
        }
    }

    //
    // Same for catalogs
    //
    delete _catalog;
    delete _catalogIndexList;

    //
    // Then terminate checkpointing thread
    //
    if(_thread != 0)
    {
        _thread->terminate();
        _thread = 0;
    }

    //
    // And finally close env
    //

    if(_envHolder.get() != 0)
    {
        try
        {
            _envHolder->close(0);
        }
        catch(const ::DbException& dx)
        {
            throw DatabaseException(__FILE__, __LINE__, dx.what());
        }
    }
}


Freeze::CheckpointThread::CheckpointThread(SharedDbEnv& dbEnv, const Time& checkpointPeriod, Int kbyte, Int trace) :
    Thread("Freeze checkpoint thread"),
    _dbEnv(dbEnv),
    _done(false),
    _checkpointPeriod(checkpointPeriod),
    _kbyte(kbyte),
    _trace(trace)
{
    __setNoDelete(true);
    start();
    __setNoDelete(false);
}

void
Freeze::CheckpointThread::terminate()
{
    {
        Lock sync(*this);
        _done = true;
        notify();
    }

    getThreadControl().join();
}

void
Freeze::CheckpointThread::run()
{
    for(;;)
    {
        {
            Lock sync(*this);
            while(!_done && timedWait(_checkpointPeriod))
            {
                //
                // Loop
                //
            }
            if(_done)
            {
                return;
            }
        }

        try
        {
            if(_trace >= 2)
            {
                Trace out(_dbEnv.getCommunicator()->getLogger(), "Freeze.DbEnv");
                out << "checkpointing environment \"" << _dbEnv.getEnvName() << "\"";
            }
            _dbEnv.getEnv()->txn_checkpoint(_kbyte, 0, 0);
        }
        catch(const DbException& dx)
        {
            Warning out(_dbEnv.getCommunicator()->getLogger());
            out << "checkpoint on DbEnv \"" << _dbEnv.getEnvName() << "\" raised DbException: " << dx.what();
        }
    }
}
