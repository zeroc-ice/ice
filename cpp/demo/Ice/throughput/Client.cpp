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
	"toggle type of data to send\n"
	"  1: byte sequence (default)\n"
	"  2: string { \"hello\" } sequence\n"
	"  3: struct { \"hello\", \"3.14\" } sequence\n"
	"select test to run\n"
	"  t: send sequence as twoway\n"
	"  o: send sequence as oneway\n"
	"  r: receive sequence\n"
	"  e: echo (send and receive) sequence\n"
	"s: shutdown server\n"
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

    int byteSeqSize = 500000;
    int stringSeqSize = 100000;
    int structSeqSize = 50000;

    ByteSeq byteSeq(byteSeqSize, 0);

    StringSeq stringSeq(stringSeqSize, "hello");

    StringDoubleSeq structSeq(structSeqSize);
    for(int i = 0; i < structSeqSize; ++i)
    {
        structSeq[i].s = "hello";
	structSeq[i].d = 3.14;
    }

    menu();

    throughput->ice_ping(); // Initial ping to setup the connection.

    //
    // By default use byte sequence.
    //
    char currentType = '1';
    int seqSize = byteSeqSize;

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;

	    IceUtil::Time tm = IceUtil::Time::now();
	    const int repetitions = 1000;

	    if(c == '1' || c == '2' || c == '3')
	    {
		currentType = c;
	        switch(c)
		{
		    case '1':
		    {
		        cout << "using byte sequences" << endl;
			seqSize = byteSeqSize;
			break;
		    }

		    case '2':
		    {
		        cout << "using string sequences" << endl;
			seqSize = stringSeqSize;
			break;
		    }

		    case '3':
		    {
		        cout << "using struct sequences" << endl;
			seqSize = structSeqSize;
			break;
		    }
		}
	    }
	    else if(c == 't' || c == 'o' || c == 'r' || c == 'e')
	    {
		switch(c)
		{
		    case 't':
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

		cout << ' ' << repetitions;
	        switch(currentType)
		{
		    case '1':
		    {
		        cout << " byte";
			break;
		    }

		    case '2':
		    {
		        cout << " string";
			break;
		    }

		    case '3':
		    {
		        cout << " struct";
			break;
		    }
		}
		cout << " sequences of size " << seqSize;


		if(c == 'o')
		{
		    cout << " as oneway";
		}
		
		cout << "..." << endl;
		
		for(int i = 0; i < repetitions; ++i)
		{
		    switch(currentType)
		    {
		        case '1':
			{
		            switch(c)
		            {
			        case 't':
			        {
			            throughput->sendByteSeq(byteSeq);
			            break;
			        }
			
			        case 'o':
			        {
			            throughputOneway->sendByteSeq(byteSeq);
			            break;
			        }
			
			        case 'r':
			        {
			            throughput->recvByteSeq();
			            break;
			        }
			
			        case 'e':
			        {
			            throughput->echoByteSeq(byteSeq);
			            break;
			        }
			    }
			    break;
			}

		        case '2':
			{
		            switch(c)
		            {
			        case 't':
			        {
			            throughput->sendStringSeq(stringSeq);
			            break;
			        }
			
			        case 'o':
			        {
			            throughputOneway->sendStringSeq(stringSeq);
			            break;
			        }
			
			        case 'r':
			        {
			            throughput->recvStringSeq();
			            break;
			        }
			
			        case 'e':
			        {
			            throughput->echoStringSeq(stringSeq);
			            break;
			        }
			    }
			}

		        case '3':
			{
		            switch(c)
		            {
			        case 't':
			        {
			            throughput->sendStructSeq(structSeq);
			            break;
			        }
			
			        case 'o':
			        {
			            throughputOneway->sendStructSeq(structSeq);
			            break;
			        }
			
			        case 'r':
			        {
			            throughput->recvStructSeq();
			            break;
			        }
			
			        case 'e':
			        {
			            throughput->echoStructSeq(structSeq);
			            break;
			        }
			    }
			}
		    }
		}

		tm = IceUtil::Time::now() - tm;
		cout << "time for " << repetitions << " sequences: " << tm * 1000 << "ms" << endl;
		cout << "time per sequence: " << tm * 1000 / repetitions << "ms" << endl;
		if(currentType == '1')
		{
		    //
		    // TODO: Calculate rate for strings and structs.
		    //
		    double mbit = repetitions * seqSize * 8.0 / tm.toMicroSeconds();
		    if(c == 'e')
		    {
		        mbit *= 2;
		    }
		    cout << "throughput: " << mbit << " MBit/s" << endl;
		}
	    }
	    else if(c == 's')
	    {
		throughput->shutdown();
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
