// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <IceUtil/Time.h>
#include <IceUtil/Thread.h>
#include <IceUtil/StaticMutex.h>
#include <TestCommon.h>
#include <set>
#include <vector>

using namespace IceUtil;
using namespace std;


static StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;

inline void usage(const char* myName)
{
    cerr << "Usage: " << myName << " [number of UUIDs to generate] [number of threads]" << endl;
}

class InsertThread : public Thread
{
public:

    
    InsertThread(int threadId, set<string>& uuidSet, long howMany, bool verbose)
	: _threadId(threadId), _uuidSet(uuidSet), _howMany(howMany), _verbose(verbose)
    {
    }


    void run()
    {
	for(long i = 0; i < _howMany; i++)
	{
	    string uuid = generateUUID();

	    StaticMutex::Lock lock(staticMutex);
	    pair<set<string>::iterator, bool> ok = _uuidSet.insert(uuid);
	    if(!ok.second)
	    {
		cerr << "******* iteration " << i << endl;
		cerr << "******* Duplicate UUID: " << *ok.first << endl;
	    }

	    test(ok.second);

	    if(_verbose && i > 0 && (i % 100000 == 0))
	    {
		cout << "Thread " << _threadId << ": generated " << i << " UUIDs." << endl; 
	    }
	}
    }


private:
    int _threadId;
    set<string>& _uuidSet;
    long _howMany;
    bool _verbose;
};

int main(int argc, char* argv[])
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
    
    cout << "Generating " << howMany << " UUIDs using " << threadCount << " thread";
    if(threadCount > 1)
    {
	cout << "s";
    }
    cout << "... ";
    
    if(verbose)
    {
	cout << endl;
    }
    else
    {
	cout << flush;
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

    cout << "ok" << endl;

    if(verbose)
    {
        cout << "Each UUID took an average of "  
	     << (double) ((finish - start).toMicroSeconds()) / howMany 
	     << " micro seconds to generate and insert into a set<string>." 
	     << endl;
    }

    return EXIT_SUCCESS;
}
