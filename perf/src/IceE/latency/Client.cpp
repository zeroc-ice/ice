// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Latency.h>
#include <IcePerf/Data.h>

#include <iostream>

using namespace std;
using namespace Demo;

class TestAdapter
{
public:
    TestAdapter(const LatencyPrx prx) :
        _prx(prx)
    {
    }

    virtual ~TestAdapter() {}

    virtual void doIt() = 0;

protected:
    LatencyPrx _prx;
};

class NoPayload : public TestAdapter
{ 
public:
    NoPayload(const LatencyPrx prx):
        TestAdapter(prx)
    {
    }

    virtual void doIt()
    {
        _prx->ping();
    }
};

class WithPayload : public TestAdapter
{
public:
    WithPayload(const LatencyPrx prx, const long payLoadSize):
        TestAdapter(prx), _payload(payLoadSize)
    {
        for(long i = 0; i < payLoadSize; ++i)
        {
            _payload[i] = '0' + (char)(i % 10);
        }
    }

    virtual void doIt()
    {
        _prx->withData(_payload);
    }

private:
    ByteSeq _payload;
};

TestAdapter* 
createAdapter(const LatencyPrx prx, const long payLoadSize)
{
    if(payLoadSize > 0)
    {
        return new WithPayload(prx, payLoadSize);
    }
    else
    {
        return new NoPayload(prx);
    }
    return 0;
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    bool oneway = false;
    bool batch = false;
    bool twoway = false;
    int i;
    long payLoadSize = 0;
    for(i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "oneway") == 0)
        {
            oneway = true;
        }
        else if(strcmp(argv[i], "batch") == 0)
        {
            batch = true;
        }
        else if(strcmp(argv[i], "twoway") == 0)
        {
            twoway = true;
        }
        else if(strncmp(argv[i], "--payload=", strlen("--payload=")) == 0)
        {
            payLoadSize = strtol(argv[i] + strlen("--payload="), 0, 10);
            if(errno == ERANGE)
            {
                cerr << argv[0] << ": payload argument range error: " << argv[i] << endl;
                return EXIT_FAILURE;
            }
        }
    }
    if(!oneway && !twoway && !batch)
    {
        twoway = true;
    }

    int repetitions = 0;
    if(twoway)
    {
        repetitions = 100000;
    }
    else if(oneway)
    {
        repetitions = 500000;
    }
    else if(batch)
    {
        repetitions = 1000000;
    }

    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Latency.Latency";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
        cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    LatencyPrx latency = LatencyPrx::uncheckedCast(base);
    if(!latency)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    LatencyPrx onewayprx = LatencyPrx::uncheckedCast(base->ice_oneway());
    LatencyPrx batchprx = LatencyPrx::uncheckedCast(base->ice_batchOneway());

    // Initial ping to setup the connection.
    latency->ping();

    TestAdapter* testAdapter = 0; 
    try
    {
        if(twoway)
        {
            testAdapter = createAdapter(latency, payLoadSize); 
        }
        else if(oneway)
        {
            testAdapter = createAdapter(onewayprx, payLoadSize); 
        }
        else if(batch)
        {
            testAdapter = createAdapter(batchprx, payLoadSize); 
        }

        //
        // MAIN TEST LOOP.
        //
        IceUtil::Time tm = IceUtil::Time::now();
        for(i = 0; i < repetitions; ++i)
        {
            if(batch)
            {
                if(i != 0 && i % 100 == 0)
                {
                    batchprx->ice_connection()->flushBatchRequests();
                }
            }
            testAdapter->doIt();
        }
        // 
        // END OF MAIN TEST LOOP
        //

        //
        // Tidy up laggart requests.
        //
        if(oneway || batch)
        {
            if(batch)
            {
                batchprx->ice_connection()->flushBatchRequests();
            }
            latency->ping();
        }

        IcePerf::TestPrinter formatter;
        formatter.fmt(cout, "IceE", "latency", IceUtil::Time::now() - tm, repetitions, payLoadSize * sizeof(Ice::Byte), argc, argv);
        latency->shutdown();
        delete testAdapter;
    }
    catch(...)
    {
        delete testAdapter;
        throw;
    }


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
