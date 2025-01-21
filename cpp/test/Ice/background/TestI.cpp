// Copyright (c) ZeroC, Inc.

#include "TestI.h"

using namespace std;
using namespace Ice;

void
BackgroundI::op(const Current& current)
{
    _controller->checkCallPause(current);
}

void
BackgroundI::opWithPayload(ByteSeq, const Current& current)
{
    _controller->checkCallPause(current);
}

void
BackgroundI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

BackgroundI::BackgroundI(BackgroundControllerIPtr controller) : _controller(std::move(controller)) {}

void
BackgroundControllerI::pauseCall(string opName, const Current&)
{
    lock_guard lock(_mutex);
    _pausedCalls.insert(opName);
}

void
BackgroundControllerI::resumeCall(string opName, const Current&)
{
    lock_guard lock(_mutex);
    _pausedCalls.erase(opName);
    _condition.notify_all();
}

void
BackgroundControllerI::checkCallPause(const Current& current)
{
    unique_lock lock(_mutex);
    _condition.wait(lock, [this, &current] { return _pausedCalls.find(current.operation) == _pausedCalls.end(); });
}

void
BackgroundControllerI::holdAdapter(const Current&)
{
    _adapter->hold();
}

void
BackgroundControllerI::resumeAdapter(const Current&)
{
    _adapter->activate();
}

void
BackgroundControllerI::initializeSocketOperation(int status, const Current&)
{
    _configuration->initializeSocketOperation(static_cast<IceInternal::SocketOperation>(status));
}

void
BackgroundControllerI::initializeException(bool enable, const Current&)
{
    _configuration->initializeException(
        enable ? make_exception_ptr(SocketException{__FILE__, __LINE__, "simulated socket error"}) : nullptr);
}

void
BackgroundControllerI::readReady(bool enable, const Current&)
{
    _configuration->readReady(enable);
}

void
BackgroundControllerI::readException(bool enable, const Current&)
{
    _configuration->readException(
        enable ? make_exception_ptr(SocketException{__FILE__, __LINE__, "simulated socket error"}) : nullptr);
}

void
BackgroundControllerI::writeReady(bool enable, const Current&)
{
    _configuration->writeReady(enable);
}

void
BackgroundControllerI::writeException(bool enable, const Current&)
{
    _configuration->writeException(
        enable ? make_exception_ptr(SocketException{__FILE__, __LINE__, "simulated socket error"}) : nullptr);
}

void
BackgroundControllerI::buffered(bool enable, const Current&)
{
    _configuration->buffered(enable);
}

BackgroundControllerI::BackgroundControllerI(ObjectAdapterPtr adapter, ConfigurationPtr configuration)
    : _adapter(std::move(adapter)),
      _configuration(std::move(configuration))
{
}
