// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Thread.h>
#include <IceE/Exception.h>
#include <IceE/Monitor.h>
#include <IceE/Mutex.h>

#include <stdio.h>
#include <list>

using namespace std;

class WorkQueue : public Ice::Thread
{
public:

    WorkQueue() { }
    ~WorkQueue() { }

    virtual void
    run()
    {
	while(1)
	{
	    string item = nextItem();
	    if(item == "destroy")
	    {
		break;
	    }

	    printf("work item: %s\n", item.c_str());
	    Ice::ThreadControl::sleep(Ice::Time::seconds(1));
	}
    }

    void
    add(const string& item)
    {
	Ice::Monitor<Ice::Mutex>::Lock lock(_monitor);
	if(_queue.empty())
	{
	    _monitor.notify();
	}
	_queue.push_back(item);
    }

private:

    string
    nextItem()
    {
	Ice::Monitor<Ice::Mutex>::Lock lock(_monitor);
	while(_queue.empty())
	{
	    _monitor.wait();
	}
	
	string item = _queue.front();
	_queue.pop_front();
	return item;
    }


    Ice::Monitor<Ice::Mutex> _monitor;
    list<string> _queue;
};

typedef Ice::Handle<WorkQueue> WorkQueuePtr;

int
main()
{
    try
    {
	WorkQueuePtr h = new WorkQueue();
	Ice::ThreadControl control = h->start();
	printf("Pushing work items");
	printf("."); fflush(stdout);
	h->add("item1");
	printf("."); fflush(stdout);
	h->add("item2");
	printf("."); fflush(stdout);
	h->add("item3");
	printf("."); fflush(stdout);
	h->add("item4");
	printf("."); fflush(stdout);
	h->add("item5");
	printf("."); fflush(stdout);
	h->add("destroy");
	printf(" ok\n");
	printf("Waiting for WorkQueue to terminate\n");
	control.join();
    }
    catch(const Ice::Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
