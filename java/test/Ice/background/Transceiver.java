// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer)
    {
        int status = _configuration.initializeSocketStatus();
        if(status == IceInternal.SocketOperation.Connect || status == IceInternal.SocketOperation.Write)
        {
            if(!_initialized)
            {
                status = _transceiver.initialize(readBuffer, writeBuffer);
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
            status = _transceiver.initialize(readBuffer, writeBuffer);
            if(status != IceInternal.SocketOperation.None)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal.SocketOperation.None;
    }

    public void
    close()
    {
        _transceiver.close();
    }

    public boolean
    write(IceInternal.Buffer buf)
    {
        if(!_configuration.writeReady())
        {
            return false;
        }
        _configuration.checkWriteException();
        return _transceiver.write(buf);
    }

    public boolean
    read(IceInternal.Buffer buf, Ice.BooleanHolder moreData)
    {
        if(!moreData.value)
        {
            if(!_configuration.readReady())
            {
                return false;
            }
        }
        _configuration.checkReadException();
        return _transceiver.read(buf, moreData);
    }

    public String
    type()
    {
        return "test-" + _transceiver.type();
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
    }

    final private IceInternal.Transceiver _transceiver;
    final private Configuration _configuration;
    private boolean _initialized = false;
}
