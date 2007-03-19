// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Latency.h>

using namespace std;
using namespace Demo;

class LatencyClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    LatencyClient app;
    return app.main(argc, argv, "config.client");
}

int
LatencyClient::run(int argc, char* argv[])
{
    Ice::ObjectPrx base = communicator()->propertyToProxy("Latency.Ping");
    PingPrx ping = PingPrx::checkedCast(base);
    if(!ping)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    // Initial ping to setup the connection.
    ping->ice_ping();

    IceUtil::Time tm = IceUtil::Time::now();

    const int repetitions = 100000;
    cout << "pinging server " << repetitions << " times (this may take a while)" << endl;
    for(int i = 0; i < repetitions; ++i)
    {
        ping->ice_ping();
    }

    tm = IceUtil::Time::now() - tm;

    cout << "time for " << repetitions << " pings: " << tm * 1000 << "ms" << endl;
    cout << "time per ping: " << tm * 1000 / repetitions << "ms" << endl;

    return EXIT_SUCCESS;
}
