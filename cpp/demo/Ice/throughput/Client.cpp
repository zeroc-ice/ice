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

void
menu()
{
    cout <<
	"usage:\n"
	"s: send byte sequence\n"
	"o: send byte sequence as oneway\n"
	"r: receive byte sequence\n"
	"e: echo (send and receive) byte sequence\n"
	"x: exit\n"
	"?: help\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
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

    ByteSeq seq(seqSize, 0);

    menu();

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;

	    throughput->ice_ping(); // Initial ping to setup the connection.

	    IceUtil::Time tm = IceUtil::Time::now();
	    const int repetitions = 100;

	    if(c == 's' || c == 'o' || c == 'r' || c == 'e')
	    {
		switch(c)
		{
		    case 's':
		    case 'o':
		    {
			cout << "sending";
			break;
		    }

		    case 'r':
		    {
			cout << "receiving";
			break;
		    }

		    case 'e':
		    {
			cout << "sending and receiving";
			break;
		    }
		}

		cout << ' ' << repetitions << " sequences of size " << seqSize;

		if(c == 'o')
		{
		    cout << " as oneway";
		}
		
		cout << "..." << endl;
		
		for(int i = 0; i < repetitions; ++i)
		{
		    switch(c)
		    {
			case 's':
			{
			    throughput->sendByteSeq(seq);
			    break;
			}
			
			case 'o':
			{
			    throughputOneway->sendByteSeq(seq);
			    break;
			}
			
			case 'r':
			{
			    throughput->recvByteSeq();
			    break;
			}
			
			case 'e':
			{
			    throughput->echoByteSeq(seq);
			    break;
			}
		    }
		}

		tm = IceUtil::Time::now() - tm;
		cout << "time for " << repetitions << " sequences: " << tm * 1000 << "ms" << endl;
		cout << "time per sequence: " << tm * 1000 / repetitions << "ms" << endl;
		double mbit = repetitions * seqSize * 8.0 / tm.toMicroSeconds();
		if(c == 'e')
		{
		    mbit *= 2;
		}
		cout << "throughput: " << mbit << " MBit/s" << endl;
	    }
	    else if(c == 'x')
	    {
		// Nothing to do
	    }
	    else if(c == '?')
	    {
		menu();
	    }
	    else
	    {
		cout << "unknown command `" << c << "'" << endl;
		menu();
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	}
    }
    while(cin.good() && c != 'x');

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
