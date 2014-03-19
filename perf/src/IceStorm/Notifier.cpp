// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Application.h>

#include <Sync.h>

using namespace std;
using namespace Perf;

class Notifier : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Notifier app;
    return app.main(argc, argv, "config");
}

int
Notifier::run(int argc, char* argv[])
{
    vector<string> proxies;
    for(int i = 1; i < argc; i++)
    {
        proxies.push_back(argv[i]);
    }

    vector<Perf::SyncPrx> syncs;
    {
        for(vector<string>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
        {
            SyncPrx s = Perf::SyncPrx::uncheckedCast(communicator()->stringToProxy(*p)->ice_oneway());
            s->ice_ping();
            syncs.push_back(s);
        }
    }

    {
        for(vector<Perf::SyncPrx>::const_iterator p = syncs.begin(); p != syncs.end(); ++p)
        {
            (*p)->notify();
        }
    }

    return EXIT_SUCCESS;
}
