// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Roundtrip.h>
#include <WorkerThread.h>
#include <iostream>

int
main(int argc, char *argv[])
{
    ACE_TRY_NEW_ENV
    {
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CORBA::Object_var poaObject = orb->resolve_initial_references("RootPOA" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	if(CORBA::is_nil(poaObject.in()))
	{
	    ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) Unable to initialize the POA.\n"), 1);
	}

	PortableServer::POA_var rootPOA = PortableServer::POA::_narrow(poaObject.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	PortableServer::POAManager_var poaManager = rootPOA->the_POAManager(ACE_ENV_SINGLE_ARG_PARAMETER);
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

	Roundtrip *roundtripImpl;
	ACE_NEW_RETURN(roundtripImpl, Roundtrip(orb.in()), 1);
	PortableServer::ServantBase_var ownerTransfer(roundtripImpl);

	Test::Roundtrip_var roundtrip = roundtripImpl->_this(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	CORBA::String_var ior = orb->object_to_string(roundtrip.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	//
	// If the iorFile exists, output the ior to it
	//
	FILE *output_file= ACE_OS::fopen("test.ior", "w");
	if(output_file == 0)
	{
	    ACE_ERROR_RETURN((LM_ERROR, "Cannot open output file for writing IOR: %s", "test.ior"), 1);
	}

	ACE_OS::fprintf(output_file, "%s", ior.in());
	ACE_OS::fclose(output_file);
	
	poaManager->activate(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	WorkerThread worker(orb.in());
	
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

	rootPOA->destroy(1, 1 ACE_ENV_ARG_PARAMETER);
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
