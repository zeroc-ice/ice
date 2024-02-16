//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_API_EXPORTS
#   define TEST_API_EXPORTS
#endif

#include <Configuration.h>

#include <mutex>

using namespace std;

ConfigurationPtr Configuration::_instance = nullptr;

Configuration::Configuration() :
    _initializeSocketOperation(IceInternal::SocketOperationNone),
    _initializeResetCount(0),
    _readReadyCount(0),
    _writeReadyCount(0),
    _buffered(false)
{
    assert(!_instance);
}

void
Configuration::init()
{
    _instance = shared_from_this();
}

Configuration::~Configuration()
{
    _instance = nullptr;
}

void
Configuration::connectorsException(Ice::LocalException* ex)
{
    lock_guard lock(_mutex);
    _connectorsException.reset(ex);
}

void
Configuration::checkConnectorsException()
{
    lock_guard lock(_mutex);
    if(_connectorsException.get())
    {
        _connectorsException->ice_throw();
    }
}

void
Configuration::connectException(Ice::LocalException* ex)
{
    lock_guard lock(_mutex);
    _connectException.reset(ex);
}

void
Configuration::checkConnectException()
{
    lock_guard lock(_mutex);
    if(_connectException.get())
    {
        _connectException->ice_throw();
    }
}

void
Configuration::initializeSocketOperation(IceInternal::SocketOperation status)
{
    lock_guard lock(_mutex);
    if(status == IceInternal::SocketOperationNone)
    {
        _initializeResetCount = 0;
        return;
    }
    _initializeResetCount = 10;
    _initializeSocketOperation = status;
}

void
Configuration::initializeException(Ice::LocalException* ex)
{
    lock_guard lock(_mutex);
    _initializeException.reset(ex);
}

IceInternal::SocketOperation
Configuration::initializeSocketOperation()
{
    lock_guard lock(_mutex);
    if(_initializeResetCount == 0)
    {
        return IceInternal::SocketOperationNone;
    }
    --_initializeResetCount;
    return _initializeSocketOperation;
}

void
Configuration::checkInitializeException()
{
    lock_guard lock(_mutex);
    if(_initializeException.get())
    {
        _initializeException->ice_throw();
    }
}

void
Configuration::readReady(bool ready)
{
    lock_guard lock(_mutex);
    _readReadyCount = ready ? 0 : 10;
}

void
Configuration::readException(Ice::LocalException* ex)
{
    lock_guard lock(_mutex);
    _readException.reset(ex);
}

bool
Configuration::readReady()
{
    lock_guard lock(_mutex);
    if(_readReadyCount == 0)
    {
        return true;
    }
    --_readReadyCount;
    return false;
}

void
Configuration::checkReadException()
{
    lock_guard lock(_mutex);
    if(_readException.get())
    {
        _readException->ice_throw();
    }
}

void
Configuration::writeReady(bool ready)
{
    lock_guard lock(_mutex);
    _writeReadyCount = ready ? 0 : 10;
}

void
Configuration::writeException(Ice::LocalException* ex)
{
    lock_guard lock(_mutex);
    _writeException.reset(ex);
}

bool
Configuration::writeReady()
{
    lock_guard lock(_mutex);
    if(_writeReadyCount == 0)
    {
        return true;
    }
    --_writeReadyCount;
    return false;
}

void
Configuration::checkWriteException()
{
    lock_guard lock(_mutex);
    if(_writeException.get())
    {
        _writeException->ice_throw();
    }
}

void
Configuration::buffered(bool buffered)
{
    lock_guard lock(_mutex);
    _buffered = buffered;
}

bool
Configuration::buffered()
{
    lock_guard lock(_mutex);
    return _buffered;
}

ConfigurationPtr
Configuration::getInstance()
{
    return _instance;
}
