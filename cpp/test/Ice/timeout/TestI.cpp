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
#include <IceUtil/Thread.h>

using namespace std;
using namespace Ice;

class ActivateAdapterThread : public IceUtil::Thread
{
public:

    ActivateAdapterThread(const ObjectAdapterPtr& adapter, int timeout) :
        _adapter(adapter), _timeout(timeout)
    {
    }

    virtual void
    run()
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(_timeout));
        _adapter->activate();
    }

private:

    ObjectAdapterPtr _adapter;
    int _timeout;
};

void
TimeoutI::op(const Ice::Current&)
{
}

void
TimeoutI::sendData(ICE_IN(Test::ByteSeq), const Ice::Current&)
{
}

void
TimeoutI::sleep(Ice::Int to, const Ice::Current& c)
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(to));
}

ControllerI::ControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter)
{
}

void
ControllerI::holdAdapter(Ice::Int to, const Ice::Current&)
{
    _adapter->hold();

    if(to >= 0)
    {
        IceUtil::ThreadPtr thread = new ActivateAdapterThread(_adapter, to);
        IceUtil::ThreadControl threadControl = thread->start();
        threadControl.detach();
    }
}

void
ControllerI::resumeAdapter(const Ice::Current&)
{
    _adapter->activate();
}

void
ControllerI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}
