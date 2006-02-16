// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Latency.h>

using namespace std;
using namespace Demo;

class AMI_Latency_pingI : public Demo::AMI_Latency_ping, IceUtil::Monitor<IceUtil::Mutex>
{
public:
    AMI_Latency_pingI() :
        _finished(false)
    {
    }

    void waitFinished()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	while(!_finished)
	{
	    wait();
	}
	_finished = false;
    }

private:
    virtual void ice_response()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	assert(!_finished);
	_finished = true;
	notify();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        assert(false);
    }

    bool _finished;
};

typedef IceUtil::Handle<AMI_Latency_pingI> AMI_Latency_pingIPtr;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    bool oneway = false;
    bool batch = false;
    bool twoway = false;
    bool ami = false;
    int i;
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

    AMI_Latency_pingIPtr cb = new AMI_Latency_pingI();

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

	if(twoway)
	{
	    if(ami)
	    {
	        latency->ping_async(cb);
		cb->waitFinished();
	    }
	    else
	    {
	        latency->ping();
	    }
	}
	else if(oneway)
	{
	    onewayprx->ping();
	}
	else if(batch)
	{
	    batchprx->ping();
	}
    }

    if(oneway || batch)
    {
        if(batch)
	{
            batchprx->ice_connection()->flushBatchRequests();
        }
        latency->ping();
    }

    tm = IceUtil::Time::now() - tm;

    latency->shutdown();

    cout << tm.toMilliSecondsDouble() / repetitions << endl;
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
