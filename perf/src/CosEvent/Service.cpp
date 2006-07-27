// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <WorkerThread.h>
#include <orbsvcs/CosEvent/CEC_EventChannel.h>
#include <orbsvcs/CosEvent/CEC_Default_Factory.h>
#include <ace/Get_Opt.h>
#include <ace/OS_NS_stdio.h>

const char *ior_output_file = "ec.ior";

class EventChannel : public TAO_CEC_EventChannel
{
public:
    
    EventChannel(CORBA::ORB* orb, 
		 const TAO_CEC_EventChannel_Attributes& attr, 
		 TAO_CEC_Factory* factory = 0, 
		 int own_factory = 0) :
	TAO_CEC_EventChannel(attr, factory, own_factory),
	_orb(CORBA::ORB::_duplicate(orb))
    {
    }

    void destroy(ACE_ENV_SINGLE_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException))
    {
	TAO_CEC_EventChannel::destroy(ACE_ENV_SINGLE_ARG_PARAMETER);
	_orb->shutdown(0 ACE_ENV_ARG_PARAMETER);	    
    }

private:

    CORBA::ORB_var _orb;
};

int
main(int argc, char* argv[])
{
    TAO_CEC_Default_Factory::init_svcs();
    
    int nthreads = 1;
    for(int i = 1; i < argc; i++)
    {
	if(strcmp(argv[i], "-n") == 0)
	{
	    nthreads = atoi(argv[++i]);
	}
    }

    ACE_DECLARE_NEW_CORBA_ENV;
    ACE_TRY
    {
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CORBA::Object_var object = orb->resolve_initial_references("RootPOA" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	PortableServer::POA_var poa = PortableServer::POA::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	PortableServer::POAManager_var poa_manager = poa->the_POAManager(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	poa_manager->activate(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	TAO_CEC_EventChannel_Attributes attributes(poa.in(), poa.in());
	
	EventChannel ec_impl(orb.in(), attributes);
	ec_impl.activate(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CosEventChannelAdmin::EventChannel_var event_channel = ec_impl._this(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CORBA::String_var ior = orb->object_to_string(event_channel.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	ACE_DEBUG ((LM_DEBUG, "Activated as <%s>\n", ior.in()));
	
	// If the ior_output_file exists, output the ior to it
	if(ior_output_file != 0)
	{
	    FILE *output_file= ACE_OS::fopen(ior_output_file, "w");
	    if(output_file == 0)
		ACE_ERROR_RETURN ((LM_ERROR, "Cannot open output file for writing IOR: %s", ior_output_file), 1);
	    ACE_OS::fprintf(output_file, "%s", ior.in());
	    ACE_OS::fclose(output_file);
	}
	
	WorkerThread worker(orb.in());
	worker.activate(THR_NEW_LWP | THR_JOINABLE, nthreads, 1);
	worker.thr_mgr()->wait();
    }
    ACE_CATCHANY
    {
	ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Service");
	return 1;
    }
    ACE_ENDTRY;
    return 0;
}
