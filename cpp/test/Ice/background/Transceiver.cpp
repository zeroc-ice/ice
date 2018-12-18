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
    IceInternal::SocketOperation status = IceInternal::SocketOperationNone;
#ifndef ICE_USE_IOCP
    status = _configuration->initializeSocketOperation();
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
        status = _transceiver->initialize(readBuffer, writeBuffer);
        if(status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _initialized = true;
    }
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
Transceiver::closing(bool initiator, const Ice::LocalException& ex)
{
    return _transceiver->closing(initiator, ex);
}

void
Transceiver::close()
{
    _transceiver->close();
}

IceInternal::SocketOperation
Transceiver::write(IceInternal::Buffer& buf)
{
    if(!_configuration->writeReady() && (!buf.b.empty() && buf.i < buf.b.end()))
    {
        return IceInternal::SocketOperationWrite;
    }

    _configuration->checkWriteException();
    return _transceiver->write(buf);
}

IceInternal::SocketOperation
Transceiver::read(IceInternal::Buffer& buf)
{
    if(!_configuration->readReady() && (!buf.b.empty() && buf.i < buf.b.end()))
    {
        return IceInternal::SocketOperationRead;
    }

    _configuration->checkReadException();

    if(_buffered)
    {
        while(buf.i != buf.b.end())
        {
            if(_readBufferPos == _readBuffer.i)
            {
                _readBufferPos = _readBuffer.i = _readBuffer.b.begin();
                _transceiver->read(_readBuffer);
                if(_readBufferPos == _readBuffer.i)
                {
                    _transceiver->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);
                    return IceInternal::SocketOperationRead;
                }
            }
            assert(_readBuffer.i > _readBufferPos);
            size_t requested = buf.b.end() - buf.i;
            size_t available = _readBuffer.i - _readBufferPos;
            assert(available > 0);
            if(available >= requested)
            {
                available = requested;
            }

            memcpy(buf.i, _readBufferPos, available);
            _readBufferPos += available;
            buf.i += available;
        }
        if(_readBufferPos < _readBuffer.i)
        {
            _transceiver->getNativeInfo()->ready(IceInternal::SocketOperationRead, true);
        }
        return IceInternal::SocketOperationNone;
    }
    else
    {
        return _transceiver->read(buf);
    }
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
    if(_buffered && _initialized)
    {
        size_t available = _readBuffer.i - _readBufferPos;
        if(available > 0)
        {
            size_t requested = buf.b.end() - buf.i;
            assert(available > 0);
            if(available >= requested)
            {
                available = requested;
            }

            memcpy(buf.i, _readBufferPos, available);
            _readBufferPos += available;
            buf.i += available;
        }

        if(_readBufferPos == _readBuffer.i && buf.i != buf.b.end())
        {
            _readBufferPos = _readBuffer.i = _readBuffer.b.begin();
            _transceiver->startRead(_readBuffer);
        }
        else
        {
            assert(buf.i == buf.b.end());
            _transceiver->getNativeInfo()->completed(IceInternal::SocketOperationRead);
        }
    }
    else
    {
        _transceiver->startRead(buf);
    }
}

void
Transceiver::finishRead(IceInternal::Buffer& buf)
{
    _configuration->checkReadException();
    if(_buffered && _initialized)
    {
        if(buf.i != buf.b.end())
        {
            _transceiver->finishRead(_readBuffer);

            size_t requested = buf.b.end() - buf.i;
            size_t available = _readBuffer.i - _readBufferPos;
            if(available > 0)
            {
                if(available >= requested)
                {
                    available = requested;
                }

                memcpy(buf.i, _readBufferPos, available);
                _readBufferPos += available;
                buf.i += available;
            }
        }
    }
    else
    {
        _transceiver->finishRead(buf);
    }
}
#endif

string
Transceiver::protocol() const
{
    return "test-" + _transceiver->protocol();
}

string
Transceiver::toString() const
{
    return _transceiver->toString();
}

string
Transceiver::toDetailedString() const
{
    return _transceiver->toDetailedString();
}

Ice::ConnectionInfoPtr
Transceiver::getInfo() const
{
    return _transceiver->getInfo();
}

void
Transceiver::checkSendSize(const IceInternal::Buffer& buf)
{
    _transceiver->checkSendSize(buf);
}

void
Transceiver::setBufferSize(int rcvSize, int sndSize)
{
    _transceiver->setBufferSize(rcvSize, sndSize);
}

//
// Only for use by Connector, Acceptor
//
Transceiver::Transceiver(const IceInternal::TransceiverPtr& transceiver) :
    _transceiver(transceiver),
    _configuration(Configuration::getInstance()),
    _initialized(false),
    _buffered(_configuration->buffered())
{
    _readBuffer.b.resize(1024 * 8); // 8KB buffer
    _readBufferPos = _readBuffer.b.begin();
    _readBuffer.i = _readBuffer.b.begin();
}
