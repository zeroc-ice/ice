// Copyright (c) ZeroC, Inc.

#include "TestHelper.h"
#include "TestI.h"

#include <stdexcept>

using namespace std;
using namespace Test;

class NullLogger final : public Ice::Logger, public enable_shared_from_this<NullLogger>
{
public:
    void print(const string&) final {}
    void trace(const string&, const string&) final {}
    void warning(const string&) final {}
    void error(const string&) final {}
    string getPrefix() final { return "NullLogger"; }
    Ice::LoggerPtr cloneWithPrefix(string) final { return shared_from_this(); }
};

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
testMiddlewareFactoryException()
{
    cout << "testing middleware factory exception... " << flush;

    // Use a separate communicator with a null logger: the pipeline creation failure is logged as an error.
    Ice::InitializationData initData;
    initData.logger = make_shared<NullLogger>();
    Ice::CommunicatorHolder communicatorHolder{Ice::initialize(initData)};
    const Ice::CommunicatorPtr& communicator = communicatorHolder.communicator();

    Ice::ObjectAdapterPtr oa = communicator->createObjectAdapter("");
    oa->add(make_shared<MyObjectI>(), Ice::Identity{"test", ""});

    oa->use([](Ice::ObjectPtr) -> Ice::ObjectPtr { throw runtime_error{"middleware factory exception"}; });

    MyObjectPrx p(communicator, "test");

    try
    {
        p->ice_ping();
        test(false);
    }
    catch (const Ice::UnknownException& ex)
    {
        // The message does not reveal the middleware factory exception.
        test(string_view{ex.what()}.find("middleware factory exception") == string_view::npos);
    }

    // The failure is permanent for this object adapter.
    try
    {
        p->ice_ping();
        test(false);
    }
    catch (const Ice::UnknownException&)
    {
    }

    oa->destroy();
    cout << "ok" << endl;
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    testMiddlewareExecutionOrder(communicator);
    testMiddlewareFactoryException();
}
