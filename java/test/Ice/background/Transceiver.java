// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

final class Transceiver implements IceInternal.Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        return _transceiver.fd();
    }

    public IceInternal.SocketStatus
    initialize(int timeout)
    {
        if(timeout == 0)
        {
            IceInternal.SocketStatus status = _configuration.initializeSocketStatus();
            if(status == IceInternal.SocketStatus.NeedConnect)
            {
                return status;
            }
            else if(status == IceInternal.SocketStatus.NeedWrite)
            {
                if(!_initialized)
                {
                    status = _transceiver.initialize(timeout);
                    if(status != IceInternal.SocketStatus.Finished)
                    {
                        return status;
                    }
                    _initialized = true;
                }
                return IceInternal.SocketStatus.NeedWrite;
            }
            else if(status == IceInternal.SocketStatus.NeedRead)
            {
                return status;
            }
        }
        _configuration.checkInitializeException();
        if(!_initialized)
        {
            IceInternal.SocketStatus status = _transceiver.initialize(timeout);
            if(status != IceInternal.SocketStatus.Finished)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal.SocketStatus.Finished;
    }

    public void
    close()
    {
        _transceiver.close();
    }

    public void
    shutdownWrite()
    {
        _transceiver.shutdownWrite();
    }

    public void
    shutdownReadWrite()
    {
        _transceiver.shutdownReadWrite();
    }

    public boolean
    write(IceInternal.Buffer buf, int timeout)
        throws  IceInternal.LocalExceptionWrapper
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(timeout == 0)
        {
            if(!_configuration.writeReady())
            {
                return false;
            }
        }
        _configuration.checkWriteException();
        return _transceiver.write(buf, timeout);
    }

    public boolean
    read(IceInternal.Buffer buf, int timeout, Ice.BooleanHolder moreData)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(timeout == 0 && !moreData.value)
        {
            if(!_configuration.readReady())
            {
                return false;
            }
        }
        _configuration.checkReadException();
        return _transceiver.read(buf, timeout, moreData);
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

    public void
    checkSendSize(IceInternal.Buffer buf, int messageSizeMax)
    {
        _transceiver.checkSendSize(buf, messageSizeMax);
    }

    //
    // Only for use by Connector, Acceptor
    //
    Transceiver(IceInternal.Transceiver transceiver)
    {
        _transceiver = transceiver;
        _configuration = Configuration.getInstance();
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        super.finalize();
    }

    final private IceInternal.Transceiver _transceiver;
    final private Configuration _configuration;
    private boolean _initialized = false;
}
