// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <list>

using namespace std;

namespace
{
IceUtil::Mutex* outputMutex = 0;

class Init
{
public:

    Init()
    {
        outputMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete outputMutex;
        outputMutex = 0;
    }
};

Init init;

}

void
mtprint(const string& data)
{
    IceUtil::Mutex::Lock sync(*outputMutex);
    cout << data << flush;
}

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

            mtprint("work item: " + item + "\n");
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
        mtprint("Pushing work items");
        mtprint(".");
        h->add("item1");
        mtprint(".");
        h->add("item2");
        mtprint(".");
        h->add("item3");
        mtprint(".");
        h->add("item4");
        mtprint(".");
        h->add("item5");
        mtprint(".");
        h->add("destroy");
        mtprint("ok\n");
        mtprint("Waiting for WorkQueue to terminate\n");
        control.join();
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream os;
        os << ex << "\n";
        mtprint(os.str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
