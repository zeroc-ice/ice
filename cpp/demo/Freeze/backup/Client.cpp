// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <IntLongMap.h>

using namespace std;

static void
testFailed(const char* expr, const char* file, unsigned int line)
{
    cout << "failed!" << endl;
    cout << file << ':' << line << ": assertion `" << expr << "' failed" << endl;
    abort();
}

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

int
main(int argc, char* argv[])
{
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    initData.properties->load("config");

    Ice::CommunicatorPtr communicator = Ice::initialize(argc, argv, initData);
    if(argc > 1)
    {
        cerr << argv[0] << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Freeze::ConnectionPtr connection = Freeze::createConnection(communicator, "backup");
    IntLongMap m(connection, "IntLongMap", true);
    
    const int size = 10000;

    if(m.size() == 0)
    {   
        cout << "********* Creating new map ***********" << endl;
        Freeze::TransactionHolder txHolder(connection);

        IceUtil::Time time = IceUtil::Time::now();
        IceUtil::Int64 ms = time.toMilliSeconds();

        for(int i = 0; i < size; ++i)
        {
            m.insert(IntLongMap::value_type(i, ms));
        }
        txHolder.commit();
    }
   
    cout << "Updating map" << endl;

    for(;;)
    {
        int count = 0;

        Freeze::TransactionHolder txHolder(connection);
        IceUtil::Time time = IceUtil::Time::now();
        IceUtil::Int64 ms = time.toMilliSeconds();

        IntLongMap::iterator p = m.begin();
        IceUtil::Int64 oldMs = p->second;
        do
        {
            if(p->second != oldMs)
            {
                cerr << "old time (ms) == " << oldMs << endl;
                cerr << "current current (ms) == " << p->second << endl;
            }
            
            test(p->second == oldMs);
            p.set(ms);
            count++;
        } while(++p != m.end());

        cout << "Read " << IceUtil::Time::milliSeconds(oldMs).toDateTime() << " in all records;" 
             << " updating with " << time.toDateTime() << " ... " << flush;
          
        txHolder.commit();
        cout << "done" << endl;
        test(count == size);
    }

    connection->close();

    communicator->destroy();

    return EXIT_SUCCESS;
}
