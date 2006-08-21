// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Throughput.h>
#include <IcePerf/Data.h>

#include <iostream>

using namespace std;
using namespace Demo;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    bool byteTest = false;
    bool stringSeqTest = false;
    bool longStringSeqTest = false;
    bool structSeqTest = false;
    bool receive = false;
    bool zeroCopy = true;
    int i;
    for(i = 0; i < argc; i++)
    {
       if(strcmp(argv[i], "structSeq") == 0)
       {
	   structSeqTest = true;
       }
       else if(strcmp(argv[i], "noZeroCopy") == 0)
       {
	   zeroCopy = false;
       }
       else if(strcmp(argv[i], "longStringSeq") == 0)
       {
           longStringSeqTest = true;
       }
       else if(strcmp(argv[i], "stringSeq") == 0)
       {
           stringSeqTest = true;
       }
       else if(strcmp(argv[i], "byte") == 0)
       {
	   byteTest = true;
       }
       else if(strcmp(argv[i], "receive") == 0)
       {
	   receive = true;
       }
    }
    if(!byteTest && !stringSeqTest && !longStringSeqTest && !structSeqTest)
    {
        byteTest = true;
    }

    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Throughput.Throughput";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    ThroughputPrx throughput = ThroughputPrx::checkedCast(base);
    if(!throughput)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }
    ThroughputPrx throughputOneway = ThroughputPrx::uncheckedCast(throughput->ice_oneway());

    ByteSeq byteSeq(ByteSeqSize, 0);
#ifdef ICEE_USE_ARRAY_MAPPING
    pair<const Ice::Byte*, const Ice::Byte*> byteArr;
    byteArr.first = &byteSeq[0];
    byteArr.second = byteArr.first + byteSeq.size();
#endif

    StringSeq stringSeq(StringSeqSize, "hello");

    StringSeq longStringSeq(LongStringSeqSize, "As far as the laws of mathematics refer to reality, they are not certain; and as far as they are certain, they do not refer to reality.");

    StringDoubleSeq structSeq(StringDoubleSeqSize);
    for(i = 0; i < StringDoubleSeqSize; ++i)
    {
        structSeq[i].s = "hello";
	structSeq[i].d = 3.14;
    }
    Ice::Int byteSeqSize = ByteSeqSize * sizeof(Ice::Byte);
    Ice::Int stringSeqSize = StringSeqSize * stringSeq[0].size() * sizeof(stringSeq[0][0]);
    Ice::Int longSeqSize = LongStringSeqSize * longStringSeq[0].size() * sizeof(longStringSeq[0][0]);
    Ice::Int structSeqSize = StringDoubleSeqSize * sizeof(structSeq[0].d) + 
			     StringDoubleSeqSize * structSeq[0].s.size() * sizeof(structSeq[0].s[0]);
    Ice::Int payloadSize = 0;

    throughput->ice_ping(); // Initial ping to setup the connection.

    IceUtil::Time tm = IceUtil::Time::now();
    const int repetitions = 1000;
    if(byteTest)
    {
	payloadSize = byteSeqSize;
	if(!receive)
	{
	    if(zeroCopy)
	    {
		for(i = 0; i < repetitions; ++i)
		{
#ifdef ICEE_USE_ARRAY_MAPPING
		    throughput->sendByteSeq(byteArr);
#else
		    throughput->sendByteSeq(byteSeq);
#endif
		}
	    }
	    else
	    {
		for(i = 0; i < repetitions; ++i)
		{
#ifdef ICEE_USE_ARRAY_MAPPING
		    throughput->sendByteSeqNZ(byteSeq);
#else
		    throughput->sendByteSeq(byteSeq);
#endif
		}
	    }
	}
	else
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->recvByteSeq();
	    }
	}
    }
    else if(stringSeqTest)
    {
	payloadSize = stringSeqSize;
	if(!receive)
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->sendStringSeq(stringSeq);
	    }
	}
	else
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->recvStringSeq();
	    }
	}
    }
    else if(longStringSeqTest)
    {
	payloadSize = longSeqSize;
	if(!receive)
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->sendStringSeq(longStringSeq);
	    }
	}
	else
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->recvLongStringSeq();
	    }
	}
    }
    else if(structSeqTest)
    {
	payloadSize = structSeqSize;
	if(!receive)
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->sendStructSeq(structSeq);
	    }
	}
	else
	{
	    for(i = 0; i < repetitions; ++i)
	    {
		throughput->recvStructSeq();
	    }
	}
    }

    IcePerf::TestPrinter formatter;
    formatter.fmt(cout, "Ice", "throughput", IceUtil::Time::now() - tm, repetitions, payloadSize, argc, argv);

    throughput->shutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties();
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex.ice_name() << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex.ice_name() << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
