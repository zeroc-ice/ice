// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Item.h>

using namespace std;
using namespace Warehouse;

const int readCount = 15000;
const int writeCount = 1500;

const int objectCount = 10000;

class WarehouseClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    WarehouseClient app;
    return app.main(argc, argv, "config.client");
}

class StopWatch
{
public:

    void
    start()
    {
        _stopped = false;
        _start = IceUtil::Time::now();
    }

    IceUtil::Time
    stop()
    {
        if(!_stopped)
        {
            _stopped = true;
            _stop = IceUtil::Time::now();
        }

        return _stop - _start;
    }

private:

    bool _stopped;
    IceUtil::Time _start;
    IceUtil::Time _stop;
};

class ReaderThread : public IceUtil::Thread
{
public:

    ReaderThread(ItemPrx anItem)
        : _anItem(anItem),
          _requestsPerSecond(-1)
    {
    }

    virtual void run()
    {
        //
        // Measures how long it takes to read 'readCount' items at random
        //
        StopWatch stopWatch;
        stopWatch.start();
        
        try
        {
            for(int i = 0; i < readCount; ++i)
            {
                int id = IceUtil::random(objectCount);
                ostringstream os;
                os << "P/N " << id;
                string name = os.str();
                
                Ice::Identity identity;
                identity.name = name;
                ItemPrx item = ItemPrx::uncheckedCast(_anItem->ice_identity(identity));
                item->getDescription();
            }
            _requestsPerSecond = static_cast<int>(readCount / stopWatch.stop().toSecondsDouble());  
        }
        catch(const IceUtil::Exception& e)
        {
            cerr << "Unexpected exception in ReaderThread: " << e << endl;
        }   
    }

    int getRequestsPerSecond() const
    {
        return _requestsPerSecond;
    }

private:
    
    ItemPrx _anItem;
    int _requestsPerSecond;
};

typedef IceUtil::Handle<ReaderThread> ReaderThreadPtr;


class WriterThread : public IceUtil::Thread
{
public:

    WriterThread(ItemPrx anItem)
        : _anItem(anItem),
          _requestsPerSecond(-1)
    {
    }

    virtual void run()
    {
        //
        // Measure how long it takes to write 'writeCount' items at random
        //
        StopWatch stopWatch;
        stopWatch.start();
        
        try
        {
            for(int i = 0; i < writeCount; ++i)
            {
                int id = IceUtil::random(objectCount);
                    
                ostringstream os;
                os << "P/N " << id;
                string name = os.str();

                Ice::Identity identity;
                identity.name = name;
                ItemPrx item = ItemPrx::uncheckedCast(_anItem->ice_identity(identity));
                
                item->adjustStock(1);
            }
            _requestsPerSecond = static_cast<int>(writeCount / stopWatch.stop().toSecondsDouble());  
        }
        catch(const IceUtil::Exception& e)
        {
            cerr << "Unexpected exception in WriterThread: " << e << endl;
        }   
    }
    
    int getRequestsPerSecond() const
    {
        return _requestsPerSecond;
    }

private:
    
    ItemPrx _anItem;
    int _requestsPerSecond;
};

typedef IceUtil::Handle<WriterThread> WriterThreadPtr;


int
WarehouseClient::run(int argc, char* argv[])
{
    //
    // Retrieve a proxy to one item (any item will do)
    //
    ItemPrx anItem = ItemPrx::checkedCast(communicator()->propertyToProxy("Item.Proxy"));

    //
    // Start 1 writer and 5 readers
    //
    WriterThreadPtr wt = new WriterThread(anItem);
    wt->start();

    const int readerCount = 5;


    ReaderThreadPtr rt[readerCount];
    int i;
    for(i = 0; i < readerCount; ++i)
    {
        rt[i] = new ReaderThread(anItem);
        rt[i]->start();
    }
   

    wt->getThreadControl().join();

    for(i = 0; i < readerCount; ++i)
    {
        rt[i]->getThreadControl().join(); 
    }
   
    //
    // Display results:
    //
    
    cout.precision(3);
    int rpt = wt->getRequestsPerSecond();
    if(rpt > 0)
    {
        cout << "Writer: " << rpt << " requests per second (" << 1000.0 / rpt << " ms per request)" << endl;
    }
   
    for(i = 0; i < readerCount; ++i)
    {
        rpt = rt[i]->getRequestsPerSecond();
        if(rpt > 0)
        {
            cout << "Reader " << i << ": " << rt[i]->getRequestsPerSecond() 
                 << " requests per second (" << 1000.0 / rpt << " ms per request)" << endl;
        }
    }
    
    return EXIT_SUCCESS;
}

