// client.cpp,v 1.6 2003/11/02 23:27:22 dhinton Exp

#include "Roundtrip.h"
#include "Roundtrip_Handler.h"
#include "Worker_Thread.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/Sched_Params.h"
#include "ace/Stats.h"
#include "ace/OS_NS_errno.h"

#include "tao/Strategies/advanced_resource.h"
#include "tao/Messaging/Messaging.h"
#include <iostream>

#ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
#endif

using namespace std;

ACE_RCSID(Thread_Per_Connection_Latency, client, "client.cpp,v 1.6 2003/11/02 23:27:22 dhinton Exp")

const char *ior = "file://test.ior";
int do_shutdown = 1;

int
main (int argc, char *argv[])
{
  int priority =
    (ACE_Sched_Params::priority_min (ACE_SCHED_FIFO)
     + ACE_Sched_Params::priority_max (ACE_SCHED_FIFO)) / 2;
  // Enable FIFO scheduling, e.g., RT scheduling class on Solaris.

  if (ACE_OS::sched_params (ACE_Sched_Params (ACE_SCHED_FIFO,
                                              priority,
                                              ACE_SCOPE_PROCESS)) != 0)
    {
      if (ACE_OS::last_error () == EPERM)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "client (%P|%t): user is not superuser, "
                      "test runs in time-shared class\n"));
        }
      else
        ACE_ERROR ((LM_ERROR,
                    "client (%P|%t): sched_params failed\n"));
    }

  ACE_TRY_NEW_ENV
    {
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv, "" ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      CORBA::Object_var object =
        orb->resolve_initial_references ("PolicyCurrent" ACE_ENV_ARG_PARAMETER);
      ACE_CHECK;

      CORBA::PolicyCurrent_var policy_current =
        CORBA::PolicyCurrent::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
      ACE_CHECK;

      CORBA::Any scope_as_any;
      scope_as_any <<= Messaging::SYNC_WITH_TRANSPORT;

      CORBA::PolicyList policies(1);
      policies.length(1);
      policies[0] =
        orb->create_policy(Messaging::SYNC_SCOPE_POLICY_TYPE, 
			   scope_as_any
			   ACE_ENV_ARG_PARAMETER);
      ACE_CHECK;

      policy_current->set_policy_overrides (policies, CORBA::ADD_OVERRIDE ACE_ENV_ARG_PARAMETER);
      ACE_CHECK;

      policies[0]->destroy (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_CHECK;

      CORBA::Object_var poa_object =
        orb->resolve_initial_references("RootPOA" ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      PortableServer::POA_var root_poa =
        PortableServer::POA::_narrow (poa_object.in () ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      PortableServer::POAManager_var poa_manager =
        root_poa->the_POAManager (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;

    bool latency = false;
    bool oneway = false;
    bool twoway = false;
    bool ami = false;

    bool throughput = false;
    bool sendbytes = false;
    bool sendstrings = false;
    bool sendlongstrings = false;
    bool sendstructs = false;


    int i;
    for(i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], "latency") == 0)
	{
	    latency = true;
	}
	else if(strcmp(argv[i], "throughput") == 0)
	{
	    throughput = true;
	}
	else if(strcmp(argv[i], "oneway") == 0)
	{
	    oneway = true;
	}
	else if(strcmp(argv[i], "twoway") == 0)
	{
	    twoway = true;
	}
	else if(strcmp(argv[i], "ami") == 0)
	{
	    ami = true;
	}
	else if(strcmp(argv[i], "byte") == 0)
	{
	    sendbytes = true;
	}
	else if(strcmp(argv[i], "string") == 0)
	{
	    sendstrings = true;
	}
	else if(strcmp(argv[i], "longString") == 0)
	{
	    sendlongstrings = true;
	}
	else if(strcmp(argv[i], "struct") == 0)
	{
	    sendstructs = true;
	}
    }

    if(!latency && !throughput)
    {
        latency = true;
    }

    if(latency)
    {
        if(!oneway && !twoway)
	{
	    twoway = true;
	}
    }
    else if(throughput)
    {
        if(!sendbytes && !sendstrings && !sendlongstrings && !sendstructs)
	{
	    sendbytes = true;
	}
    }

      object =
        orb->string_to_object (ior ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      Test::Roundtrip_var roundtrip =
        Test::Roundtrip::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (CORBA::is_nil (roundtrip.in ()))
        {
          ACE_ERROR_RETURN ((LM_ERROR,
                             "Nil Test::Roundtrip reference <%s>\n",
                             ior),
                            1);
        }

      roundtrip->test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;

    if(latency)
    {
        Roundtrip_Handler *roundtrip_handler_impl;
	ACE_NEW_RETURN (roundtrip_handler_impl,
			Roundtrip_Handler (),
			1);
	PortableServer::ServantBase_var owner_transfer(roundtrip_handler_impl);

	Test::AMI_RoundtripHandler_var roundtrip_handler =
	  roundtrip_handler_impl->_this (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	poa_manager->activate (ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	if(ami)
	{
	    Worker_Thread *worker = new Worker_Thread(orb.in ());
	    worker->activate (THR_NEW_LWP | THR_JOINABLE, 1, 1);
	}

#ifdef WIN32
        struct _timeb tb;
        _ftime(&tb);
        long start = tb.time * 1000000 + tb.millitm * 1000;
#else
        struct timeval tv;
        gettimeofday(&tv, 0);
        long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

	int repetitions;
	if(oneway)
	{
	    repetitions = 500000;
	}
	else
	{
	    repetitions = 100000;
	}

        for (int i = 0; i != repetitions; ++i)
        {
	  if(oneway)
	  {
              roundtrip->test_oneway (ACE_ENV_SINGLE_ARG_PARAMETER);
              ACE_TRY_CHECK;
	  }
	  else if(ami)
	  {
	      roundtrip->sendc_test_method (roundtrip_handler.in ()
	      				    ACE_ENV_SINGLE_ARG_PARAMETER);
	      ACE_TRY_CHECK;

	      roundtrip_handler_impl->waitFinished();
	  }
	  else
	  {
              roundtrip->test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
              ACE_TRY_CHECK;
	  }
        }

	if(oneway)
	{
            roundtrip->test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
	}

#ifdef WIN32
        _ftime(&tb);
        float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
        gettimeofday(&tv, 0);
        float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif

        cout << (float) tm / repetitions << endl;
    }
    else
    {
        int i;

        Test::ByteSeq seq;
        seq.length(500000);

        Test::StringSeq stringSeq;
        stringSeq.length(50000);
        for(i = 0; i < 50000; ++i)
        {
            stringSeq[i] = CORBA::string_dup("hello");
        }

        Test::StringSeq longStringSeq;
        longStringSeq.length(5000);
        for(i = 0; i < 5000; ++i)
        {
            longStringSeq[i] = CORBA::string_dup("As far as the laws of mathematics refer to reality, they are not certain; and as far as they are certain, they do not refer to reality.");
	}
	
        Test::StringDoubleSeq stringDoubleSeq;
        stringDoubleSeq.length(50000);
        for(i = 0; i < 50000; ++i)
        {
            stringDoubleSeq[i].str = CORBA::string_dup("hello");
            stringDoubleSeq[i].d = 3.14;
        }

#ifdef WIN32
	struct _timeb tb;
	_ftime(&tb);
	long start = tb.time * 1000000 + tb.millitm * 1000;
#else
	struct timeval tv;
	gettimeofday(&tv, 0);
	long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
	    
        const int repetitions = 1000;
        for(i = 0; i < repetitions; ++i)
        {
	    if(sendbytes)
	    {
	        roundtrip->sendByteSeq(seq);
	    }
	    else if(sendstrings)
	    {
	        roundtrip->sendStringSeq(stringSeq);
	    }
	    else if(sendlongstrings)
	    {
	        roundtrip->sendStringSeq(longStringSeq);
	    }
	    else if(sendstructs)
	    {
	        roundtrip->sendStringDoubleSeq(stringDoubleSeq);
	    }
	}
#ifdef WIN32
	_ftime(&tb);
	float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
      	gettimeofday(&tv, 0);
      	float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif

        cout << tm / repetitions << endl;
      }			

      roundtrip->shutdown (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Exception caught:");
      return 1;
    }
  ACE_ENDTRY;

  return 0;
}
