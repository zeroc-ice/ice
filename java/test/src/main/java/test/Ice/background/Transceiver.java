// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

final class Transceiver implements IceInternal.Transceiver
{
    @Override
    public java.nio.channels.SelectableChannel
    fd()
    {
        return _transceiver.fd();
    }

    @Override
    public int
    initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, Ice.Holder<Boolean> moreData)
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

    @Override
    public int
    closing(boolean initiator, Ice.LocalException ex)
    {
        return _transceiver.closing(initiator, ex);
    }

    @Override
    public void
    close()
    {
        _transceiver.close();
    }

    @Override
    public IceInternal.EndpointI
    bind()
    {
        return _transceiver.bind();
    }

    @Override
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

    @Override
    public int
    read(IceInternal.Buffer buf, Ice.Holder<Boolean> moreData)
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

    @Override
    public String
    protocol()
    {
        return "test-" + _transceiver.protocol();
    }

    @Override
    public String
    toString()
    {
        return _transceiver.toString();
    }

    @Override
    public String
    toDetailedString()
    {
        return _transceiver.toDetailedString();
    }

    @Override
    public Ice.ConnectionInfo
    getInfo()
    {
        return _transceiver.getInfo();
    }

    @Override
    public void
    checkSendSize(IceInternal.Buffer buf)
    {
        _transceiver.checkSendSize(buf);
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize)
    {
        _transceiver.setBufferSize(rcvSize, sndSize);
    }

    public IceInternal.Transceiver
    delegate()
    {
        return _transceiver;
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
        _readBuffer = new IceInternal.Buffer(false);
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
