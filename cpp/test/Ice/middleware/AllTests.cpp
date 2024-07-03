// Copyright (c) ZeroC, Inc.

#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

class Middleware final : public Ice::Object
{
public:
    Middleware(Ice::ObjectPtr next, string name, list<string>& inLog, list<string>& outLog)
        : _next(std::move(next)),
          _name(std::move(name)),
          _inLog(inLog),
          _outLog(outLog)
    {
    }

    void dispatch(Ice::IncomingRequest& request, std::function<void(Ice::OutgoingResponse)> sendResponse) final
    {
        _inLog.push_back(_name);

        _next->dispatch(
            request,
            [this, sendResponse = std::move(sendResponse)](Ice::OutgoingResponse response)
            {
                _outLog.push_back(_name);
                sendResponse(std::move(response));
            });
    }

private:
    Ice::ObjectPtr _next;
    string _name;
    list<string>& _inLog;
    list<string>& _outLog;
};

void
testMiddlewareExecutionOrder(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing middleware execution order... " << flush;

    // Arrange
    list<string> inLog;
    list<string> outLog;

    Ice::ObjectAdapterPtr oa = communicator->createObjectAdapter("");
    oa->add(make_shared<MyObjectI>(), Ice::Identity{"test", ""});

    oa->use([&](Ice::ObjectPtr next) { return make_shared<Middleware>(std::move(next), "A", inLog, outLog); })
        ->use([&](Ice::ObjectPtr next) { return make_shared<Middleware>(std::move(next), "B", inLog, outLog); })
        ->use([&](Ice::ObjectPtr next) { return make_shared<Middleware>(std::move(next), "C", inLog, outLog); });

    MyObjectPrx p(communicator, "test");

    // Act
    p->ice_ping();

    // Assert
    bool inOk = inLog == list<string>{"A", "B", "C"};
    test(inOk);
    bool outOk = outLog == list<string>{"C", "B", "A"};
    test(outOk);

    oa->destroy();
    cout << "ok" << endl;
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    testMiddlewareExecutionOrder(communicator);
}
