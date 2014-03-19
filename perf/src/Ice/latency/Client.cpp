// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Latency.h>
#include <IcePerf/Data.h>

using namespace std;
using namespace Demo;

class AMI_Latency_pingI : public Demo::AMI_Latency_ping, IceUtil::Monitor<IceUtil::Mutex>
{
public:
    AMI_Latency_pingI() :
        _finished(false)
    {
    }

    void 
    waitFinished()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_finished)
        {
            wait();
        }
        _finished = false;
    }

private:
    virtual void 
    ice_response()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_finished);
        _finished = true;
        notify();
    }

    virtual void 
    ice_exception(const ::Ice::Exception&)
    {
        assert(false);
    }

    bool _finished;
};

typedef IceUtil::Handle<AMI_Latency_pingI> AMI_Latency_pingIPtr;

class AMI_Latency_withDataI : public Demo::AMI_Latency_withData, IceUtil::Monitor<IceUtil::Mutex>
{
public:
    AMI_Latency_withDataI() :
        _finished(false)
    {
    }

    void 
    waitFinished()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_finished)
        {
            wait();
        }
        _finished = false;
    }
    
private:
    virtual void
    ice_response()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_finished);
        _finished = true;
        notify();
    }

    virtual void
    ice_exception(const ::Ice::Exception&)
    {
        assert(false);
    }

    bool _finished;
};

typedef IceUtil::Handle<AMI_Latency_withDataI> AMI_Latency_withDataIPtr;

class TestAdapter
{
public:
    TestAdapter(const LatencyPrx prx) :
        _prx(prx)
    {
    }

    virtual ~TestAdapter() {}

    virtual void doIt() = 0;

    virtual void doItAsync() = 0;

    virtual void 
    flush()
    {
        _prx->ice_getConnection()->flushBatchRequests();
    }

protected:
    LatencyPrx _prx;
};

class NoPayload : public TestAdapter
{ 
public:
    NoPayload(const LatencyPrx prx):
        TestAdapter(prx),
        _cb(new AMI_Latency_pingI)
    {
    }

    virtual void 
    doIt()
    {
        _prx->ping();
    }

    virtual void 
    doItAsync()
    {
        _prx->ping_async(_cb);
        _cb->waitFinished();
    }

private:
    AMI_Latency_pingIPtr _cb;
};

class WithPayload : public TestAdapter
{
public:
    WithPayload(const LatencyPrx prx, const long payLoadSize):
        TestAdapter(prx), _payload(payLoadSize), _cb(new AMI_Latency_withDataI) 
    {
        for(long i = 0; i < payLoadSize; ++i)
        {
            _payload[i] = '0' + (char)(i % 10);
        }
    }

    virtual void 
    doIt()
    {
        _prx->withData(_payload);
    }

    virtual void 
    doItAsync()
    {
        _prx->withData_async(_cb, _payload);
        _cb->waitFinished();
    }

private:
    ByteSeq _payload;
    AMI_Latency_withDataIPtr _cb;
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
    bool ami = false;
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
        else if(strcmp(argv[i], "ami") == 0)
        {
            ami = true;
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

    // Initial ping to setup the connection.
    latency->ping();

    //
    // Unless there is a payload defined there is no payload per-se. This
    // will result in data throughput of 0. This is okay since we don't
    // care about throughput for requests with no data anyway.
    //

    TestAdapter* adapter = 0;
    try
    {
        if(batch)
        {
            adapter = createAdapter(LatencyPrx::uncheckedCast(base->ice_batchOneway()), payLoadSize);
        }
        else if(oneway)
        {
            adapter = createAdapter(LatencyPrx::uncheckedCast(base->ice_oneway()), payLoadSize);
        }
        else
        {
            adapter = createAdapter(latency, payLoadSize);
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
                    adapter->flush();
                }
            }
            if(ami)
            {
                adapter->doItAsync();
            }
            else
            {
                adapter->doIt();
            }
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
                adapter->flush();
            }
            latency->ping();
        }

        IcePerf::TestPrinter formatter;
        formatter.fmt(cout, "Ice", "latency", IceUtil::Time::now() - tm, repetitions, payLoadSize * sizeof(Ice::Byte), argc, argv);
        latency->shutdown();
        delete adapter;
    }
    catch(...)
    {
        delete adapter;
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
