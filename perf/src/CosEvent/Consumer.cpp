// Consumer.cpp,v 1.3 2002/01/29 20:20:46 okellogg Exp

#include "Consumer.h"
#include "orbsvcs/CosEventChannelAdminS.h"
#include "PerfC.h"
#include "ace/Date_Time.h"

#include <iostream>
#include <math.h>

using namespace std;

int
main (int argc, char* argv[])
{
    Consumer consumer;
    return consumer.run (argc, argv);
}

// ****************************************************************

Consumer::Consumer (void) :
    _payload(false),
    _startTime(0),
    _stopTime(0),
    _nPublishers(0),
    _nStartedPublishers(0),
    _nStoppedPublishers(0)
{
}

int
Consumer::run (int argc, char* argv[])
{
    int repetitions = 10000;
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
	if(strcmp(argv[i], "-c") == 0)
	{
	    _nPublishers = atoi(argv[++i]);
	}
	else if(strlen(argv[i]) > 3 && argv[i][0] == 'I' && argv[i][1] == 'O' && argv[i][2] == 'R')
	{
	    ior = strdup(argv[i]);
	}
    }

    ACE_DECLARE_NEW_CORBA_ENV;
    ACE_TRY
    {
	CORBA::ORB_var orb = CORBA::ORB_init (argc, argv, "" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	this->orb_ = orb.in ();
	
	if(argc <= 1)
	{
	    ACE_ERROR ((LM_ERROR, "Usage: Consumer <event_channel_ior>\n"));
	    return 1;
	}

	CORBA::Object_var object = orb->resolve_initial_references ("RootPOA" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	PortableServer::POA_var poa = PortableServer::POA::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	PortableServer::POAManager_var poa_manager = poa->the_POAManager (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	poa_manager->activate (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	object = orb->string_to_object (ior ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	    
	CosEventChannelAdmin::EventChannel_var event_channel = 
	    CosEventChannelAdmin::EventChannel::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	CosEventChannelAdmin::ConsumerAdmin_var consumer_admin = 
	    event_channel->for_consumers (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	CosEventChannelAdmin::ProxyPushSupplier_var supplier =
	    consumer_admin->obtain_push_supplier (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	CosEventComm::PushConsumer_var consumer = this->_this (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	supplier->connect_push_consumer (consumer.in () ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	cout << "Consumer ready" << endl;

	orb->run ();
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
    long long time = 0;
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

    if(time > 0 && _nStartedPublishers == _nPublishers)
    {
	add(time);
    }
    else if(time == 0)
    {
	started();
    }
    else if(time < 0)
    {
	if(stopped())
	{
	    this->orb_->shutdown (0 ACE_ENV_ARG_PARAMETER);
	}
    }
}

void
Consumer::started()
{
    if(++_nStartedPublishers == _nPublishers)
    {
	timeval tv;
	gettimeofday(&tv, 0);
	_startTime = tv.tv_sec * static_cast<long long>(1000000) + tv.tv_usec;
    }
}

bool
Consumer::stopped()
{
    if(_nStoppedPublishers == 0)
    {
	if(_nStartedPublishers < _nPublishers)
	{
	    cerr << "Some publishers are already finished while others aren't even started" << endl;
	}

	timeval tv;
	gettimeofday(&tv, 0);
	_stopTime = tv.tv_sec * static_cast<long long>(1000000) + tv.tv_usec;
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
Consumer::add(long long time)
{
    timeval tv;
    gettimeofday(&tv, 0);
    _results.push_back(tv.tv_sec * static_cast<long long>(1000000) + tv.tv_usec - time);
}

void
Consumer::disconnect_push_consumer(ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Consumer::calc()
{
    double total = 0.0;
    for(vector<int>::const_iterator p = _results.begin(); p != _results.end(); ++p)
    {
	total += *p;
    }
    double mean = total / _results.size();
    
    double deviation;
    total = 0.0;
    for(vector<int>::const_iterator p = _results.begin(); p != _results.end(); ++p)
    {
	total = (*p - mean) * (*p - mean);
    }
    deviation = sqrt(total / (_results.size() - 1));
    
    cout << mean << " " << deviation << " " 
	 << static_cast<double>(_results.size()) / (_stopTime - _startTime) * 1000000.0 
	 << " " << flush;

    _results.clear();    
    _nStartedPublishers = 0;
    _nStoppedPublishers = 0;
}

// ****************************************************************

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
