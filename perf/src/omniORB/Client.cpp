// eg2_clt.cc - This is the source code of example 2 used in Chapter 2
//              "The Basics" of the omniORB user guide.
//
//              This is the client. The object reference is given as a
//              stringified IOR on the command line.
//
// Usage: eg2_clt <object reference>
//

#include <ping.hh>

#include <iostream>
#include <fstream>

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

using namespace std;
using namespace Test;

int main(int argc, char** argv)
{
    try 
    {
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);	

	bool latency = false;
	bool oneway = false;
	bool twoway = false;

	bool throughput = false;
	bool sendbytes = false;
	bool sendstrings = false;
	bool sendlongstrings = false;
	bool sendstructs = false;

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
	    else if(strcmp(argv[i], "byte") == 0)
	    {
		sendbytes = true;
	    }
	    else if(strcmp(argv[i], "string") == 0)
	    {
		sendstrings = true;
	    }
	    else if(strcmp(argv[i], "longString") == 0)
	    {
		sendlongstrings = true;
	    }
	    else if(strcmp(argv[i], "struct") == 0)
	    {
		sendstructs = true;
	    }
	}

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
	    if(!sendbytes && !sendstrings && !sendlongstrings && !sendstructs)
	    {
		sendbytes = true;
	    }
	}

	FILE* in = fopen("test.ior", "r");
	if(!in)
	{
	    cerr << "Cannot open test.ior" << endl;
	}
	char ior[4096];
	fgets(ior, 4096, in);
	fclose(in);

	CORBA::Object_var obj = orb->string_to_object(ior);
	if(latency)
	{
	    Throughput_var pingref = Throughput::_narrow(obj);
	    if( CORBA::is_nil(pingref) ) 
	    {
		cerr << "Can't narrow reference to type Throughput (or it was nil)." << endl;
		return 1;
	    }

	    pingref->ping();
	    
#ifdef WIN32
	    struct _timeb tb;
	    _ftime(&tb);
	    __time64_t start = tb.time * 1000000 + tb.millitm * 1000;
#else
	    struct timeval tv;
	    gettimeofday(&tv, 0);
	    long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

	    const int repetitions = 100000;
	    for(int i = 0; i < repetitions; ++i)
	    {
		if(oneway)
		{
		    pingref->ping_oneway();
		}
		else
		{
		    pingref->ping();
		}
	    }

	    if(oneway)
	    {
		pingref->ping();
	    }
	    
#ifdef WIN32
	    _ftime(&tb);
	    float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
	    gettimeofday(&tv, 0);
	    float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif
	    cout << (float) tm / repetitions << endl;

	    pingref->shutdown();
	}
	else
	{
	    int i;
	    Throughput_var throughput = Throughput::_narrow(obj);
	    if( CORBA::is_nil(throughput) ) 
	    {
		cerr << "Can't narrow reference to type Throughput (or it was nil)." << endl;
		return 1;
	    }

	    ByteSeq seq;
	    seq.length(ByteSeqSize);

	    StringSeq stringSeq;
	    stringSeq.length(StringSeqSize);
	    for(i = 0; i < StringSeqSize; ++i)
	    {
		stringSeq[i] = CORBA::string_dup("hello");
	    }

 	    StringSeq longStringSeq;
 	    longStringSeq.length(5000);
 	    for(i = 0; i < 5000; ++i)
 	    {
 		longStringSeq[i] = CORBA::string_dup("As far as the laws of mathematics refer to reality, they are not certain; and as far as they are certain, they do not refer to reality.");
 	    }
	
	    StringDoubleSeq stringDoubleSeq;
	    stringDoubleSeq.length(StringDoubleSeqSize);
	    for(i = 0; i < StringDoubleSeqSize; ++i)
	    {
		stringDoubleSeq[i].str = CORBA::string_dup("hello");
		stringDoubleSeq[i].d = 3.14;
	    }

#ifdef WIN32
	    struct _timeb tb;
	    _ftime(&tb);
	    __time64_t start = tb.time * 1000000 + tb.millitm * 1000;
#else
	    struct timeval tv;
	    gettimeofday(&tv, 0);
	    long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
	    
	    const int repetitions = 1000;
	    for(i = 0; i < repetitions; ++i)
	    {
		if(sendbytes)
		{
		    throughput->sendByteSeq(seq);
		}
		else if(sendstrings)
		{
		    throughput->sendStringSeq(stringSeq);
		}
 		else if(sendlongstrings)
 		{
 		    throughput->sendStringSeq(longStringSeq);
 		}
		else if(sendstructs)
		{
		    throughput->sendStringDoubleSeq(stringDoubleSeq);
		}
	    }
#ifdef WIN32
	    _ftime(&tb);
	    float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
	    gettimeofday(&tv, 0);
	    float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif

	    cout << tm / repetitions << endl;	

	    throughput->shutdown();
	}

	orb->destroy();
    }
    catch(CORBA::COMM_FAILURE&) 
    {
	cerr << "Caught system exception COMM_FAILURE -- unable to contact the object." << endl;
    }
    catch(CORBA::SystemException&) 
    {
	cerr << "Caught a CORBA::SystemException." << endl;
    }
    catch(CORBA::Exception&) 
    {
	cerr << "Caught CORBA::Exception." << endl;
    }
    catch(omniORB::fatalException& fe) 
    {
	cerr << "Caught omniORB::fatalException:" << endl;
	cerr << "  file: " << fe.file() << endl;
	cerr << "  line: " << fe.line() << endl;
	cerr << "  mesg: " << fe.errmsg() << endl;
    }
    catch(...)
    {
	cerr << "Caught unknown exception." << endl;
    }
    return 0;
}
