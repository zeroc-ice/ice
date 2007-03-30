// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Throughput.h>

using namespace std;
using namespace Demo;

void
menu()
{
    printf("usage:\n");
    printf("\n");
    printf("toggle type of data to send:\n");
    printf("1: sequence of bytes (default)\n");
    printf("2: sequence of strings (\"hello\")\n");
    printf("3: sequence of structs with a string (\"hello\") and a double\n");
    printf("4: sequence of structs with two ints and a double\n");
    printf("\n");
    printf("select test to run:\n");
    printf("t: Send sequence as twoway\n");
    printf("o: Send sequence as oneway\n");
    printf("r: Receive sequence\n");
    printf("e: Echo (send and receive) sequence\n");
    printf("\n");
    printf("other commands:\n");
    printf("s: shutdown server\n");
    printf("x: exit\n");
    printf("?: help\n");
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    //
    // Check if we need to run with small sequences
    //
    int reduce = 1;
    int i;
    for(i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], "--small") == 0)
        {
            reduce = 100;
        }
    }

    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Throughput.Proxy";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	fprintf(stderr, "%s: property `%s' not set\n", argv[0], proxyProperty);
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    ThroughputPrx throughput = ThroughputPrx::checkedCast(base);
    if(!throughput)
    {
	fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }
    ThroughputPrx throughputOneway = ThroughputPrx::uncheckedCast(throughput->ice_oneway());

    ByteSeq byteSeq(ByteSeqSize / reduce, 0);
    pair<const Ice::Byte*, const Ice::Byte*> byteArr;
    byteArr.first = &byteSeq[0];
    byteArr.second = byteArr.first + byteSeq.size();

    StringSeq stringSeq(StringSeqSize / reduce, "hello");

    StringDoubleSeq structSeq(StringDoubleSeqSize / reduce);
    for(i = 0; i < StringDoubleSeqSize / reduce; ++i)
    {
        structSeq[i].s = "hello";
	structSeq[i].d = 3.14;
    }

    FixedSeq fixedSeq(FixedSeqSize / reduce);
    for(i = 0; i < FixedSeqSize / reduce; ++i)
    {
        fixedSeq[i].i = 0;
        fixedSeq[i].j = 0;
        fixedSeq[i].d = 0;
    }

    menu();

    throughput->ice_ping(); // Initial ping to setup the connection.

    //
    // By default use byte sequence.
    //
    char currentType = '1';
    int seqSize = ByteSeqSize / reduce;

    char c = EOF;
    do
    {
	try
	{
	    printf("==> ");
	    do
	    {
	        c = getchar();
	    }
	    while(c != EOF && c == '\n');

	    IceUtil::Time tm = IceUtil::Time::now();
	    const int repetitions = 1000;

	    if(c == '1' || c == '2' || c == '3' || c == '4')
	    {
		currentType = c;
	        switch(c)
		{
		    case '1':
		    {
		        printf("using byte sequences\n");
			seqSize = ByteSeqSize / reduce;
			break;
		    }

		    case '2':
		    {
		        printf("using string sequences\n");
			seqSize = StringSeqSize / reduce;
			break;
		    }

		    case '3':
		    {
		        printf("using variable-length struct sequences\n");
			seqSize = StringDoubleSeqSize / reduce;
			break;
		    }

		    case '4':
		    {
		        printf("using fixed-length struct sequences\n");
			seqSize = FixedSeqSize / reduce;
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
			printf("sending");
			break;
		    }

		    case 'r':
		    {
			printf("receiving");
			break;
		    }

		    case 'e':
		    {
			printf("sending and receiving");
			break;
		    }
		}

		printf(" %d", repetitions);
	        switch(currentType)
		{
		    case '1':
		    {
		        printf(" byte");
			break;
		    }

		    case '2':
		    {
		        printf(" string");
			break;
		    }

		    case '3':
		    {
		        printf(" variable-length struct");
			break;
		    }

		    case '4':
		    {
		        printf(" fixed-length struct");
			break;
		    }
		}
		printf(" sequences of size %d", seqSize);


		if(c == 'o')
		{
		    printf(" as oneway");
		}
		
		printf("...\n");
		
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
			            throughput->sendByteSeq(byteArr);
			            break;
			        }
			
			        case 'o':
			        {
			            throughputOneway->sendByteSeq(byteArr);
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
			    break;
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
			    break;
			}

		        case '4':
			{
		            switch(c)
		            {
			        case 't':
			        {
			            throughput->sendFixedSeq(fixedSeq);
			            break;
			        }
			
			        case 'o':
			        {
			            throughputOneway->sendFixedSeq(fixedSeq);
			            break;
			        }
			
			        case 'r':
			        {
			            throughput->recvFixedSeq();
			            break;
			        }
			
			        case 'e':
			        {
			            throughput->echoFixedSeq(fixedSeq);
			            break;
			        }
			    }
			    break;
			}
		    }
		}

		tm = IceUtil::Time::now() - tm;
		printf("time for %d sequences: %lfms\n", repetitions, tm.toMilliSecondsDouble());
		printf("time per sequence: %lfms\n", tm.toMilliSecondsDouble() / repetitions);
		int wireSize = 0;
		switch(currentType)
		{
		    case '1':
		    {
		    	wireSize = 1;
		    	break;
		    }
		    case '2':
		    {
		        wireSize = static_cast<int>(stringSeq[0].size());
		    	break;
		    }
		    case '3':
		    {
		    	wireSize = static_cast<int>(structSeq[0].s.size());
			wireSize += 8; // Size of double on the wire.
		    	break;
		    }
		    case '4':
		    {
			wireSize = 16; // Size of two ints and a double on the wire.
		    	break;
		    }
		}
		double mbit = repetitions * seqSize * wireSize * 8.0 / tm.toMicroSeconds();
		if(c == 'e')
		{
		    mbit *= 2;
		}
		printf("throughput: %.2lfMbps\n", mbit);
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
		printf("unknown command `%c'\n", c);
		menu();
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	}
    }
    while(c != EOF && c != 'x');

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
	fprintf(stderr, "%s\n", ex.toString().c_str());
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
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
