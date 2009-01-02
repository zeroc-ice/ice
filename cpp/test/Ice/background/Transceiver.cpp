// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Transceiver.h>

using namespace std;

SOCKET
Transceiver::fd()
{
    return _transceiver->fd();
}

IceInternal::SocketStatus
Transceiver::initialize()
{
    IceInternal::SocketStatus status = _configuration->initializeSocketStatus();
    if(status == IceInternal::NeedConnect)
    {
        return status;
    }
    else if(status == IceInternal::NeedWrite)
    {
        if(!_initialized)
        {
            status = _transceiver->initialize();
            if(status != IceInternal::Finished)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal::NeedWrite;
    }
    else if(status == IceInternal::NeedRead)
    {
        return status;
    }

    _configuration->checkInitializeException();
    if(!_initialized)
    {
        IceInternal::SocketStatus status = _transceiver->initialize();
        if(status != IceInternal::Finished)
        {
            return status;
        }
        _initialized = true;
    }
    return IceInternal::Finished;
}

void
Transceiver::close()
{
    _transceiver->close();
}

bool
Transceiver::write(IceInternal::Buffer& buf)
{
    if(!_initialized)
    {
        throw Ice::SocketException(__FILE__, __LINE__);
    }

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
    if(!_initialized)
    {
        throw Ice::SocketException(__FILE__, __LINE__);
    }

    if(!_configuration->readReady())
    {
        return false;
    }

    _configuration->checkReadException();
    return _transceiver->read(buf);
}

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
