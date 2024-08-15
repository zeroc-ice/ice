//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;

TestIntfI::TestIntfI() : _batchCount(0), _shutdown(false) {}

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
TestIntfI::close(Test::CloseMode mode, const Ice::Current& current)
{
    switch (mode)
    {
        case Test::CloseMode::Forcefully:
            current.con->abort();
            break;
        default:
            current.con->close(nullptr);
            break;
    }
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
TestIntfI::supportsAMD(const Ice::Current&)
{
    return true;
}

bool
TestIntfI::supportsFunctionalTests(const Ice::Current&)
{
    return false;
}

void
TestIntfI::pingBiDir(optional<Test::PingReplyPrx> reply, const Ice::Current& current)
{
    reply->ice_fixed(current.con)->replyAsync().get();
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

TestIntfControllerI::TestIntfControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter) {}

int32_t
TestIntfII::op(int32_t i, int32_t& j, const Ice::Current&)
{
    j = i;
    return i;
}
