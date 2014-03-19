// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePerf/Data.h>
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
    pair<const Ice::Byte*, const Ice::Byte*> byteArr;
    byteArr.first = &byteSeq[0];
    byteArr.second = byteArr.first + byteSeq.size();

    StringSeq stringSeq(StringSeqSize, "hello");

    StringSeq longStringSeq(LongStringSeqSize, "As far as the laws of mathematics refer to reality, "
                            "they are not certain; and as far as they are certain, they do not refer to reality.");

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

    //
    // Force the connection to be established by calling a ping.
    //
    throughput->ice_ping(); 

    IceUtil::Time tm = IceUtil::Time::now();
    const int repetitions = 1000;
    if(byteTest)
    {
        payloadSize = byteSeqSize;
        if(zeroCopy)
        {
            for(i = 0; i < repetitions; ++i)
            {
                throughput->sendByteSeq(byteArr);
            }
        }
        else
        {
            for(i = 0; i < repetitions; ++i)
            {
                throughput->sendByteSeqNZ(byteSeq);
            }
        }
    }
    else if(stringSeqTest)
    {
        payloadSize = stringSeqSize;
        for(i = 0; i < repetitions; ++i)
        {
            throughput->sendStringSeq(stringSeq);
        }
    }
    else if(longStringSeqTest)
    {
        payloadSize = longSeqSize;
        for(i = 0; i < repetitions; ++i)
        {
            throughput->sendStringSeq(longStringSeq);
        }
    }
    else if(structSeqTest)
    {
        payloadSize = structSeqSize;
        for(i = 0; i < repetitions; ++i)
        {
            throughput->sendStructSeq(structSeq);
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->load("config");
        communicator = Ice::initialize(argc, argv, initData);
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
