// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <tao/Messaging/Messaging.h>
#include <iostream>
#include <Roundtrip.h>
#include <Roundtrip_Handler.h>
#include <WorkerThread.h>

#include <IcePerf/Data.h>

using namespace std;

int do_shutdown = 1;

int
main(int argc, char *argv[])
{
    ACE_TRY_NEW_ENV
    {
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;
	
	CORBA::Object_var object = orb->resolve_initial_references("PolicyCurrent" ACE_ENV_ARG_PARAMETER);
	ACE_CHECK;

	CORBA::PolicyCurrent_var policy_current = CORBA::PolicyCurrent::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
	ACE_CHECK;

	CORBA::Any scope_as_any;
	scope_as_any <<= Messaging::SYNC_WITH_TRANSPORT;

	CORBA::PolicyList policies(1);
	policies.length(1);
	policies[0] = orb->create_policy(Messaging::SYNC_SCOPE_POLICY_TYPE, scope_as_any ACE_ENV_ARG_PARAMETER);
	ACE_CHECK;

	policy_current->set_policy_overrides(policies, CORBA::ADD_OVERRIDE ACE_ENV_ARG_PARAMETER);
	ACE_CHECK;

	policies[0]->destroy(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_CHECK;

	CORBA::Object_var poa_object = orb->resolve_initial_references("RootPOA" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	PortableServer::POAManager_var poa_manager = root_poa->the_POAManager(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	bool latency = false;
	bool oneway = false;
	bool twoway = false;
	bool ami = false;

	bool throughput = false;
	bool sendBytes = false;
	bool sendStrings = false;
	bool sendLongStrings = false;
	bool sendStructs = false;
	long byteSeqSize = 0;
	Ice::Int payloadSize = 0;
	int repetitions;

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
		sendBytes = true;
	    }
	    else if(strcmp(argv[i], "string") == 0)
	    {
		sendStrings = true;
	    }
	    else if(strcmp(argv[i], "longString") == 0)
	    {
		sendLongStrings = true;
	    }
	    else if(strcmp(argv[i], "struct") == 0)
	    {
		sendStructs = true;
	    }
	    else if(strncmp(argv[i], "--payload=", strlen("--payload=")) == 0)
	    {
		byteSeqSize = strtol(argv[i] + strlen("--payload="), 0, 10);
		if(errno == ERANGE)
		{
		    cerr << argv[0] << ": payload argument range error: " << argv[i] << endl;
		    return EXIT_FAILURE;
		}
	    }
	}

	//
	// Defaults to latency test if neither was selected.
	//
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
	    if(!sendBytes && !sendStrings && !sendLongStrings && !sendStructs)
	    {
		sendBytes = true;
	    }
	}

	object = orb->string_to_object("file://test.ior" ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	Test::Roundtrip_var roundtrip = Test::Roundtrip::_narrow(object.in() ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	if(CORBA::is_nil(roundtrip.in()))
        {
	    ACE_ERROR_RETURN((LM_ERROR, "Nil Test::Roundtrip reference <%s>\n", "file://test.ior"), 1);
        }

	roundtrip->test_method(ACE_ENV_SINGLE_ARG_PARAMETER);
	ACE_TRY_CHECK;

	if(latency)
	{
	    Roundtrip_Handler *roundtrip_handler_impl;
	    ACE_NEW_RETURN(roundtrip_handler_impl, Roundtrip_Handler(), 1);
	    PortableServer::ServantBase_var owner_transfer(roundtrip_handler_impl);

	    Test::AMI_RoundtripHandler_var roundtrip_handler = 
		roundtrip_handler_impl->_this(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;

	    poa_manager->activate(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;

	    if(ami)
	    {
		WorkerThread *worker = new WorkerThread(orb.in());
		worker->activate(THR_NEW_LWP | THR_JOINABLE, 1, 1);
	    }

	    IceUtil::Time start = IceUtil::Time::now();

	    if(oneway)
	    {
		repetitions = 500000;
	    }
	    else
	    {
		repetitions = 100000;
	    }

	    Test::ByteSeq seq;
	    if(byteSeqSize > 0)
	    {
		seq.length(byteSeqSize);
		for(int i = 0; i < byteSeqSize; ++i)
		{
		    seq[i] = '0' + (char)(i % 10);
		}
		payloadSize = seq.length() * sizeof(seq[0]);
	    }

	    for(int i = 0; i != repetitions; ++i)
	    {
		if(oneway)
		{
		    if(byteSeqSize > 0)
		    {
			roundtrip->test_oneway_with_data(seq ACE_ENV_ARG_DECL_WITH_DEFAULTS);
			ACE_TRY_CHECK;
		    }
		    else
		    {
			roundtrip->test_oneway(ACE_ENV_SINGLE_ARG_PARAMETER);
			ACE_TRY_CHECK;
		    }
		}
		else if(ami)
		{
		    if(byteSeqSize > 0)
		    {
        
			roundtrip->sendc_sendByteSeq(roundtrip_handler.in(), seq ACE_ENV_ARG_DECL_WITH_DEFAULTS);
			ACE_TRY_CHECK;
			roundtrip_handler_impl->waitFinished();
		    }
		    else
		    {
			roundtrip->sendc_test_method(roundtrip_handler.in() ACE_ENV_SINGLE_ARG_PARAMETER);
			ACE_TRY_CHECK;
			roundtrip_handler_impl->waitFinished();
		    }
		}
		else if(byteSeqSize > 0)
		{
		    roundtrip->sendByteSeq(seq ACE_ENV_SINGLE_ARG_PARAMETER);
		    ACE_TRY_CHECK;
		}
		else
		{
		    roundtrip->test_method(ACE_ENV_SINGLE_ARG_PARAMETER);
		    ACE_TRY_CHECK;
		}
	    }

	    if(oneway)
	    {
		roundtrip->test_method(ACE_ENV_SINGLE_ARG_PARAMETER);
	    }

	    IcePerf::TestPrinter formatter;
	    formatter.fmt(cout, "TAO", "latency", IceUtil::Time::now() - start, repetitions, payloadSize, argc, argv);

	    roundtrip->shutdown(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;

	    orb->destroy(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;
	}
	else
	{
	    repetitions = 1000;
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
		longStringSeq[i] = CORBA::string_dup("As far as the laws of mathematics refer to reality, "
						     "they are not certain; and as far as they are certain, "
						     "they do not refer to reality.");
	    }
	
	    Test::StringDoubleSeq stringDoubleSeq;
	    stringDoubleSeq.length(50000);
	    for(i = 0; i < 50000; ++i)
	    {
		stringDoubleSeq[i].str = CORBA::string_dup("hello");
		stringDoubleSeq[i].d = 3.14;
	    }

	    IceUtil::Time start = IceUtil::Time::now();

	    if(sendBytes)
	    {
		payloadSize = seq.length() * sizeof(seq[0]);
	    }
	    else if(sendStrings)
	    {
		payloadSize = stringSeq.length() * strlen(stringSeq[0]) * sizeof(stringSeq[0][0]);
	    }
	    else if(sendLongStrings)
	    {
		payloadSize = longStringSeq.length() * strlen(longStringSeq[0]) * sizeof(longStringSeq[0][0]);
	    }
	    else if(sendStructs)
	    {
		payloadSize = stringDoubleSeq.length() * (strlen(stringDoubleSeq[0].str) * sizeof(stringDoubleSeq[0].str[0]) + 
							  sizeof(stringDoubleSeq[0].d));
	    }
	    else
	    {
		assert(false);
	    }
	    
	    for(i = 0; i < repetitions; ++i)
	    {
		if(sendBytes)
		{
		    roundtrip->sendByteSeq(seq);
		}
		else if(sendStrings)
		{
		    roundtrip->sendStringSeq(stringSeq);
		}
		else if(sendLongStrings)
		{
		    roundtrip->sendStringSeq(longStringSeq);
		}
		else if(sendStructs)
		{
		    roundtrip->sendStringDoubleSeq(stringDoubleSeq);
		}
	    }

	    IcePerf::TestPrinter formatter;
	    formatter.fmt(cout, "TAO", "throughput", IceUtil::Time::now() - start, repetitions, payloadSize, argc, argv);

	    roundtrip->shutdown(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;

	    orb->destroy(ACE_ENV_SINGLE_ARG_PARAMETER);
	    ACE_TRY_CHECK;
	}			
    }
    ACE_CATCHANY
    {
	ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "Exception caught:");
	return 1;
    }
    ACE_ENDTRY;

    return 0;
}
