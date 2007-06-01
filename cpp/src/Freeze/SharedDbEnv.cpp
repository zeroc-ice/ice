// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/SharedDbEnv.h>
#include <Freeze/Exception.h>
#include <Freeze/Util.h>
#include <Freeze/SharedDb.h>
#include <Freeze/TransactionalEvictorContext.h>

#include <IceUtil/IceUtil.h>

#include <cstdlib>
#include <map>
#include <memory>


using namespace std;
using namespace IceUtil;
using namespace Ice;

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

#if DB_VERSION_MAJOR != 4
   #error Freeze requires DB 4.x
#endif

#if DB_VERSION_MINOR < 3
void
dbErrCallback(const char* prefix, char* msg)
#else
void
dbErrCallback(const ::DbEnv* ignored, const char* prefix, const char* msg)
#endif    
{
    const Freeze::SharedDbEnv* env = reinterpret_cast<const Freeze::SharedDbEnv*>(prefix);
    assert(env != 0);
    
    Ice::Trace out(env->getCommunicator()->getLogger(), "Berkeley DB");
    out << "DbEnv \"" << env->getEnvName() << "\": " << msg;
}

StaticMutex _mapMutex = ICE_STATIC_MUTEX_INITIALIZER;
StaticMutex _refCountMutex = ICE_STATIC_MUTEX_INITIALIZER;  

typedef map<MapKey, Freeze::SharedDbEnv*> SharedDbEnvMap;
SharedDbEnvMap* sharedDbEnvMap;

}


Freeze::SharedDbEnvPtr 
Freeze::SharedDbEnv::get(const CommunicatorPtr& communicator,
                         const string& envName, DbEnv* env)
{
    StaticMutex::Lock lock(_mapMutex);

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
    auto_ptr<SharedDbEnv> result(new SharedDbEnv(envName, communicator, env));
    
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
    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.DbEnv");
        out << "closing database environment \"" << _envName << "\"";
    }

    //
    // Release catalog (to close it)
    //
    _catalog = 0;

    //
    // First terminate checkpointing thread
    //
    if(_thread != 0)
    {
        _thread->terminate();
        _thread = 0;
    }
    
    if(_envHolder.get() != 0)
    {
        try
        {
            _envHolder->close(0);
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}

void Freeze::SharedDbEnv::__incRef()
{
    IceUtil::StaticMutex::Lock lock(_refCountMutex);
    _refCount++;
}

void Freeze::SharedDbEnv::__decRef()
{
    IceUtil::StaticMutex::Lock lock(_refCountMutex);
    if(--_refCount == 0)
    {
        IceUtil::StaticMutex::TryLock mapLock(_mapMutex);
        if(!mapLock.acquired())
        {
            //
            // Reacquire mutex in proper order and check again
            //
            lock.release();
            mapLock.acquire();
            lock.acquire();
            if(_refCount > 0)
            {
                return;
            }
        }

        //
        // Remove from map
        //

        MapKey key;
        key.envName = _envName;
        key.communicator = _communicator;
        size_t one;
        one = sharedDbEnvMap->erase(key);
        assert(one == 1);

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
        if(txi->getConnectionI() == 0 || txi->getConnectionI()->dbEnv() == 0)
        {
            throw DatabaseException(__FILE__, __LINE__, "invalid transaction");
        }

        if(txi->getConnectionI()->dbEnv().get() != this)
        {
            throw DatabaseException(__FILE__, __LINE__, "the given transaction is bound to environment '" +
                                            txi->getConnectionI()->dbEnv()->_envName + "'");
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

    _trace = properties->getPropertyAsInt("Freeze.Trace.DbEnv");

    if(_env == 0)
    {
        _envHolder.reset(new DbEnv(0));
        _env = _envHolder.get();

        if(_trace >= 1)
        {
            Trace out(_communicator->getLogger(), "Freeze.DbEnv");
            out << "opening database environment \"" << envName << "\"";
        }

        string propertyPrefix = string("Freeze.DbEnv.") + envName;
        
        try
        {
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
            
            if(properties->getPropertyAsInt(propertyPrefix + ".DbRecoverFatal") != 0)
            {
                flags |= DB_RECOVER_FATAL | DB_CREATE;
            }
            else
            {
                flags |= DB_RECOVER | DB_CREATE;
            }
            
            if(properties->getPropertyAsIntWithDefault(propertyPrefix + ".DbPrivate", 1) != 0)
            {
                flags |= DB_PRIVATE;
            }
            
            
            //
            // Auto delete
            //
            bool autoDelete = (properties->getPropertyAsIntWithDefault(
                                   propertyPrefix + ".OldLogsAutoDelete", 1) != 0); 
            
            if(autoDelete)
            {
                _env->set_flags(DB_LOG_AUTOREMOVE, 1);
            }
            
            //
            // Threading
            // 
            flags |= DB_THREAD;
            
            string dbHome = properties->getPropertyWithDefault(
                propertyPrefix + ".DbHome", envName);
            
            _env->open(dbHome.c_str(), flags, FREEZE_DB_MODE);
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        
        //
        // Default checkpoint period is every 120 seconds
        //
        Int checkpointPeriod = properties->getPropertyAsIntWithDefault(
            propertyPrefix + ".CheckpointPeriod", 120);
        Int kbyte = properties->getPropertyAsIntWithDefault(propertyPrefix + ".PeriodicCheckpointMinSize", 0);
        
        if(checkpointPeriod > 0)
        {
            _thread = new CheckpointThread(*this, Time::seconds(checkpointPeriod), kbyte, _trace);
        }
    }

    //
    // Get catalog
    //
    _catalog = SharedDb::openCatalog(*this);
}



Freeze::CheckpointThread::CheckpointThread(SharedDbEnv& dbEnv, const Time& checkpointPeriod, Int kbyte, Int trace) : 
    _dbEnv(dbEnv), 
    _done(false), 
    _checkpointPeriod(checkpointPeriod), 
    _kbyte(kbyte),
    _trace(trace)
{
    start();
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
