// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;

TestIntfI::TestIntfI() = default;

void
TestIntfI::op(const Ice::Current&)
{
}

int
TestIntfI::opWithResult(const Ice::Current&)
{
    return 15;
}

void
TestIntfI::opWithUE(const Ice::Current&)
{
    throw Test::TestIntfException();
}

int
TestIntfI::opWithResultAndUE(const Ice::Current&)
{
    throw Test::TestIntfException();
}

void
TestIntfI::opWithPayload(Ice::ByteSeq, const Ice::Current&)
{
}

void
TestIntfI::opBatch(const Ice::Current&)
{
    lock_guard lock(_mutex);
    ++_batchCount;
    _condition.notify_one();
}

int32_t
TestIntfI::opBatchCount(const Ice::Current&)
{
    lock_guard lock(_mutex);
    return _batchCount;
}

void
TestIntfI::opWithArgs(
    int32_t& one,
    int32_t& two,
    int32_t& three,
    int32_t& four,
    int32_t& five,
    int32_t& six,
    int32_t& seven,
    int32_t& eight,
    int32_t& nine,
    int32_t& ten,
    int32_t& eleven,
    const Ice::Current&)
{
    one = 1;
    two = 2;
    three = 3;
    four = 4;
    five = 5;
    six = 6;
    seven = 7;
    eight = 8;
    nine = 9;
    ten = 10;
    eleven = 11;
}

bool
TestIntfI::waitForBatch(int32_t count, const Ice::Current&)
{
    unique_lock lock(_mutex);
    while (_batchCount < count)
    {
        test(_condition.wait_for(lock, chrono::milliseconds(5000)) != cv_status::timeout);
    }
    bool result = count == _batchCount;
    _batchCount = 0;
    return result;
}

void
TestIntfI::closeConnection(const Ice::Current& current)
{
    current.con->close(
        nullptr,
        [](exception_ptr ex)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                cerr << "Connection::close failed with: " << e.what() << endl;
                test(false);
            }
        });
}

void
TestIntfI::abortConnection(const Ice::Current& current)
{
    current.con->abort();
}

void
TestIntfI::sleep(int32_t ms, const Ice::Current&)
{
    unique_lock lock(_mutex);
    _condition.wait_for(lock, chrono::milliseconds(ms));
}

void
TestIntfI::startDispatchAsync(
    std::function<void()> response,
    std::function<void(std::exception_ptr)>,
    const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (_shutdown)
    {
        response();
        return;
    }
    else if (_pending)
    {
        _pending();
    }
    _pending = std::move(response);
}

void
TestIntfI::finishDispatch(const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (_shutdown)
    {
        return;
    }
    else if (_pending) // Pending might not be set yet if startDispatch is dispatch out-of-order
    {
        _pending();
        _pending = nullptr;
    }
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    lock_guard lock(_mutex);
    _shutdown = true;
    if (_pending)
    {
        _pending();
        _pending = nullptr;
    }
    current.adapter->getCommunicator()->shutdown();
}

bool
TestIntfI::supportsFunctionalTests(const Ice::Current&)
{
    return false;
}

bool
TestIntfI::supportsBackPressureTests(const Ice::Current&)
{
    return true;
}

void
TestIntfI::pingBiDirAsync(
    optional<Test::PingReplyPrx> reply,
    std::function<void()> response,
    std::function<void(std::exception_ptr)> exception,
    const Ice::Current& current)
{
    reply = reply->ice_fixed(current.con);
    const bool expectSuccess = current.ctx.find("ONE") == current.ctx.end();

    reply->ice_fixed(current.con)
        ->replyAsync(
            [expectSuccess, response, exception]()
            {
                try
                {
                    test(expectSuccess);
                    response();
                }
                catch (...)
                {
                    exception(std::current_exception());
                }
            },
            [expectSuccess, response, exception](exception_ptr ex)
            {
                try
                {
                    test(!expectSuccess);
                    rethrow_exception(ex);
                }
                catch (const Ice::ObjectNotExistException&)
                {
                    response();
                }
                catch (...)
                {
                    exception(std::current_exception());
                }
            });
}

void
TestIntfControllerI::holdAdapter(const Ice::Current&)
{
    _adapter->hold();
}

void
TestIntfControllerI::resumeAdapter(const Ice::Current&)
{
    _adapter->activate();
}

TestIntfControllerI::TestIntfControllerI(Ice::ObjectAdapterPtr adapter) : _adapter(std::move(adapter)) {}

int32_t
TestIntfII::op(int32_t i, int32_t& j, const Ice::Current&)
{
    j = i;
    return i;
}
