// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Consumer.h>
#include <WorkerThread.h>
#include <ace/Thread_Mutex.h>
#include <orbsvcs/CosEventChannelAdminC.h>
#include <PerfC.h>
#include <ace/Date_Time.h>

#include <iostream>
#include <math.h>

#include <IceUtil/Time.h>

using namespace std;

Consumer::Consumer() :
    _payload(false),
    _startTime(0),
    _stopTime(0),
    _nPublishers(0),
    _nStartedPublishers(0),
    _nStoppedPublishers(0),
    _payloadSize(1)
{
}

int
Consumer::run(int argc, char* argv[])
{
    int repetitions = 10000;
    int nthreads = 1;
    char* ior = 0;
    _payload = false;
    _nPublishers = 1;
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-r") == 0)
        {
            repetitions = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-w") == 0)
        {
            _payload = true;
        }
        else if(strcmp(argv[i], "-c") == 0)
        {
            _nPublishers = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            nthreads = atoi(argv[++i]);
        }
        else if(strlen(argv[i]) > 3 && argv[i][0] == 'I' && argv[i][1] == 'O' && argv[i][2] == 'R')
        {
            ior = strdup(argv[i]);
        }
    }

    _nExpectedTicks = repetitions * _nPublishers;
    _results.reserve(_nExpectedTicks);

    ACE_DECLARE_NEW_CORBA_ENV;
    ACE_TRY
    {
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
        
        _orb = orb.in();
        
        if(argc <= 1)
        {
            ACE_ERROR ((LM_ERROR, "Usage: Consumer <event_channel_ior>\n"));
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

        WorkerThread worker(orb.in());
        worker.activate(THR_NEW_LWP | THR_JOINABLE, nthreads, 1);
        
        object = orb->string_to_object(ior ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;
            
        CosEventChannelAdmin::EventChannel_var event_channel = 
            CosEventChannelAdmin::EventChannel::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        CosEventChannelAdmin::ConsumerAdmin_var consumer_admin = 
            event_channel->for_consumers(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;

        CosEventChannelAdmin::ProxyPushSupplier_var supplier =
            consumer_admin->obtain_push_supplier(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;

        CosEventComm::PushConsumer_var consumer = _this(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;

        supplier->connect_push_consumer(consumer.in() ACE_ENV_ARG_PARAMETER);
        ACE_TRY_CHECK;

        cout << "Consumer ready" << endl;

        worker.thr_mgr()->wait();
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Consumer::run");
        return 1;
    }
    ACE_ENDTRY;
    return 0;
}

void
Consumer::push(const CORBA::Any& any ACE_ENV_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC ((CORBA::SystemException))
{
    ACE_Guard<ACE_Thread_Mutex> sync(_lock);

    CORBA::LongLong time = 0;
    if(!_payload)
    {
        any >>= time;
    }
    else
    {
        const Perf::Event* e;
        any >>= e;
        time = e->time;
    }

    if(time > 0)
    {
        add(time);
    }
    else if(time == 0)
    {
        if(_payload)
        {
            const Perf::Event* e;
            any >>= e;
            _payloadSize = 0;
            _payloadSize += sizeof(e->time);
            _payloadSize += sizeof(e->e);
            _payloadSize += sizeof(e->i);
            _payloadSize += sizeof(e->s.e);
            _payloadSize += sizeof(e->s.d);
            _payloadSize += strlen(e->s.s);
        }
        started();
    }
    else if(time == -1)
    {
        if(stopped())
        {
            _orb->shutdown(0 ACE_ENV_ARG_PARAMETER);
        }
    }
    else if(time < 0)
    {
        cerr << "time < 0: " << time << endl;
    }
}

void
Consumer::started()
{
    if(++_nStartedPublishers == _nPublishers)
    {
        _startTime = IceUtil::Time::now().toMicroSeconds(); 
    }
}

bool
Consumer::stopped()
{
    //
    // It is *very* important to record the stop time be recorded once the
    // first publisher has stopped. The stop time is used to calculate the
    // throughput values. We are looking for a throughput value that
    // reflects the service's ability to process events when fully loaded.
    // If we keep counting after the first stopped publisher we may end up
    // including data that skews the results, possibly for better, possibly
    // for worse.
    //
    if(_nStoppedPublishers == 0)
    {
        _stopTime = IceUtil::Time::now().toMicroSeconds();
    }
    if(_nStartedPublishers < _nPublishers)
    {
        cerr << "Some publishers are already finished while others aren't even started" << endl;
        cerr << _nPublishers << " " << _nStartedPublishers << " " << _nStoppedPublishers << endl;
        cerr << _startTime - _stopTime << " " << _results.size() << endl;
    }

    if(++_nStoppedPublishers == _nPublishers)
    {
        calc();
        return true;
    }
    else
    {
        return false;
    }
}

void
Consumer::add(CORBA::LongLong time)
{
    //
    // Do *NOT* add record more events after the first publisher has
    // stopped. 
    //
    if(_nStartedPublishers == _nPublishers && _nStoppedPublishers == 0)
    {
        //
        // Save end to end time in microseconds.
        //
#ifdef WIN32
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        CORBA::LongLong interval = t.QuadPart - time;
        QueryPerformanceFrequency(&t);
        double t = (double)interval / t.QuadPart;
        interval = (CORBA::LongLong)(t * 1000000); // Convert to microseconds.
        _results.push_back(static_cast<long>(interval));
#else
        _results.push_back((static_cast<long>(IceUtil::Time::now().toMicroSeconds() - time)));
#endif
    }
}

void
Consumer::disconnect_push_consumer(ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Consumer::calc()
{
    double originalSize = _results.size();
    if(_results.empty())
    {
        cout << "{ 'latency': -1.0, 'throughput': -1.0, 'deviation': -1.0 }" << flush;
    }

    //
    // Only keep the N/2 best results
    //
    sort(_results.begin(), _results.end());
    _results.resize(_results.size() / 2);

    double total = 0;
    {
        for(vector<long>::const_iterator i = _results.begin(); i != _results.end(); ++i)
        {
            total += *i;
        }
    }
    double mean = total / _results.size();
    
    double deviation;
    double x = 0.0;
    {
        for(vector<long>::const_iterator i = _results.begin(); i != _results.end(); ++i)
        {
            x += (*i - mean) * (*i - mean);
        }
    }
    deviation = sqrt(x / (_results.size() - 1));

    //
    // If the expected number of results falls below a the 90% threshold,
    // we should display a warning. There isn't much else that can be done
    // fairly. What data there is should still be accurate --if not
    // statistically sound-- because it is based on what actually occurred,
    // not expected parameters. 
    //
    if(originalSize < (_nExpectedTicks * 0.90))
    {
        cerr << "WARNING: Less than 90% of the expected ticks were used for the test. " <<
            _nExpectedTicks << " events were expected, but only " << originalSize << " were received.\n" << endl;
        cerr << "The results are based on a smaller sample size and comparisons with other tests\n"
                "may not be fair." << endl; 
    }

    cout << "{ 'latency' : " << mean / 1000 << ", 'deviation' : " << deviation << ", 'throughput' : " <<
        ((double)(originalSize * _payloadSize) / (1024^2)) / (_stopTime - _startTime) * 1000000.0 << 
        ", 'repetitions': " << originalSize << ", 'payload': " << _payloadSize << "}" << endl;

    _results.clear();    
    _nStartedPublishers = 0;
    _nStoppedPublishers = 0;
}

int
main(int argc, char* argv[])
{
    Consumer consumer;
    return consumer.run(argc, argv);
}
