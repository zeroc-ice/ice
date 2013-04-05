// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Transceiver.h>

using namespace std;

IceInternal::NativeInfoPtr
Transceiver::getNativeInfo()
{
    return _transceiver->getNativeInfo();
}

IceInternal::SocketOperation
Transceiver::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
#ifndef ICE_USE_IOCP
    IceInternal::SocketOperation status = _configuration->initializeSocketOperation();
    if(status == IceInternal::SocketOperationConnect)
    {
        return status;
    }
    else if(status == IceInternal::SocketOperationWrite)
    {
        if(!_initialized)
        {
            status = _transceiver->initialize(readBuffer, writeBuffer);
            if(status != IceInternal::SocketOperationNone)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal::SocketOperationWrite;
    }
    else if(status == IceInternal::SocketOperationRead)
    {
        return status;
    }
#endif

    _configuration->checkInitializeException();
    if(!_initialized)
    {
        IceInternal::SocketOperation status = _transceiver->initialize(readBuffer, writeBuffer);
        if(status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _initialized = true;
    }
    return IceInternal::SocketOperationNone;
}

void
Transceiver::close()
{
    _transceiver->close();
}

bool
Transceiver::write(IceInternal::Buffer& buf)
{
    if(!_configuration->writeReady())
    {
        return false;
    }

    _configuration->checkWriteException();
    return _transceiver->write(buf);
}

bool
Transceiver::read(IceInternal::Buffer& buf)
{
    if(!_configuration->readReady())
    {
        return false;
    }

    _configuration->checkReadException();
    return _transceiver->read(buf);
}

#ifdef ICE_USE_IOCP
bool
Transceiver::startWrite(IceInternal::Buffer& buf)
{
    _configuration->checkWriteException();
    return _transceiver->startWrite(buf);
}

void
Transceiver::finishWrite(IceInternal::Buffer& buf)
{
    _configuration->checkWriteException();
    _transceiver->finishWrite(buf);
}

void
Transceiver::startRead(IceInternal::Buffer& buf)
{
    _configuration->checkReadException();
    _transceiver->startRead(buf);
}

void
Transceiver::finishRead(IceInternal::Buffer& buf)
{
    _configuration->checkReadException();
    _transceiver->finishRead(buf);
}
#endif

string
Transceiver::type() const
{
    return "test-" + _transceiver->type();
}

string
Transceiver::toString() const
{
    return _transceiver->toString();
}

Ice::ConnectionInfoPtr
Transceiver::getInfo() const
{
    return _transceiver->getInfo();
}

void
Transceiver::checkSendSize(const IceInternal::Buffer& buf, size_t messageSizeMax)
{
    _transceiver->checkSendSize(buf, messageSizeMax);
}

//
// Only for use by Connector, Acceptor
//
Transceiver::Transceiver(const IceInternal::TransceiverPtr& transceiver) :
    _transceiver(transceiver),
    _configuration(Configuration::getInstance()),
    _initialized(false)
{
}
