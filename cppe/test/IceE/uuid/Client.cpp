// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/UUID.h>
#include <IceE/Time.h>
#include <IceE/Thread.h>
#include <IceE/StaticMutex.h>
#include <TestApplication.h>
#include <set>
#include <vector>

using namespace IceE;
using namespace std;


static StaticMutex staticMutex = ICEE_STATIC_MUTEX_INITIALIZER;

inline void usage(const char* myName)
{
    tprintf("Usage: %s [number of UUIDs to generate] [number of threads]\n", myName);
}

class InsertThread : public Thread
{
public:

    
    InsertThread(int threadId, set<string>& uuidSet, long howMany, bool verbose)
	: _threadId(threadId), _uuidSet(uuidSet), _howMany(howMany), _verbose(verbose)
    {
    }


    virtual void run()
    {
	for(long i = 0; i < _howMany; i++)
	{
	    string uuid = generateUUID();

	    StaticMutex::Lock lock(staticMutex);
	    pair<set<string>::iterator, bool> ok = _uuidSet.insert(uuid);
	    if(!ok.second)
	    {
	        tprintf("******* iteration %d\n", i);
		tprintf("******* Duplicate UUID: %s\n", (*ok.first).c_str());
	    }

	    test(ok.second);

	    if(_verbose && i > 0 && (i % 100000 == 0))
	    {
		tprintf("Thread %d: generated %d UUIDs.\n", _threadId, i);
	    }
	}
    }


private:
    int _threadId;
    set<string>& _uuidSet;
    long _howMany;
    bool _verbose;
};

class UuidTestApplication : public TestApplication
{
public:

    UuidTestApplication() :
        TestApplication("uuid test")
    {
    }

    virtual int run(int argc, char* argv[])
    {
        long howMany = 300000;
        int threadCount = 3;
        bool verbose = false;

        if(argc > 3)
        {
	    usage(argv[0]);
	    return EXIT_FAILURE;
        }
        else if(argc == 3)
        {
	    howMany = atol(argv[1]);
	    if (howMany == 0)
	    {
	        usage(argv[0]);
	        return EXIT_FAILURE;
	    }
	    threadCount = atoi(argv[2]);
	    if(threadCount <= 0)
	    {
	        usage(argv[0]);
	        return EXIT_FAILURE;
	    }
	    verbose = true;
        }
        else if(argc == 2)
        {
	    howMany = atol(argv[1]);
	    if (howMany == 0)
	    {
	        usage(argv[0]);
	        return EXIT_FAILURE;
	    }
        }
    
        tprintf("Generating %d UUIDs using %d thread", howMany, threadCount);
        if(threadCount > 1)
        {
	    tprintf("s");
        }
        tprintf("... ");
    
        if(verbose)
        {
	    tprintf("\n");
        }

        set<string> uuidSet;
    
        vector<ThreadControl> threads;

        Time start = Time::now();
        for(int i = 0; i < threadCount; i++)
        {
	    ThreadPtr t = new InsertThread(i, uuidSet, howMany / threadCount, verbose); 
	    threads.push_back(t->start());
        }
        for(vector<ThreadControl>::iterator p = threads.begin(); p != threads.end(); ++p)
        {
	    p->join();
        }
        Time finish = Time::now();

        tprintf("ok\n");;

        if(verbose)
        {
            tprintf("Each UUID took an average of %f micro seconds to generate and insert into a set<string>.\n",
	           (double) ((finish - start).toMicroSeconds()) / howMany);
        }

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UuidTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    UuidTestApplication app;
    return app.main(argc, argv);
}

#endif
