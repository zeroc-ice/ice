// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

#include <list>

using namespace std;

class WorkQueue : public IceUtil::Thread
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

	    cout << "work item: " << item << endl;
	    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));
	}
    }

    void
    add(const string& item)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
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
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
	while(_queue.empty())
	{
	    _monitor.wait();
	}
	
	string item = _queue.front();
	_queue.pop_front();
	return item;
    }


    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    list<string> _queue;
};

typedef IceUtil::Handle<WorkQueue> WorkQueuePtr;

int
main()
{
    try
    {
	WorkQueuePtr h = new WorkQueue();
	IceUtil::ThreadControl control = h->start();
	cout << "Pushing work items";
	cout << '.' << flush;
	h->add("item1");
	cout << '.' << flush;
	h->add("item2");
	cout << '.' << flush;
	h->add("item3");
	cout << '.' << flush;
	h->add("item4");
	cout << '.' << flush;
	h->add("item5");
	cout << '.' << flush;
	h->add("destroy");
	cout << " ok" << endl;
	cout << "Waiting for WorkQueue to terminate" << endl;
	control.join();
    }
    catch(const IceUtil::Exception& ex)
    {
	cerr << ex << endl;
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
