// client.cpp,v 1.6 2003/11/02 23:27:22 dhinton Exp

#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/Sched_Params.h"
#include "ace/Stats.h"
#include "ace/OS_NS_errno.h"

#include "tao/Strategies/advanced_resource.h"
#include <iostream>

#ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
#endif

using namespace std;

ACE_RCSID(Thread_Per_Connection_Latency, client, "client.cpp,v 1.6 2003/11/02 23:27:22 dhinton Exp")

const char *ior = "file://test.ior";
int niterations = 100000;
int do_shutdown = 1;

void
usage()
{
    cout << "For latenct tests:" << endl;
    cout << "  client latency oneway|twoway" << endl;
    cout << "For throughput tests:" << endl;
    cout << "  client throughput byte|string|struct" << endl;
}

int
main (int argc, char *argv[])
{
  int priority =
    (ACE_Sched_Params::priority_min (ACE_SCHED_FIFO)
     + ACE_Sched_Params::priority_max (ACE_SCHED_FIFO)) / 2;
  // Enable FIFO scheduling, e.g., RT scheduling class on Solaris.

  if (ACE_OS::sched_params (ACE_Sched_Params (ACE_SCHED_FIFO,
                                              priority,
                                              ACE_SCOPE_PROCESS)) != 0)
    {
      if (ACE_OS::last_error () == EPERM)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "client (%P|%t): user is not superuser, "
                      "test runs in time-shared class\n"));
        }
      else
        ACE_ERROR ((LM_ERROR,
                    "client (%P|%t): sched_params failed\n"));
    }

  ACE_TRY_NEW_ENV
    {
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv, "" ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

    bool latency = false;
    bool oneway = false;

    bool throughput = false;
    bool sendbytes = false;
    bool sendstrings = false;
    bool sendstructs = false;

    if(argc != 3)
    {
        usage();
        return 1;
    }

    if(strcmp(argv[1], "latency") == 0)
    {
    	latency = true;
        if(strcmp(argv[2], "oneway") == 0)
	{
	    oneway = true;
	    cout << "oneway latency test" << endl;
	}
	else
	{
	    cout << "twoway latency test" << endl;
	}

    }
    else if(strcmp(argv[1], "throughput") == 0)
    {
        throughput = true;
	if(strcmp(argv[2], "byte") == 0)
	{
	    sendbytes = true;
	    cout << "byte sequence throughput test" << endl;
	}
	else if(strcmp(argv[2], "string") == 0)
	{
	    sendstrings = true;
	    cout << "string sequence throughput test" << endl;
	}
	else if(strcmp(argv[2], "struct") == 0)
	{
	    sendstructs = true;
	    cout << "struct sequence throughput test" << endl;
	}
	else
	{
	    usage();
	    return 1;
	}
    }
    else
    {
        usage();
	return 1;
    }


      CORBA::Object_var object =
        orb->string_to_object (ior ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      Test::Roundtrip_var roundtrip =
        Test::Roundtrip::_narrow (object.in () ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (CORBA::is_nil (roundtrip.in ()))
        {
          ACE_ERROR_RETURN ((LM_ERROR,
                             "Nil Test::Roundtrip reference <%s>\n",
                             ior),
                            1);
        }

      roundtrip->test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;

    if(latency)
    {
        cout << "pinging server " << niterations << " times (this may take a while)" << endl;

#ifdef WIN32
        struct _timeb tb;
        _ftime(&tb);
        long start = tb.time * 1000000 + tb.millitm * 1000;
#else
        struct timeval tv;
        gettimeofday(&tv, 0);
        long start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

        for (int i = 0; i != niterations; ++i)
        {
	  if(oneway)
	  {
              roundtrip->test_oneway (ACE_ENV_SINGLE_ARG_PARAMETER);
              ACE_TRY_CHECK;
	  }
	  else
	  {
              roundtrip->test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
              ACE_TRY_CHECK;
	  }
        }

	if(oneway)
	{
            roundtrip->test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
	}

#ifdef WIN32
        _ftime(&tb);
        float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
        gettimeofday(&tv, 0);
        float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif

        cout << "time for " << niterations << " pings: " << tm  << "ms" << endl;
        cout << "time per ping: " << (float) tm / niterations << "ms" << endl;
    }
    else
    {
        int i;

        Test::ByteSeq seq;
        seq.length(500000);

        Test::StringSeq stringSeq;
        stringSeq.length(50000);
        for(i = 0; i < 50000; ++i)
        {
            stringSeq[i] = "hello";
        }

        Test::StringDoubleSeq stringDoubleSeq;
        stringDoubleSeq.length(50000);
        for(i = 0; i < 50000; ++i)
        {
            stringDoubleSeq[i].str = "hello";
            stringDoubleSeq[i].d = 3.14;
        }

#ifdef WIN32
	struct _timeb tb;
	_ftime(&tb);
	long start = tb.time * 1000000 + tb.millitm * 1000;
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
	        roundtrip->sendByteSeq(seq);
	    }
	    else if(sendstrings)
	    {
	        roundtrip->sendStringSeq(stringSeq);
	    }
	    else if(sendstructs)
	    {
	        roundtrip->sendStringDoubleSeq(stringDoubleSeq);
	    }
	}
#ifdef WIN32
	_ftime(&tb);
	float tm = (tb.time * 1000000 + tb.millitm * 1000 - start) / 1000.0f;
#else
      	gettimeofday(&tv, 0);
      	float tm = (tv.tv_sec * 1000000 + tv.tv_usec - start) / 1000;
#endif

        cout << "time for " << repetitions << " sequences: " << tm << "ms" << endl;
        cout << "time per sequence: " << tm / repetitions << "ms" << endl;
      }			
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "Exception caught:");
      return 1;
    }
  ACE_ENDTRY;

  return 0;
}
