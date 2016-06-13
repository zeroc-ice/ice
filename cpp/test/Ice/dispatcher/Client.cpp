// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <Dispatcher.h>

DEFINE_TEST("client")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    void allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);
    return EXIT_SUCCESS;
}

#ifdef ICE_CPP11_MAPPING
class DispatcherCall : public Ice::DispatcherCall
{
public:

    DispatcherCall(function<void()> call) :
        _call(move(call))
    {
    }

    virtual void run()
    {
        _call();
    }

private:

    function<void()> _call;
};
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    int status;
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);

        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties->setProperty("Ice.TCP.SndSize", "50000");

#ifdef ICE_CPP11_MAPPING
        Ice::DispatcherPtr dispatcher = new Dispatcher();
        initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
            {
                dispatcher->dispatch(new DispatcherCall(call), conn);
            };
#else
        initData.dispatcher = new Dispatcher();
#endif
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }
    Dispatcher::terminate();
    return status;
}
