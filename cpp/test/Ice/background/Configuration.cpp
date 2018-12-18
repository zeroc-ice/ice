// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_API_EXPORTS
#   define TEST_API_EXPORTS
#endif

#include <Configuration.h>

Configuration* Configuration::_instance = 0;

Configuration::Configuration() :
    _initializeSocketOperation(IceInternal::SocketOperationNone),
    _initializeResetCount(0),
    _readReadyCount(0),
    _writeReadyCount(0),
    _buffered(false)
{
    assert(!_instance);
    _instance = this;
}

Configuration::~Configuration()
{
    _instance = 0;
}

void
Configuration::connectorsException(Ice::LocalException* ex)
{
    Lock sync(*this);
    _connectorsException.reset(ex);
}

void
Configuration::checkConnectorsException()
{
    Lock sync(*this);
    if(_connectorsException.get())
    {
        _connectorsException->ice_throw();
    }
}

void
Configuration::connectException(Ice::LocalException* ex)
{
    Lock sync(*this);
    _connectException.reset(ex);
}

void
Configuration::checkConnectException()
{
    Lock sync(*this);
    if(_connectException.get())
    {
        _connectException->ice_throw();
    }
}

void
Configuration::initializeSocketOperation(IceInternal::SocketOperation status)
{
    Lock sync(*this);
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
    Lock sync(*this);
    _initializeException.reset(ex);
}

IceInternal::SocketOperation
Configuration::initializeSocketOperation()
{
    Lock sync(*this);
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
    Lock sync(*this);
    if(_initializeException.get())
    {
        _initializeException->ice_throw();
    }
}

void
Configuration::readReady(bool ready)
{
    Lock sync(*this);
    _readReadyCount = ready ? 0 : 10;
}

void
Configuration::readException(Ice::LocalException* ex)
{
    Lock sync(*this);
    _readException.reset(ex);
}

bool
Configuration::readReady()
{
    Lock sync(*this);
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
    Lock sync(*this);
    if(_readException.get())
    {
        _readException->ice_throw();
    }
}

void
Configuration::writeReady(bool ready)
{
    Lock sync(*this);
    _writeReadyCount = ready ? 0 : 10;
}

void
Configuration::writeException(Ice::LocalException* ex)
{
    Lock sync(*this);
    _writeException.reset(ex);
}

bool
Configuration::writeReady()
{
    Lock sync(*this);
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
    Lock sync(*this);
    if(_writeException.get())
    {
        _writeException->ice_throw();
    }
}

void
Configuration::buffered(bool buffered)
{
    Lock sync(*this);
    _buffered = buffered;
}

bool
Configuration::buffered()
{
    Lock sync(*this);
    return _buffered;
}

Configuration*
Configuration::getInstance()
{
    return _instance;
}
