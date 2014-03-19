// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Supplier.h>
#include <WorkerThread.h>
#include <orbsvcs/CosEventChannelAdminC.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Date_Time.h>
#include <ace/Thread_Semaphore.h>
#include <PerfS.h>
#include <SyncS.h>

#include <IceUtil/Time.h>

#include <iostream>
#include <string>

using namespace std;

//
// Time types do not give us sufficient resolution on Windows, so the
// high resolution performance counters are used.
//
CORBA::LongLong
getTick()
{
#ifdef WIN32
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
#else
    return IceUtil::Time::now().toMicroSeconds();
#endif
}

class Sync_impl : public POA_Perf::Sync, public PortableServer::RefCountServantBase
{
public:

    Sync_impl() : 
        _semaphore(0)
    {
    }
    
    void
    wait()
    {
        _semaphore.acquire();
    }

    virtual void
    notify()
        ACE_THROW_SPEC ((CORBA::SystemException))
    {
        _semaphore.release();
    }

private:
    
    ACE_Thread_Semaphore _semaphore;
};

int
Supplier::run(int argc, char* argv[])
{
    int period = 0;
    int repetitions = 10000;
    bool payload = false;
    CORBA::String_var ior;
    int nthreads = 1;
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-p") == 0)
        {
            period = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-r") == 0)
        {
            repetitions = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-w") == 0)
        {
            payload = true;
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            nthreads = atoi(argv[++i]);
        }
        else if(strlen(argv[i]) > 3 && argv[i][0] == 'I' && argv[i][1] == 'O' && argv[i][2] == 'R')
        {
            ior = CORBA::string_dup(argv[i]);
        }
    }

    ACE_DECLARE_NEW_CORBA_ENV;
    ACE_TRY
    {
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        if(argc <= 1)
        {
            ACE_ERROR ((LM_ERROR, "Usage: Supplier <event_channel_ior>\n"));
            return 1;
        }
        
        CORBA::Object_var object = orb->resolve_initial_references("RootPOA" ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        PortableServer::POA_var poa = PortableServer::POA::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        PortableServer::POAManager_var poa_manager = poa->the_POAManager(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
        poa_manager->activate(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;

        object = orb->string_to_object(ior.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        CosEventChannelAdmin::EventChannel_var event_channel = 
            CosEventChannelAdmin::EventChannel::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        CosEventChannelAdmin::SupplierAdmin_var supplier_admin =
            event_channel->for_suppliers(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;

        CosEventChannelAdmin::ProxyPushConsumer_var consumer =
            supplier_admin->obtain_push_consumer(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        CosEventComm::PushSupplier_var supplier = _this(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        consumer->connect_push_supplier(supplier.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        //
        // Start the ORB.
        //
        WorkerThread worker(orb.in());
        worker.activate(THR_NEW_LWP | THR_JOINABLE, nthreads, 1);
        
        Sync_impl* syncImpl = new Sync_impl;
        PortableServer::ServantBase_var servant = syncImpl;

        //
        // Note that the Sync servant will be instantiated on the default
        // ORB and default POA.
        //
        CORBA::Object_var obj = syncImpl->_this();
        Perf::Sync_var sync = Perf::Sync::_narrow(obj);

        ACE_Time_Value sleep_time(0, period * 1000);

        ior = orb->object_to_string(sync.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        cout << ior.in() << endl;
        cout << "Supplier ready" << endl;
        syncImpl->wait();
        
        if(!payload)
        {
            {
                CORBA::Any event;
                event <<= CORBA::LongLong(0);
                consumer->push(event ACE_ENV_ARG_PARAMETER);
            }
            for(int i = 0; i < repetitions; ++i)
            {
                CORBA::Any event;
                CORBA::LongLong start = getTick();
                event <<= start;
                
                consumer->push(event ACE_ENV_ARG_PARAMETER);
                ACE_TRY_CHECK;
                if(period > 0)
                {
                    ACE_OS::sleep(sleep_time);
                }
            }
            {
                CORBA::Any event;
                event <<= CORBA::LongLong(-1);
                consumer->push(event ACE_ENV_ARG_PARAMETER);
            }
        }
        else
        {
            {
                CORBA::Any event;
                Perf::Event e;
                e.time = 0;
                event <<= e;
                consumer->push(event ACE_ENV_ARG_PARAMETER);
            }
            for(int i = 0; i < repetitions; ++i)
            {
                Perf::Event e;
                e.e = Perf::A;
                e.i = 10;
                e.s.s = "TEST";
                e.time = getTick();
                CORBA::Any event;
                event <<= e;    
                consumer->push(event ACE_ENV_ARG_PARAMETER);
                ACE_TRY_CHECK;
                if(period > 0)
                {
                    ACE_OS::sleep(sleep_time);
                }
            }
            {
                CORBA::Any event;
                Perf::Event e;
                e.time = -1;
                event <<= e;
                consumer->push(event ACE_ENV_ARG_PARAMETER);
            }
        }
        
        // Disconnect from the EC
        consumer->disconnect_push_consumer(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;

        // Deactivate this object...
        PortableServer::ObjectId_var id = poa->servant_to_id(this ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        poa->deactivate_object(id.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        // Destroy the POA
        poa->destroy(1, 0 ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        orb->shutdown(1);
        worker.thr_mgr()->wait();
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Supplier::run");
        return 1;
    }
    ACE_ENDTRY;
    return 0;
}

void
Supplier::disconnect_push_supplier(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC ((CORBA::SystemException))
{
}

int
main(int argc, char* argv[])
{
    Supplier supplier;
    return supplier.run(argc, argv);
}
