// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/UUID.h>
#include <IceE/Time.h>
#include <IceE/Thread.h>
#include <IceE/StaticMutex.h>
#include <IceE/Monitor.h>
#include <TestApplication.h>
#include <set>
#include <vector>

using namespace IceUtil;
using namespace std;

static StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;

inline void usage(const char* myName)
{
    tprintf("Usage: %s [number of UUIDs to generate] [number of threads]\n", myName);
}

class CountedBarrier : public Shared, public Monitor<Mutex>
{
public:

    CountedBarrier(int count) :
      _count(count)
    {
    }

    void
    decrement()
    {
	Lock sync(*this);
	--_count;
	if(_count == 0)
	{
	    notifyAll();
	}
    }

    void
    waitZero()
    {
	Lock sync(*this);
	while(_count != 0)
	{
	    wait();
	}
    }

    bool
    isZero() const
    {
	Lock sync(*this);
	return _count == 0;
    }

private:

    int _count;
};
typedef Handle<CountedBarrier> CountedBarrierPtr;

class InsertThread : public Thread, public Mutex
{
public:
    
    InsertThread(const CountedBarrierPtr& start, const CountedBarrierPtr& stop, int threadId, set<string>& uuidSet, long howMany, bool verbose)
	: _start(start), _stop(stop), _threadId(threadId), _uuidSet(uuidSet), _howMany(howMany), _verbose(verbose), _destroyed(false)
    {
    }

    virtual void
    run()
    {
	_start->decrement();
	_start->waitZero();

	for(long i = 0; i < _howMany && !destroyed(); i++)
	{
	    string uuid = generateUUID();
	    {
		StaticMutex::Lock lock(staticMutex);
		pair<set<string>::iterator, bool> ok = _uuidSet.insert(uuid);
		if(!ok.second)
		{
		    tprintf("******* iteration %d\n", i);
		    tprintf("******* Duplicate UUID: %s\n", (*ok.first).c_str());
		}

		test(ok.second);
	    }

#ifdef _WIN32_WCE
	    if(i > 0 && (i % 100) == 0)
	    {
		tprintf(".");
	    }
#else
	    if(_verbose && i > 0 && (i % 100000 == 0))
	    {
		tprintf("Thread %d: generated %d UUIDs.\n", _threadId, i);
	    }
#endif
	}

	_stop->decrement();
#ifdef _WIN32_WCE
	// This will cause the main thread to wake.
	tprintf(".");
#endif
    }

    void
    destroy()
    {
	Lock sync(*this);
	_destroyed = true;
    }

    bool
    destroyed() const
    {
	Lock sync(*this);
	return _destroyed;
    }
 private:

    const CountedBarrierPtr _start;
    const CountedBarrierPtr _stop;
    const int _threadId;
    set<string>& _uuidSet;
    const long _howMany;
    const bool _verbose;
    bool _destroyed;
};
typedef Handle<InsertThread> InsertThreadPtr;

class UuidTestApplication : public TestApplication
{
public:

    UuidTestApplication() :
        TestApplication("uuid test")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
#ifdef _WIN32_WCE
        long howMany = 3000;
#else
        long howMany = 300000;
#endif
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

	tprintf("Generating %d UUIDs", howMany);
        tprintf("... ");
    
        if(verbose)
        {
	    tprintf("\n");
        }
	//
	// First measure raw time to produce UUIDs.
	//
        Time startTime = Time::now();
	long i;
	for(i = 0; i < howMany; i++)
	{
    	    generateUUID();
	}
#ifdef _WIN32_WCE
	if(terminated())
	{
	    return EXIT_SUCCESS;
	}
#endif

        Time finish = Time::now();

        tprintf("ok\n");

#ifndef _WIN32_WCE
        if(verbose)
#endif
        {
            tprintf("Each UUID took an average of %.04f ms to generate and insert into a set<string>.\n",
		    ((double) ((finish - startTime).toMilliSeconds())) / howMany);
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
        
	startTime = Time::now();
	vector<InsertThreadPtr> threads;

	CountedBarrierPtr stop = new CountedBarrier(threadCount);
	CountedBarrierPtr start = new CountedBarrier(threadCount);
        for(i = 0; i < threadCount; i++)
        {
	    InsertThreadPtr t = new InsertThread(start, stop, i, uuidSet, howMany / threadCount, verbose); 
	    t->start();
	    threads.push_back(t);
        }

        vector<InsertThreadPtr>::iterator p;

#ifdef _WIN32_WCE
	while(!stop->isZero() && !terminated())
	{
	    MSG Msg;
	    if(GetMessage(&Msg, NULL, 0, 0))
	    {
		//
		// Process all pending events.
		//
		do
		{
		    TranslateMessage(&Msg);
		    DispatchMessage(&Msg);
		}
		while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE));
	    }
	}
	
	//
	// If the user terminated the app, the destroy all the
	// threads. This loop will end once all the threads have gone.
	//
	if(terminated())
	{
	    for(p = threads.begin(); p != threads.end(); ++p)
	    {
		(*p)->destroy();
	    }
	}
#endif
	stop->waitZero();
        finish = Time::now();

	for(p = threads.begin(); p != threads.end(); ++p)
        {
	    (*p)->getThreadControl().join();
        }

        tprintf("ok\n");

#ifndef _WIN32_WCE
        if(verbose)
#endif
        {
            tprintf("Each UUID took an average of %.04f ms to generate and insert into a set<string>.\n",
		    ((double) ((finish - startTime).toMilliSeconds())) / howMany);
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
