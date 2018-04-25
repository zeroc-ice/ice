// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>

using namespace std;
using namespace Ice;

TestIntfI::TestIntfI() :
    _batchCount(0), _shutdown(false)
{
}

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
TestIntfI::opWithPayload(ICE_IN(Ice::ByteSeq), const Ice::Current&)
{
}

void
TestIntfI::opBatch(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    ++_batchCount;
    notify();
}

Ice::Int
TestIntfI::opBatchCount(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _batchCount;
}

void
TestIntfI::opWithArgs(Ice::Int& one, Ice::Int& two, Ice::Int& three, Ice::Int& four, Ice::Int& five, Ice::Int& six,
                      Ice::Int& seven, Ice::Int& eight, Ice::Int& nine, Ice::Int& ten, Ice::Int& eleven,
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
TestIntfI::waitForBatch(Ice::Int count, const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(_batchCount < count)
    {
        timedWait(IceUtil::Time::milliSeconds(5000));
    }
    bool result = count == _batchCount;
    _batchCount = 0;
    return result;
}

void
TestIntfI::close(Test::CloseMode mode, const Ice::Current& current)
{
    current.con->close(static_cast<ConnectionClose>(mode));
}

void
TestIntfI::sleep(Ice::Int ms, const Ice::Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    timedWait(IceUtil::Time::milliSeconds(ms));
}

#ifdef ICE_CPP11_MAPPING
void
TestIntfI::startDispatchAsync(std::function<void()> response, std::function<void(std::exception_ptr)> ex,
                              const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_shutdown)
    {
        response();
        return;
    }
    else if(_pending)
    {
        _pending();
    }
    _pending = move(response);
}
#else
void
TestIntfI::startDispatch_async(const Test::AMD_TestIntf_startDispatchPtr& cb, const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_shutdown)
    {
        // Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
        // before start dispatch.
        cb->ice_response();
        return;
    }
    else if(_pending)
    {
        _pending->ice_response();
    }
    _pending = cb;
}
#endif

void
TestIntfI::finishDispatch(const Ice::Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_shutdown)
    {
        return;
    }
    else if(_pending) // Pending might not be set yet if startDispatch is dispatch out-of-order
    {
#ifdef ICE_CPP11_MAPPING
        _pending();
        _pending = nullptr;
#else
        _pending->ice_response();
        _pending = 0;
#endif
    }
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _shutdown = true;
    if(_pending)
    {
#ifdef ICE_CPP11_MAPPING
        _pending();
        _pending = nullptr;
#else
        _pending->ice_response();
        _pending = 0;
#endif
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
TestIntfI::pingBiDir(ICE_IN(Ice::Identity) id, const Ice::Current& current)
{
    ICE_UNCHECKED_CAST(Test::PingReplyPrx, current.con->createProxy(id))->reply();
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

TestIntfControllerI::TestIntfControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter)
{
}

Ice::Int
TestIntfII::op(Ice::Int i, Ice::Int& j, const Ice::Current&)
{
    j = i;
    return i;
}
