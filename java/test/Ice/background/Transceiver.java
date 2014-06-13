// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

final class Transceiver implements IceInternal.Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        return _transceiver.fd();
    }

    public int
    initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, Ice.BooleanHolder moreData)
    {
        int status = _configuration.initializeSocketStatus();
        if(status == IceInternal.SocketOperation.Connect)
        {
            return status;
        }
        else if(status == IceInternal.SocketOperation.Write)
        {
            if(!_initialized)
            {
                status = _transceiver.initialize(readBuffer, writeBuffer, moreData);
                if(status != IceInternal.SocketOperation.None)
                {
                    return status;
                }
                _initialized = true;
            }
            return IceInternal.SocketOperation.Write;
        }
        else if(status == IceInternal.SocketOperation.Read)
        {
            return status;
        }

        _configuration.checkInitializeException();
        if(!_initialized)
        {
            status = _transceiver.initialize(readBuffer, writeBuffer, moreData);
            if(status != IceInternal.SocketOperation.None)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal.SocketOperation.None;
    }

    public int
    closing(boolean initiator, Ice.LocalException ex)
    {
        return _transceiver.closing(initiator, ex);
    }

    public void
    close()
    {
        _transceiver.close();
    }

    public int
    write(IceInternal.Buffer buf)
    {
        if(!_configuration.writeReady() && buf.b.hasRemaining())
        {
            return IceInternal.SocketOperation.Write;
        }

        _configuration.checkWriteException();
        return _transceiver.write(buf);
    }

    public int
    read(IceInternal.Buffer buf, Ice.BooleanHolder moreData)
    {
        if(!_configuration.readReady() && buf.b.hasRemaining())
        {
            return IceInternal.SocketOperation.Read;
        }

        _configuration.checkReadException();

        if(_buffered)
        {
            while(buf.b.hasRemaining())
            {
                if(_readBufferPos == _readBuffer.b.position())
                {
                    _readBufferPos = 0;
                    _readBuffer.b.position(0);
                    _transceiver.read(_readBuffer, moreData);
                    if(_readBufferPos == _readBuffer.b.position())
                    {
                        moreData.value = false;
                        return IceInternal.SocketOperation.Read;
                    }
                }
                final int pos = _readBuffer.b.position();
                assert(pos > _readBufferPos);
                final int requested = buf.b.remaining();
                int available = pos - _readBufferPos;
                assert(available > 0);
                if(available >= requested)
                {
                    available = requested;
                }

                byte[] arr = new byte[available];
                _readBuffer.b.position(_readBufferPos);
                _readBuffer.b.get(arr);
                buf.b.put(arr);
                _readBufferPos += available;
                _readBuffer.b.position(pos);
            }
            moreData.value = _readBufferPos < _readBuffer.b.position();
            return IceInternal.SocketOperation.None;
        }
        else
        {
            return _transceiver.read(buf, moreData);
        }
    }

    public String
    protocol()
    {
        return "test-" + _transceiver.protocol();
    }

    public String
    toString()
    {
        return _transceiver.toString();
    }

    public Ice.ConnectionInfo
    getInfo()
    {
        return _transceiver.getInfo();
    }

    public void
    checkSendSize(IceInternal.Buffer buf, int messageSizeMax)
    {
        _transceiver.checkSendSize(buf, messageSizeMax);
    }

    //
    // Only for use by Connector, Acceptor
    //
    Transceiver(Configuration configuration, IceInternal.Transceiver transceiver)
    {
        _transceiver = transceiver;
        _configuration = configuration;
        _initialized = false;
        _buffered = _configuration.buffered();
        _readBuffer = new IceInternal.Buffer(100 * 1024, false);
        _readBuffer.resize(1024 * 8, true); // 8KB buffer
        _readBufferPos = 0;
    }

    private final IceInternal.Transceiver _transceiver;
    private final Configuration _configuration;
    private boolean _initialized;
    private final boolean _buffered;
    private IceInternal.Buffer _readBuffer;
    private int _readBufferPos;
}
