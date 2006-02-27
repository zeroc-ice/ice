// server.cpp,v 1.5 2003/11/02 23:27:22 dhinton Exp

#include "Roundtrip.h"
#include "Worker_Thread.h"
#include "ace/Get_Opt.h"
#include "ace/Sched_Params.h"
#include "ace/OS_NS_errno.h"

#include "tao/Strategies/advanced_resource.h"

#include <iostream>

ACE_RCSID(Thread_Pool_Latency, server, "server.cpp,v 1.5 2003/11/02 23:27:22 dhinton Exp")

const char *ior_output_file = "test.ior";

int
main(int argc, char *argv[])
{
    //
    // NOTE: The following needs to be enabled, otherwise the server
    // doesn't properly shutdown...
    //
    int priority = (ACE_Sched_Params::priority_min(ACE_SCHED_FIFO) +
		    ACE_Sched_Params::priority_max(ACE_SCHED_FIFO)) / 2;
    priority = ACE_Sched_Params::next_priority(ACE_SCHED_FIFO, priority);
    if(ACE_OS::sched_params(ACE_Sched_Params(ACE_SCHED_FIFO, priority, ACE_SCOPE_PROCESS)) != 0)
    {
	if(ACE_OS::last_error() == EPERM)
        {
	    ACE_DEBUG((LM_DEBUG, "server(%P|%t): user is not superuser, ""test runs in time-shared class\n"));
        }
	else
	{
	    ACE_ERROR((LM_ERROR, "server(%P|%t): sched_params failed\n"));
	}
    }

    ACE_TRY_NEW_ENV
    {
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CORBA::Object_var poa_object = orb->resolve_initial_references("RootPOA" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	if(CORBA::is_nil(poa_object.in()))
	{
	    ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) Unable to initialize the POA.\n"), 1);
	}

	PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	PortableServer::POAManager_var poa_manager = root_poa->the_POAManager(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	int i;
	int numthreads = 0;
	for(i = 0; i < argc; ++i)
	{
	    if(strcmp(argv[i], "threadPool") == 0)
	    {
		++i;
		if(i == argc)
		{
		    ACE_ERROR_RETURN((LM_ERROR, "missing number of threads\n"), 1);
		}
		numthreads = atoi(argv[i]);
	    }
	}

	Roundtrip *roundtrip_impl;
	ACE_NEW_RETURN(roundtrip_impl, Roundtrip(orb.in()), 1);
	PortableServer::ServantBase_var owner_transfer(roundtrip_impl);

	Test::Roundtrip_var roundtrip = roundtrip_impl->_this(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	CORBA::String_var ior = orb->object_to_string(roundtrip.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	// If the ior_output_file exists, output the ior to it
	FILE *output_file= ACE_OS::fopen(ior_output_file, "w");
	if(output_file == 0)
	{
	    ACE_ERROR_RETURN((LM_ERROR, "Cannot open output file for writing IOR: %s", ior_output_file), 1);
	}
	ACE_OS::fprintf(output_file, "%s", ior.in());
	ACE_OS::fclose(output_file);
	
	poa_manager->activate(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	Worker_Thread worker(orb.in());
	
	std::cout << "POA ready" << std::endl;

	if(numthreads > 0)
	{
	    worker.activate(THR_NEW_LWP | THR_JOINABLE, numthreads, 1);
	    worker.thr_mgr()->wait();
	}
	else
	{
	    orb->run(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;
	}

	root_poa->destroy(1, 1 ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	orb->destroy(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
	ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "Exception caught:");
	return 1;
    }
    ACE_ENDTRY;

    return 0;
}
