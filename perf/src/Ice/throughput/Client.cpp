// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Throughput.h>

using namespace std;
using namespace Demo;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    bool byteTest = false;
    bool stringSeqTest = false;
    bool longStringSeqTest = false;
    bool structSeqTest = false;
    int i;
    for(i = 0; i < argc; i++)
    {
       if(strcmp(argv[i], "structSeq") == 0)
       {
	   structSeqTest = true;
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

    StringSeq stringSeq(StringSeqSize, "hello");

    StringSeq longStringSeq(LongStringSeqSize, "As far as the laws of mathematics refer to reality, they are not certain; and as far as they are certain, they do not refer to reality.");

    StringDoubleSeq structSeq(StringDoubleSeqSize);
    for(i = 0; i < StringDoubleSeqSize; ++i)
    {
        structSeq[i].s = "hello";
	structSeq[i].d = 3.14;
    }

    throughput->ice_ping(); // Initial ping to setup the connection.

    IceUtil::Time tm = IceUtil::Time::now();
    const int repetitions = 1000;
    if(byteTest)
    {
	for(i = 0; i < repetitions; ++i)
	{
	    throughput->sendByteSeq(byteSeq);
	}
    }
    else if(stringSeqTest)
    {
	for(i = 0; i < repetitions; ++i)
	{
	    throughput->sendStringSeq(stringSeq);
	}
    }
    else if(longStringSeqTest)
    {
	for(i = 0; i < repetitions; ++i)
	{
	    throughput->sendStringSeq(longStringSeq);
	}
    }
    else if(structSeqTest)
    {
	for(i = 0; i < repetitions; ++i)
	{
	    throughput->sendStructSeq(structSeq);
	}
    }

    tm = IceUtil::Time::now() - tm;
    cout << tm * 1000 / repetitions << endl;

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
	cerr << ex << endl;
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
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
