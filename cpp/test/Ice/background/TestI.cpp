// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>

using namespace std;
using namespace Ice;

void
BackgroundI::op(const Ice::Current& current)
{
    _controller->checkCallPause(current);
}

void
BackgroundI::opWithPayload(ICE_IN(Ice::ByteSeq), const Ice::Current& current)
{
    _controller->checkCallPause(current);
}

void
BackgroundI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

BackgroundI::BackgroundI(const BackgroundControllerIPtr& controller) :
    _controller(controller)
{
}

void
BackgroundControllerI::pauseCall(ICE_IN(string) opName, const Ice::Current&)
{
    Lock sync(*this);
    _pausedCalls.insert(opName);
}

void
BackgroundControllerI::resumeCall(ICE_IN(string) opName, const Ice::Current&)
{
    Lock sync(*this);
    _pausedCalls.erase(opName);
    notifyAll();
}

void
BackgroundControllerI::checkCallPause(const Ice::Current& current)
{
    Lock sync(*this);
    while(_pausedCalls.find(current.operation) != _pausedCalls.end())
    {
        wait();
    }
}

void
BackgroundControllerI::holdAdapter(const Ice::Current&)
{
    _adapter->hold();
}

void
BackgroundControllerI::resumeAdapter(const Ice::Current&)
{
    _adapter->activate();
}

void
BackgroundControllerI::initializeSocketOperation(int status, const Ice::Current&)
{
    _configuration->initializeSocketOperation(static_cast<IceInternal::SocketOperation>(status));
}

void
BackgroundControllerI::initializeException(bool enable, const Ice::Current&)
{
    _configuration->initializeException(enable ? new Ice::SocketException(__FILE__, __LINE__) : 0);
}

void
BackgroundControllerI::readReady(bool enable, const Ice::Current&)
{
    _configuration->readReady(enable);
}

void
BackgroundControllerI::readException(bool enable, const Ice::Current&)
{
    _configuration->readException(enable ? new Ice::SocketException(__FILE__, __LINE__) : 0);
}

void
BackgroundControllerI::writeReady(bool enable, const Ice::Current&)
{
    _configuration->writeReady(enable);
}

void
BackgroundControllerI::writeException(bool enable, const Ice::Current&)
{
    _configuration->writeException(enable ? new Ice::SocketException(__FILE__, __LINE__) : 0);
}

void
BackgroundControllerI::buffered(bool enable, const Ice::Current&)
{
    _configuration->buffered(enable);
}

BackgroundControllerI::BackgroundControllerI(const Ice::ObjectAdapterPtr& adapter,
                                             const ConfigurationPtr& configuration) :
    _adapter(adapter),
    _configuration(configuration)
{
}
