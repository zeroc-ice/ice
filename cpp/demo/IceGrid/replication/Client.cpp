// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloClient : public Ice::Application
{
public:

    HelloClient();
    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    HelloClient app;
    return app.main(argc, argv, "config.client");
}

HelloClient::HelloClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
HelloClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    //
    // Get the hello proxy. We configure the proxy to not cache the
    // server connection with the proxy and to disable the locator
    // cache. With this configuration, the IceGrid locator will be
    // queried for each invocation on the proxy and the invocation
    // will be sent over the server connection matching the returned
    // endpoints.
    //
    Ice::ObjectPrx obj = communicator()->stringToProxy("hello");
    obj = obj->ice_connectionCached(false);
    obj = obj->ice_locatorCacheTimeout(0);

    HelloPrx hello = HelloPrx::checkedCast(obj);
    if(!hello)
    {
        cerr << argv[0] << ": couldn't find a `::Demo::Hello' object." << endl;
        return EXIT_FAILURE;
    }

    string s;
    do
    {
        cout << "enter the number of iterations: ";
        cin >> s;
        int count = atoi(s.c_str());
        cout << "enter the delay between each greetings (in ms): ";
        cin >> s;
        int delay = atoi(s.c_str());
        if(delay < 0)
        {
            delay = 500; // 500 milli-seconds
        }
        
        for(int i = 0; i < count; i++)
        {
            cout << hello->getGreeting() << endl;
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(delay));
        }
    }
    while(cin.good() && s != "x");

    return EXIT_SUCCESS;
}
