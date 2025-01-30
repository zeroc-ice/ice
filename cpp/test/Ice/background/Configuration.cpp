// Copyright (c) ZeroC, Inc.

#ifndef TEST_API_EXPORTS
#    define TEST_API_EXPORTS
#endif

#include "Configuration.h"

#include <mutex>

using namespace std;

ConfigurationPtr Configuration::_instance = nullptr;

Configuration::Configuration() { assert(!_instance); }

void
Configuration::init()
{
    _instance = shared_from_this();
}

Configuration::~Configuration() { _instance = nullptr; }

void
Configuration::connectorsException(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _connectorsException = ex;
}

void
Configuration::checkConnectorsException()
{
    lock_guard lock(_mutex);
    if (_connectorsException)
    {
        rethrow_exception(_connectorsException);
    }
}

void
Configuration::connectException(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _connectException = ex;
}

void
Configuration::checkConnectException()
{
    lock_guard lock(_mutex);
    if (_connectException)
    {
        rethrow_exception(_connectException);
    }
}

void
Configuration::initializeSocketOperation(IceInternal::SocketOperation status)
{
    lock_guard lock(_mutex);
    if (status == IceInternal::SocketOperationNone)
    {
        _initializeResetCount = 0;
        return;
    }
    _initializeResetCount = 10;
    _initializeSocketOperation = status;
}

void
Configuration::initializeException(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _initializeException = ex;
}

IceInternal::SocketOperation
Configuration::initializeSocketOperation()
{
    lock_guard lock(_mutex);
    if (_initializeResetCount == 0)
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
    if (_initializeException)
    {
        rethrow_exception(_initializeException);
    }
}

void
Configuration::readReady(bool ready)
{
    lock_guard lock(_mutex);
    _readReadyCount = ready ? 0 : 10;
}

void
Configuration::readException(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _readException = ex;
}

bool
Configuration::readReady()
{
    lock_guard lock(_mutex);
    if (_readReadyCount == 0)
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
    if (_readException)
    {
        rethrow_exception(_readException);
    }
}

void
Configuration::writeReady(bool ready)
{
    lock_guard lock(_mutex);
    _writeReadyCount = ready ? 0 : 10;
}

void
Configuration::writeException(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _writeException = ex;
}

bool
Configuration::writeReady()
{
    lock_guard lock(_mutex);
    if (_writeReadyCount == 0)
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
    if (_writeException)
    {
        rethrow_exception(_writeException);
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
