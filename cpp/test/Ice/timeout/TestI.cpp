//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        this_thread::sleep_for(chrono::milliseconds(_timeout));
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
TimeoutI::sendData(Test::ByteSeq, const Ice::Current&)
{
}

void
TimeoutI::sleep(Ice::Int to, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(to));
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
        IceUtil::ThreadPtr thread = make_shared<ActivateAdapterThread>(_adapter, to);
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
